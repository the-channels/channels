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

    CallbackStatus get_attachment(int client, const BoardId &board, const ThreadId &thread, const PostId &post,
        const std::string& attachment, uint32_t width, uint32_t height,
        std::string& fout) override;
    GetBoardsResult get_boards(int client, uint32_t limit) override;
    GetThreadsResult get_threads(int client, const BoardId &board) override;
    GetThreadResult get_thread(int client, const BoardId &board, const ThreadId &thread) override;

    void new_client(int client) override;
    void client_released(int client) override;

private:
    std::map<int, py::object> m_client_settings;
    std::mutex m_client_settings_mutex;
    py::object m_instance;
    py::object m_client_class;
};

#endif
