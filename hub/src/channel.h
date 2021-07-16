#ifndef CHANNEL_HUB_CHANNEL_H
#define CHANNEL_HUB_CHANNEL_H

#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "board.h"
#include "thread.h"
#include "callbacks.h"


typedef std::string ChannelId;
typedef std::unique_ptr<class Channel> ChannelPtr;

class Channel
{
public:
    virtual CallbackStatus get_attachment(const BoardId &board, const ThreadId &thread, const PostId &post,
        const std::string& attachment, uint32_t width, uint32_t height,
        std::string& fout) = 0;
    virtual GetBoardsResult get_boards(uint32_t limit) = 0;
    virtual GetThreadsResult get_threads(const BoardId &board) = 0;
    virtual GetThreadResult get_thread(const BoardId &board, const ThreadId &thread) = 0;

    const std::string& get_name() const { return m_name; }
    const std::string& get_title() const { return m_title; }
    virtual ~Channel() = default;

    ChannelObject* write() const;
protected:
    explicit Channel(std::string&& name, std::string&& title) :
        m_name(std::move(name)), m_title(std::move(title)) {}

    std::string preprocess_html(const std::string& source);

    std::string cache_key(const std::string& key);
private:
    std::string m_name;
    std::string m_title;
};

#endif