#include "hub.h"

#include <unordered_map>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include "board.h"
#include "channels/four_chan.h"

ChannelHub* ChannelHub::s_hub = nullptr;
bool ChannelHub::s_verbose = false;

ChannelHub::ChannelHub() :
    m_image_processing()
{
    s_hub = this;

    m_channels.emplace("4chan", new FourChan());
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

GetBoardsResult ChannelHub::get_boards(const ChannelId &channel, uint32_t limit)
{
    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        return GetBoardsResult(CallbackStatus::unknown_resource, std::vector<class Board>());
    }

    return ch->get_boards(limit);
}

GetImageResult ChannelHub::get_image(const ChannelId &channel, const BoardId &board,
    const ThreadId& thread, const PostId &post, uint32_t target_w, uint32_t target_h,
    ImageProcessing::ImageEncoding encoding)
{
    std::string attachment;
    uint32_t w = 0;
    uint32_t h = 0;

    {
        std::lock_guard<std::mutex> cache_guard(m_catalog_cache_mutex);

        auto cached_catalog_thread = m_catalog_cache.find({channel, board});
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
        auto cached_thread = m_thread_cache.find({channel, board, thread});
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
        return GetImageResult(CallbackStatus::unknown_resource);
    }

    if (target_w > w)
    {
        return GetImageResult(CallbackStatus::failed);
    }

    if (target_h > h)
    {
        return GetImageResult(CallbackStatus::failed);
    }

    const ChannelPtr& ch = get_channel(channel);
    if (ch == nullptr)
    {
        return GetImageResult(CallbackStatus::unknown_resource);
    }

    std::string fname;

    auto result = ch->get_attachment(board, thread, post, attachment, w, h, fname);
    if (result != CallbackStatus::ok)
    {
        return GetImageResult(result);
    }

    return m_image_processing.reencode_image(fname, w, h, target_w, target_h, encoding);
}

GetThreadsResult ChannelHub::get_threads(const ChannelId &channel, const BoardId &board,
    bool flush, uint32_t offset, uint32_t limit)
{
    {
        std::lock_guard<std::mutex> cache_guard(m_catalog_cache_mutex);
        auto cached_catalog = m_catalog_cache.find({channel, board});

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

    auto res = ch->get_threads(board);

    if (res.status == CallbackStatus::ok)
    {
        {
            std::lock_guard<std::mutex> cache_guard(m_catalog_cache_mutex);
            m_catalog_cache[{channel, board}] = res.threads;
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

GetThreadResult ChannelHub::get_thread(const ChannelId &channel, const BoardId &board, const ThreadId &thread,
    bool flush, uint32_t offset, uint32_t limit)
{
    {
        std::lock_guard<std::mutex> cache_guard(m_thread_cache_mutex);
        auto cached_thread = m_thread_cache.find({channel, board, thread});
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

    auto res = ch->get_thread(board, thread);

    if (res.status == CallbackStatus::ok)
    {
        {
            std::lock_guard<std::mutex> cache_guard(m_thread_cache_mutex);
            m_thread_cache[{channel, board, thread}] = res.posts;
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
