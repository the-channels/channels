#ifndef CHANNEL_HUB_FOUR_CHAN_H
#define CHANNEL_HUB_FOUR_CHAN_H

#include "channel.h"

class FourChan: public Channel
{
private:
public:
    FourChan();

public:
    CallbackStatus get_attachment(const BoardId &board, const ThreadId &thread, const PostId &post,
        const std::string& attachment, uint32_t width, uint32_t height, std::string& fout) override;
    GetBoardsResult get_boards(uint32_t limit) override;
    GetThreadsResult get_threads(const BoardId& board) override;
    GetThreadResult get_thread(const BoardId& board, const ThreadId& thread) override;

private:
    static std::string s_base_url;
};

#endif
