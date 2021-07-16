#ifndef CHANNEL_HUB_HUB_H
#define CHANNEL_HUB_HUB_H

#include <unordered_map>
#include <mutex>
#include <memory>
#include "channel.h"
#include "img.h"

struct BoardKey
{
    std::string channel;
    std::string board;

    bool operator==(const BoardKey& other) const
    {
        return this->channel == other.channel && this->board == other.board;
    }

    struct Hash
    {
        size_t operator()(const BoardKey& pos) const
        {
            size_t channelHash = std::hash<std::string>()(pos.channel);
            size_t boardHash = std::hash<std::string>()(pos.board) << 1;
            return channelHash ^ boardHash;
        }
    };
};

struct ThreadKey
{
    std::string channel;
    std::string board;
    std::string thread;

    bool operator==(const ThreadKey& other) const
    {
        return this->channel == other.channel && this->board == other.board && this->thread == other.thread;
    }

    struct Hash
    {
        size_t operator()(const ThreadKey& pos) const
        {
            size_t channelHash = std::hash<std::string>()(pos.channel);
            size_t boardHash = std::hash<std::string>()(pos.board) << 1;
            size_t threadHash = std::hash<std::string>()(pos.thread) << 2;
            return channelHash ^ boardHash ^ threadHash;
        }
    };
};

using CatalogCache = std::unordered_map<BoardKey, std::vector<class Thread>, BoardKey::Hash>;
using ThreadCache = std::unordered_map<ThreadKey, std::vector<class Post>, ThreadKey::Hash>;

class ChannelHub
{
public:
    ChannelHub();

public:
    static bool IsVerbose() { return s_verbose; }
    static void SetVerbose() { s_verbose = true; }
    static ChannelHub* Get() { return s_hub; }

public:
    virtual int run() = 0;

    GetImageResult get_image(const ChannelId &channel, const BoardId &board,
        const ThreadId& thread, const PostId &post,
        uint32_t target_w, uint32_t target_h,
        ImageProcessing::ImageEncoding encoding);
    GetBoardsResult get_boards(const ChannelId &channel, uint32_t limit);
    GetThreadsResult get_threads(const ChannelId &channel, const BoardId &board,
        bool flush, uint32_t offset, uint32_t limit);
    GetThreadResult get_thread(const ChannelId &channel, const BoardId &board, const ThreadId &thread,
        bool flush, uint32_t offset, uint32_t limit);

    const std::map<ChannelId, ChannelPtr>& get_channels() const { return m_channels; }

private:
    const ChannelPtr& get_channel(const ChannelId& channel) const;

private:
    static ChannelHub* s_hub;
    static bool s_verbose;

    CatalogCache m_catalog_cache;
    std::mutex m_catalog_cache_mutex;

    ThreadCache m_thread_cache;
    std::mutex m_thread_cache_mutex;

    ImageProcessing m_image_processing;
    std::map<ChannelId, ChannelPtr> m_channels;
};

#endif
