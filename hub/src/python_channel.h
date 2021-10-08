#ifndef CHANNEL_HUB_PYTHON_CHANNEL_H
#define CHANNEL_HUB_PYTHON_CHANNEL_H

#include <mutex>
#include <map>
#include "channel.h"

#include <pybind11/embed.h>
namespace py = pybind11;

class PythonChannel: public Channel
{
public:
    PythonChannel(const std::string& id, const std::string& title, const py::object& clazz);

    CallbackStatus get_attachment(int client, const std::string& url, std::string& fout) override;
    GetChannelBoardsResult get_boards(int client, uint32_t limit) override;
    void set_settings(int client, const std::unordered_map<std::string, std::string>& s) override;
    GetSettingDefsResult get_setting_defs(int client) override;
    GetChannelThreadsResult get_threads(int client, const BoardId &board) override;
    GetChannelThreadResult get_thread(int client, const BoardId &board, const ThreadId &thread) override;
    PostResult post(int client, const BoardId &board, const ThreadId &thread, const std::string& comment,
        const PostId& reply_to) override;

    void new_client(int client) override;
    void client_released(int client) override;
    void set_key(int client, const std::string& key) override;

private:
    std::map<int, py::object> m_client_settings;
    std::mutex m_client_settings_mutex;
    py::object m_instance;
};

#endif
