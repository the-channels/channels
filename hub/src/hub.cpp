#include "hub.h"

#include <unordered_map>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "board.h"
#include "python_channel.h"

ChannelHub* ChannelHub::s_hub = nullptr;
bool ChannelHub::s_verbose = false;
bool ChannelHub::s_debug = false;

ChannelHub::ChannelHub() :
    m_image_processing(),
    m_python()
{
    s_hub = this;
}

bool ChannelHub::init()
{
    if (IsDebug())
    {
        using namespace pybind11::literals;

        try
        {
            py::module_::import("pydevd").attr("settrace")("localhost", "port"_a=5678, "stdoutToServer"_a=true,
                "stderrToServer"_a=true, "suspend"_a=false);
        }
        catch (py::error_already_set &e)
        {
            std::cerr << "Cannot connect to pydevd (" << e.what() << "), skipping debugging" << std::endl;
        }
    }

    try
    {
        py::module_ channels_module = py::module_::import("channels.base");

        channels_module.attr("import_modules")(py::cpp_function(
            [this](std::string name, std::string description, py::object clazz)
            {
                std::cout << "Registering channel: " << name << std::endl;
                register_channel(name, new PythonChannel(name, description, clazz));
            }));
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

void ChannelHub::register_channel(const ChannelId& channel_id, Channel* ptr)
{
    m_channels.emplace(channel_id, ptr);
}

void ChannelHub::new_client(int client)
{
    for (auto& entry: m_channels)
    {
        entry.second->new_client(client);
    }
}

void ChannelHub::client_released(int client)
{
    for (auto& entry: m_channels)
    {
        entry.second->client_released(client);
    }
}

const ChannelPtr& ChannelHub::get_channel(const ChannelId& channel) const
{
    static ChannelPtr null_channel = ChannelPtr();

    auto it = m_channels.find(channel);
    if (it == m_channels.end())
    {
        return null_channel;
    }

    return it->second;
}

GetBoardsResult ChannelHub::get_boards(int client, const ChannelId &channel, uint32_t limit)
{
    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        return GetBoardsResult(CallbackStatus::unknown_resource, std::vector<class Board>());
    }

    return ch->get_boards(client, limit);
}

GetImageResult ChannelHub::get_image(int client, const ChannelId &channel, const BoardId &board,
    const ThreadId& thread, const PostId &post, uint32_t target_w, uint32_t target_h,
    ImageProcessing::ImageEncoding encoding)
{
    std::string attachment;
    uint32_t w = 0;
    uint32_t h = 0;

    {
        std::lock_guard<std::mutex> cache_guard(m_catalog_cache_mutex);

        auto cached_catalog_thread = m_catalog_cache.find({client, channel, board});
        if (cached_catalog_thread != m_catalog_cache.end())
        {
            auto& cached_catalog = cached_catalog_thread->second;

            for (auto& th: cached_catalog)
            {
                if (th.id == thread)
                {
                    attachment = th.attachment;
                    w = th.attachment_width;
                    h = th.attachment_height;
                    break;
                }
            }
        }
    }

    if (!attachment.empty())
    {
        std::lock_guard<std::mutex> cache_guard(m_thread_cache_mutex);
        auto cached_thread = m_thread_cache.find({client, channel, board, thread});
        if (cached_thread != m_thread_cache.end())
        {
            auto& posts = cached_thread->second;

            for (auto& ps: posts)
            {
                if (ps.id == post)
                {
                    attachment = ps.attachment;
                    w = ps.attachment_width;
                    h = ps.attachment_height;
                    break;
                }
            }
        }
    }

    if (attachment.empty())
    {
        std::cerr << "empty attachment" << std::endl;
        return GetImageResult(CallbackStatus::unknown_resource);
    }

    if (target_w > w)
    {
        std::cerr << "wrong target_w" << std::endl;
        return GetImageResult(CallbackStatus::failed);
    }

    if (target_h > h)
    {
        std::cerr << "wrong target_h" << std::endl;
        return GetImageResult(CallbackStatus::failed);
    }

    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        std::cerr << "Cannot obtain channel" << std::endl;
        return GetImageResult(CallbackStatus::unknown_resource);
    }

    std::string fname;

    auto result = ch->get_attachment(client, board, thread, post, attachment, w, h, fname);
    if (result != CallbackStatus::ok)
    {
        std::cerr << "Cannot fetch attachment" << std::endl;
        return GetImageResult(result);
    }

    return m_image_processing.reencode_image(fname, w, h, target_w, target_h, encoding);
}

GetThreadsResult ChannelHub::get_threads(
    int client, const ChannelId &channel, const BoardId &board,
    bool flush, uint32_t offset, uint32_t limit)
{
    {
        std::lock_guard<std::mutex> cache_guard(m_catalog_cache_mutex);
        auto cached_catalog = m_catalog_cache.find({client, channel, board});

        if (!flush && cached_catalog != m_catalog_cache.end())
        {
            auto& catalog = cached_catalog->second;
            std::vector<class Thread> result;
            auto end = limit + offset < catalog.size() ? catalog.begin() + limit + offset : catalog.end();
            std::copy(catalog.begin() + offset, end, std::back_inserter(result));
            return GetThreadsResult(CallbackStatus::ok, std::move(result), catalog.size());
        }
    }

    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        return GetThreadsResult(CallbackStatus::unknown_resource, std::vector<class Thread>(), 0);
    }

    auto res = ch->get_threads(client, board);

    if (res.status == CallbackStatus::ok)
    {
        {
            std::lock_guard<std::mutex> cache_guard(m_catalog_cache_mutex);
            m_catalog_cache[{client, channel, board}] = res.threads;
        }

        std::vector<class Thread> result;
        if (offset < res.threads.size())
        {
            auto end = limit + offset < res.threads.size() ? res.threads.begin() + limit + offset : res.threads.end();
            result.insert(result.end(), res.threads.begin() + offset, end);
        }
        return GetThreadsResult(CallbackStatus::ok, std::move(result), res.threads.size());
    }
    else
    {
        return res;
    }
}

GetThreadResult ChannelHub::get_thread(
    int client, const ChannelId &channel, const BoardId &board, const ThreadId &thread,
    bool flush, uint32_t offset, uint32_t limit)
{
    {
        std::lock_guard<std::mutex> cache_guard(m_thread_cache_mutex);
        auto cached_thread = m_thread_cache.find({client, channel, board, thread});
        if (!flush && cached_thread != m_thread_cache.end())
        {
            auto& posts = cached_thread->second;
            std::vector<class Post> result;
            if (offset < posts.size())
            {
                auto end = limit + offset < posts.size() ? posts.begin() + limit + offset : posts.end();
                result.insert(result.end(), posts.begin() + offset, end);
            }
            return GetThreadResult(CallbackStatus::ok, std::move(result), posts.size());
        }
    }

    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        return GetThreadResult(CallbackStatus::unknown_resource, std::vector<class Post>(), 0);
    }

    auto res = ch->get_thread(client, board, thread);

    if (res.status == CallbackStatus::ok)
    {
        {
            std::lock_guard<std::mutex> cache_guard(m_thread_cache_mutex);
            m_thread_cache[{client, channel, board, thread}] = res.posts;
        }

        std::vector<class Post> result;
        auto end = limit + offset < res.posts.size() ? res.posts.begin() + limit + offset : res.posts.end();
        result.insert(result.end(), res.posts.begin() + offset, end);
        return GetThreadResult(CallbackStatus::ok, std::move(result), res.posts.size());
    }
    else
    {
        return res;
    }
}
