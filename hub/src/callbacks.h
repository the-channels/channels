#ifndef CHANNEL_HUB_CALLBACKS_H
#define CHANNEL_HUB_CALLBACKS_H

#include <utility>
#include <vector>

enum class CallbackStatus
{
    ok,
    failed,
    unknown_resource
};

typedef std::pair<CallbackStatus, std::vector<class Board>> GetBoardsResult;

typedef struct GetThreadsResult {
    GetThreadsResult(CallbackStatus s, std::vector<class Thread>&& th, uint32_t num_threads) :
        status(s), threads(std::move(th)), num_threads(num_threads) {}
    GetThreadsResult(CallbackStatus s) :
        status(s), threads(), num_threads(0) {}

    CallbackStatus status;
    std::vector<class Thread> threads;
    uint32_t num_threads;
} GetThreadsResult;

typedef struct GetThreadResult {
    GetThreadResult(CallbackStatus s, std::vector<class Post>&& ps, uint32_t num_posts) :
        status(s), posts(std::move(ps)), num_posts(num_posts) {}
    GetThreadResult(CallbackStatus s) :
        status(s), posts(), num_posts(0) {}

    CallbackStatus status;
    std::vector<class Post> posts;
    uint32_t num_posts;
} GetThreadResult;


typedef struct GetImageResult {
    GetImageResult(CallbackStatus s, const std::vector<uint8_t>* data, uint16_t data_size, uint32_t w, uint32_t h) :
        status(s), data(data), data_size(data_size), w(w), h(h) {}
    GetImageResult(CallbackStatus s) :
        status(s), data(nullptr), data_size(0), w(0), h(0) {}

    CallbackStatus status;
    const std::vector<uint8_t>* data;
    uint16_t w;
    uint16_t h;
    uint16_t data_size;
} GetImageResult;

#endif
