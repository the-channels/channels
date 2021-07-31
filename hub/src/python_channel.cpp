#include "python_channel.h"
#include <iostream>

PythonChannel::PythonChannel(const std::string& id, const std::string& title, const py::object& clazz) :
    Channel(id, title)
{
    // allocate a instance
    m_instance = clazz();
    m_instance.inc_ref();

    m_client_class = getattr(py::module_::import("channels.base"), "Client");
}

CallbackStatus PythonChannel::get_attachment(
    int client, const BoardId &board, const ThreadId &thread, const PostId &post,
    const std::string& attachment, uint32_t width, uint32_t height,
    std::string& fout)
{
    py::gil_scoped_acquire acquire;

    try
    {
        auto res = m_instance.attr("get_attachment")(m_client_settings[client], board, thread, post,
            attachment, width, height);

        fout = py::cast<std::string>(res);
        return CallbackStatus(CallbackStatus::ok);
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
        return CallbackStatus(CallbackStatus::failed);
    }
}

GetBoardsResult PythonChannel::get_boards(int client, uint32_t limit)
{
    py::gil_scoped_acquire acquire;

    try
    {
        auto res = m_instance.attr("get_boards")(m_client_settings[client], limit);
        std::vector<Board> boards;
        for (auto& entry: py::list(res))
        {
            Board board;
            board.id = py::cast<std::string>(py::getattr(entry, "id"));
            if (!py::isinstance<py::none>(py::getattr(entry, "title")))
            {
                board.title = py::cast<std::string>(py::getattr(entry, "title"));
            }
            if (!py::isinstance<py::none>(py::getattr(entry, "description")))
            {
                board.description = py::cast<std::string>(py::getattr(entry, "description"));
            }
            boards.push_back(board);
        }
        return GetBoardsResult(CallbackStatus::ok, std::move(boards));
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
        return GetBoardsResult(CallbackStatus::failed, std::vector<Board>());
    }
}

GetThreadsResult PythonChannel::get_threads(int client, const BoardId &board)
{
    py::gil_scoped_acquire acquire;

    try
    {
        auto res = m_instance.attr("get_threads")(m_client_settings[client], board);
        std::vector<Thread> threads;
        uint32_t num_posts = 0;
        for (auto& entry: py::list(res))
        {
            Thread thread;
            thread.id = py::cast<std::string>(py::getattr(entry, "id"));
            if (!py::isinstance<py::none>(py::getattr(entry, "title")))
            {
                thread.title = py::cast<std::string>(py::getattr(entry, "title"));
            }
            if (!py::isinstance<py::none>(py::getattr(entry, "comment")))
            {
                thread.comment = py::cast<std::string>(py::getattr(entry, "comment"));
            }
            if (!py::isinstance<py::none>(py::getattr(entry, "attachment")))
            {
                thread.attachment = py::cast<std::string>(py::getattr(entry, "attachment"));
                thread.attachment_width = py::cast<int>(py::getattr(entry, "attachment_width"));
                thread.attachment_height = py::cast<int>(py::getattr(entry, "attachment_height"));
            }
            threads.push_back(thread);
            num_posts++;
        }
        return GetThreadsResult(CallbackStatus::ok, std::move(threads), num_posts);
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
        return GetThreadsResult(CallbackStatus::failed);
    }
}

GetThreadResult PythonChannel::get_thread(int client, const BoardId &board, const ThreadId &thread)
{
    py::gil_scoped_acquire acquire;

    try
    {
        auto res = m_instance.attr("get_thread")(m_client_settings[client], board, thread);
        std::vector<Post> posts;
        uint32_t num_posts = 0;
        for (auto& entry: py::list(res))
        {
            Post post;
            post.id = py::cast<std::string>(py::getattr(entry, "id"));
            if (!py::isinstance<py::none>(py::getattr(entry, "title")))
            {
                post.title = py::cast<std::string>(py::getattr(entry, "title"));
            }
            if (!py::isinstance<py::none>(py::getattr(entry, "comment")))
            {
                post.comment = py::cast<std::string>(py::getattr(entry, "comment"));
            }
            if (!py::isinstance<py::none>(py::getattr(entry, "attachment")))
            {
                post.attachment = py::cast<std::string>(py::getattr(entry, "attachment"));
                post.attachment_width = py::cast<int>(py::getattr(entry, "attachment_width"));
                post.attachment_height = py::cast<int>(py::getattr(entry, "attachment_height"));
            }
            posts.push_back(post);
            num_posts++;
        }
        return GetThreadResult(CallbackStatus::ok, std::move(posts), num_posts);
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
        return GetThreadResult(CallbackStatus::failed);
    }
}

void PythonChannel::new_client(int client)
{
    py::gil_scoped_acquire acquire;
    std::lock_guard<std::mutex> guard(m_client_settings_mutex);

    m_client_settings[client] = m_client_class(client);
}
void PythonChannel::client_released(int client)
{
    py::gil_scoped_acquire acquire;
    std::lock_guard<std::mutex> guard(m_client_settings_mutex);

    auto f = m_client_settings.find(client);
    if (f != m_client_settings.end())
    {
        m_client_settings.erase(f);
    }
}