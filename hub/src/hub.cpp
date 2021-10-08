#include "hub.h"

#include <unordered_map>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "board.h"
#include "setting_def.h"
#include "python_channel.h"

ChannelHub* ChannelHub::s_hub = nullptr;
bool ChannelHub::s_verbose = false;
bool ChannelHub::s_debug = false;

ChannelHub::ChannelHub() :
    m_image_processing(),
    m_python(),
    m_next_image_id(0)
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

void ChannelHub::set_key(int client, const std::string& key)
{
    for (auto& entry: m_channels)
    {
        entry.second->set_key(client, key);
    }
}

void ChannelHub::client_released(int client)
{
    for (auto& entry: m_channels)
    {
        entry.second->client_released(client);
    }
}

void ChannelHub::clear_thread_cache(int client, const ChannelId &channel, const BoardId &board, const ThreadId& thread)
{
    m_thread_cache.erase({client, channel, board, thread});
}

void ChannelHub::clear_catalog_cache(int client, const ChannelId &channel, const BoardId &board)
{
    m_catalog_cache.erase({client, channel, board});
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

GetSettingDefsResult ChannelHub::get_setting_defs(int client, const ChannelId &channel)
{
    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        return GetSettingDefsResult(CallbackStatus::unknown_resource, std::vector<class SettingDef>());
    }

    return ch->get_setting_defs(client);
}

void ChannelHub::set_settings(int client, const ChannelId &channel, const std::unordered_map<std::string, std::string>& s)
{
    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        return;
    }

    return ch->set_settings(client, s);
}

uint16_t ChannelHub::register_image(const std::string& url)
{
    auto it = m_images.find(url);
    if (it == m_images.end())
    {
        m_next_image_id++;
        m_images[url] = m_next_image_id;
        m_image_ids[m_next_image_id] = url;
        return m_next_image_id;
    }
    else
    {
        return it->second;
    }
}

GetImageResult ChannelHub::get_image(int client, const ChannelId &channel, uint16_t image_id,
    uint32_t target_w, uint32_t target_h,
    ImageProcessing::ImageEncoding encoding)
{
    auto it = m_image_ids.find(image_id);

    if (it == m_image_ids.end())
    {
        std::cerr << "unknown image" << std::endl;
        return GetImageResult(CallbackStatus::unknown_resource);
    }

    const std::string& url = it->second;
    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        std::cerr << "Cannot obtain channel" << std::endl;
        return GetImageResult(CallbackStatus::unknown_resource);
    }

    std::string fname;

    auto result = ch->get_attachment(client, url, fname);
    if (result != CallbackStatus::ok)
    {
        std::cerr << "Cannot fetch attachment" << std::endl;
        return GetImageResult(result);
    }

    return m_image_processing.reencode_image(fname, target_w, target_h, encoding);
}

GetThreadsResult ChannelHub::get_threads(
    int client, const ChannelId &channel, const BoardId &board,
    bool flush)
{
    {
        std::lock_guard<std::mutex> cache_guard(m_catalog_cache_mutex);
        auto cached_catalog = m_catalog_cache.find({client, channel, board});

        if (!flush && cached_catalog != m_catalog_cache.end())
        {
            return GetThreadsResult(CallbackStatus::ok, &cached_catalog->second, cached_catalog->second.size());
        }
    }

    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        return GetThreadsResult(CallbackStatus::unknown_resource);
    }

    auto res = ch->get_threads(client, board);

    if (res.status == CallbackStatus::ok)
    {
        {
            std::lock_guard<std::mutex> cache_guard(m_catalog_cache_mutex);
            m_catalog_cache[{client, channel, board}] = std::move(res.threads);
        }

        auto rr = m_catalog_cache.find({client, channel, board});
        return GetThreadsResult(CallbackStatus::ok, &rr->second, rr->second.size());
    }
    else
    {
        return GetThreadsResult(res.status);
    }
}

GetThreadResult ChannelHub::get_thread(
    int client, const ChannelId &channel, const BoardId &board, const ThreadId &thread,
    bool flush)
{
    {
        std::lock_guard<std::mutex> cache_guard(m_thread_cache_mutex);
        auto cached_thread = m_thread_cache.find({client, channel, board, thread});
        if (!flush && cached_thread != m_thread_cache.end())
        {
            return GetThreadResult(CallbackStatus::ok, &cached_thread->second, cached_thread->second.size());
        }
    }

    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        return GetThreadResult(CallbackStatus::unknown_resource, nullptr, 0);
    }

    auto res = ch->get_thread(client, board, thread);

    if (res.status == CallbackStatus::ok)
    {
        {
            std::lock_guard<std::mutex> cache_guard(m_thread_cache_mutex);
            m_thread_cache[{client, channel, board, thread}] = std::move(res.posts);
        }

        auto rr = m_thread_cache.find({client, channel, board, thread});
        return GetThreadResult(CallbackStatus::ok, &rr->second, rr->second.size());
    }
    else
    {
        return GetThreadResult(res.status);
    }
}

PostResult ChannelHub::post(int client, const ChannelId& channel, const BoardId& board, const ThreadId& thread,
    const std::string& comment, const PostId& reply_to)
{
    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        return PostResult(CallbackStatus::unknown_resource, "No such channel");
    }

    PostResult res = ch->post(client, board, thread, comment, reply_to);
    if (res.status == CallbackStatus::ok)
    {
        if (thread.empty())
        {
            clear_catalog_cache(client, channel, board);
        }
        else
        {
            clear_thread_cache(client, channel, board, thread);
        }
    }
    return res;
}
