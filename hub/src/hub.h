#ifndef CHANNEL_HUB_HUB_H
#define CHANNEL_HUB_HUB_H

#include <unordered_map>
#include <mutex>
#include <memory>
#include "channel.h"
#include "img.h"
#include <pybind11/embed.h>
namespace py = pybind11;

struct BoardKey
{
    int client;
    std::string channel;
    std::string board;

    bool operator==(const BoardKey& other) const
    {
        return this->client == other.client && this->channel == other.channel && this->board == other.board;
    }

    struct Hash
    {
        size_t operator()(const BoardKey& pos) const
        {
            size_t clientHash = std::hash<int>()(pos.client);
            size_t channelHash = std::hash<std::string>()(pos.channel) << 1;
            size_t boardHash = std::hash<std::string>()(pos.board) << 2;
            return clientHash ^ channelHash ^ boardHash;
        }
    };
};

struct ThreadKey
{
    int client;
    std::string channel;
    std::string board;
    std::string thread;

    bool operator==(const ThreadKey& other) const
    {
        return this->client == other.client && this->channel == other.channel && this->board == other.board && this->thread == other.thread;
    }

    struct Hash
    {
        size_t operator()(const ThreadKey& pos) const
        {
            size_t clientHash = std::hash<int>()(pos.client);
            size_t channelHash = std::hash<std::string>()(pos.channel) << 1;
            size_t boardHash = std::hash<std::string>()(pos.board) << 2;
            size_t threadHash = std::hash<std::string>()(pos.thread) << 3;
            return clientHash ^ channelHash ^ boardHash ^ threadHash;
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

    GetImageResult get_image(int client, const ChannelId &channel, const BoardId &board,
        const ThreadId& thread, const PostId &post,
        uint32_t target_w, uint32_t target_h,
        ImageProcessing::ImageEncoding encoding);
    GetBoardsResult get_boards(int client, const ChannelId &channel, uint32_t limit);
    GetThreadsResult get_threads(int client, const ChannelId &channel, const BoardId &board,
        bool flush, uint32_t offset, uint32_t limit);
    GetThreadResult get_thread(int client, const ChannelId &channel, const BoardId &board, const ThreadId &thread,
        bool flush, uint32_t offset, uint32_t limit);

    void new_client(int client);
    void client_released(int client);

    const std::map<ChannelId, ChannelPtr>& get_channels() const { return m_channels; }
    void register_channel(const ChannelId& channel_id, Channel* ptr);

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
    py::scoped_interpreter m_python;
};

#endif
