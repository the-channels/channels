#ifndef CHANNEL_HUB_CHANNEL_H
#define CHANNEL_HUB_CHANNEL_H

#include <sstream>
#include <string>
#include <unordered_map>
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
    virtual CallbackStatus get_attachment(int client, const std::string& url, std::string& fout) = 0;
    virtual GetChannelBoardsResult get_boards(int client, uint32_t limit) = 0;
    virtual GetSettingDefsResult get_setting_defs(int client) = 0;
    virtual void set_settings(int client, const std::unordered_map<std::string, std::string>& s) = 0;
    virtual GetChannelThreadsResult get_threads(int client, const BoardId &board) = 0;
    virtual GetChannelThreadResult get_thread(int client, const BoardId &board, const ThreadId &thread) = 0;
    virtual void new_client(int client) {}
    virtual void set_key(int client, const std::string& key) {}
    virtual void client_released(int client) {}

    const std::string& get_name() const { return m_name; }
    const std::string& get_title() const { return m_title; }
    virtual ~Channel() = default;

    ChannelObject* write() const;
protected:
    explicit Channel(const std::string& name, const std::string& title) :
        m_name(name), m_title(title) {}

    std::string preprocess_html(const std::string& source);

    std::string cache_key(const std::string& key);
private:
    std::string m_name;
    std::string m_title;
};

#endif