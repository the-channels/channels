#include "python_channel.h"
#include "setting_def.h"
#include <iostream>

PythonChannel::PythonChannel(const std::string& id, const std::string& title, const py::object& clazz) :
    Channel(id, title)
{
    // allocate a instance
    m_instance = clazz();
    m_instance.inc_ref();

    m_client_class = getattr(py::module_::import("channels.base"), "Client");
}

CallbackStatus PythonChannel::get_attachment(int client, const std::string& url, std::string& fout)
{
    py::gil_scoped_acquire acquire;

    try
    {
        auto res = m_instance.attr("get_attachment")(m_client_settings[client], url);
        fout = py::cast<std::string>(res);
        return CallbackStatus(CallbackStatus::ok);
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
        return CallbackStatus(CallbackStatus::failed);
    }
}

GetChannelBoardsResult PythonChannel::get_boards(int client, uint32_t limit)
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
        return GetChannelBoardsResult(CallbackStatus::ok, std::move(boards));
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
        return GetChannelBoardsResult(CallbackStatus::failed, std::vector<Board>());
    }
}

GetSettingDefsResult PythonChannel::get_setting_defs(int client)
{
    py::gil_scoped_acquire acquire;

    try
    {
        auto res = m_instance.attr("get_setting_definitions")(m_client_settings[client]);
        std::vector<SettingDef> defs;
        for (auto& entry: py::list(res))
        {
            SettingDef def;
            def.id = py::cast<std::string>(py::getattr(entry, "id"));
            def.description = py::cast<std::string>(py::getattr(entry, "description"));
            def.current_value = py::cast<std::string>(py::getattr(m_client_settings[client], "get_option")(def.id));
            defs.push_back(def);
        }
        return GetSettingDefsResult(CallbackStatus::ok, std::move(defs));
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
        return GetSettingDefsResult(CallbackStatus::failed, std::vector<SettingDef>());
    }
}

void PythonChannel::set_settings(int client, const std::unordered_map<std::string, std::string>& s)
{
    py::gil_scoped_acquire acquire;

    try
    {
        auto sett = m_client_settings[client];

        for (const auto& it: s)
        {
            sett.attr("set_option")(it.first, it.second);
        }

        sett.attr("save_options")();
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

GetChannelThreadsResult PythonChannel::get_threads(int client, const BoardId &board)
{
    py::gil_scoped_acquire acquire;

    try
    {
        auto res = m_instance.attr("get_threads")(m_client_settings[client], board);
        std::vector<Thread> threads;
        uint32_t num_posts = 0;
        for (auto& entry: py::list(res))
        {
            Thread thread = {};
            thread.id = py::cast<std::string>(py::getattr(entry, "id"));
            if (!py::isinstance<py::none>(py::getattr(entry, "title")))
            {
                thread.title = py::cast<std::string>(py::getattr(entry, "title"));
            }
            if (!py::isinstance<py::none>(py::getattr(entry, "comment")))
            {
                thread.comment = py::cast<std::string>(py::getattr(entry, "comment"));
            }
            if (!py::isinstance<py::none>(py::getattr(entry, "attachments")))
            {
                for (auto& att: py::list(py::getattr(entry, "attachments")))
                {
                    if (py::isinstance<py::none>(py::getattr(att, "url")))
                        continue;

                    thread.attachments.emplace_back(py::cast<std::string>(py::getattr(att, "url")));
                }
            }
            if (py::isinstance<py::int_>(py::getattr(entry, "num_replies")))
            {
                thread.num_replies = py::cast<int>(py::getattr(entry, "num_replies"));
            }
            threads.push_back(thread);
            num_posts++;
        }
        return GetChannelThreadsResult(CallbackStatus::ok, std::move(threads), num_posts);
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
        return GetChannelThreadsResult(CallbackStatus::failed);
    }
}

GetChannelThreadResult PythonChannel::get_thread(int client, const BoardId &board, const ThreadId &thread)
{
    py::gil_scoped_acquire acquire;

    try
    {
        auto res = m_instance.attr("get_thread")(m_client_settings[client], board, thread);
        std::vector<Post> posts;
        uint32_t num_posts = 0;
        for (auto& entry: py::list(res))
        {
            Post post = {};
            post.id = py::cast<std::string>(py::getattr(entry, "id"));
            if (!py::isinstance<py::none>(py::getattr(entry, "title")))
            {
                post.title = py::cast<std::string>(py::getattr(entry, "title"));
            }
            if (!py::isinstance<py::none>(py::getattr(entry, "comment")))
            {
                post.comment = py::cast<std::string>(py::getattr(entry, "comment"));
            }
            if (!py::isinstance<py::none>(py::getattr(entry, "attachments")))
            {
                for (auto& att: py::list(py::getattr(entry, "attachments")))
                {
                    if (py::isinstance<py::none>(py::getattr(att, "url")))
                        continue;

                    post.attachments.emplace_back(py::cast<std::string>(py::getattr(att, "url")));
                }
            }
            if (py::isinstance<py::list>(py::getattr(entry, "replies")))
            {
                post.replies.clear();

                for (auto reply: py::cast<py::list>(py::getattr(entry, "replies")))
                {
                    post.replies.push_back(py::cast<std::string>(reply));
                }
            }
            posts.push_back(post);
            num_posts++;
        }
        return GetChannelThreadResult(CallbackStatus::ok, std::move(posts), num_posts);
    }
    catch (py::error_already_set &e)
    {
        std::cerr << e.what() << std::endl;
        return GetChannelThreadResult(CallbackStatus::failed);
    }
}

void PythonChannel::set_key(int client, const std::string& key)
{
    py::gil_scoped_acquire acquire;
    std::lock_guard<std::mutex> guard(m_client_settings_mutex);

    auto it = m_client_settings.find(client);
    if (it != m_client_settings.end())
    {
        try
        {
            py::getattr(it->second, "set_key")(py::bytes(key));
        }
        catch (py::error_already_set &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

void PythonChannel::new_client(int client)
{
    py::gil_scoped_acquire acquire;
    std::lock_guard<std::mutex> guard(m_client_settings_mutex);

    m_client_settings[client] = m_client_class(get_name(), client);
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