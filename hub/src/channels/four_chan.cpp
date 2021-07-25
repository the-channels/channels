#include "four_chan.h"
#include <cpr/cpr.h>
#include "json.hpp"
#include <iostream>

std::string FourChan::s_base_url = "https://a.4cdn.org";

FourChan::FourChan() :
    Channel("4chan", "4chan.org")
{
}

CallbackStatus FourChan::get_attachment(const BoardId &board, const ThreadId &thread, const PostId &post,
    const std::string &attachment, uint32_t width, uint32_t height, std::string& fout)
{
    fout = cache_key(board + "_" + attachment);

    {
        std::ifstream f(fout);
        if (f.good())
        {
            return CallbackStatus::ok;
        }
    }

    auto session = cpr::Session();
    session.SetUrl(cpr::Url{"https://i.4cdn.org/" + board + "/" + attachment});

    std::ofstream f(fout, std::ios_base::out | std::ios_base::binary);
    auto r = session.Download(f);

    if (r.status_code != 200)
    {
        std::cerr << "Error:" << r.status_code << std::endl;
        return CallbackStatus::failed;
    }

    return CallbackStatus::ok;
}

GetBoardsResult FourChan::get_boards(uint32_t limit)
{
    cpr::Response r = cpr::Get(cpr::Url{s_base_url + "/boards.json"});

    if (r.status_code != 200)
    {
        return GetBoardsResult(CallbackStatus::failed, std::vector<class Board>());
    }

    auto parsed = nlohmann::json::parse(r.text);

    std::vector<Board> boards;

    for (auto& board: parsed["boards"])
    {
        if (!board.contains("board"))
            continue;
        if (!board.contains("title"))
            continue;
        boards.push_back({ board["board"], board["title"] });
        if (--limit == 0)
        {
            break;
        }
    }

    return GetBoardsResult(CallbackStatus::ok, std::move(boards));
}

GetThreadsResult FourChan::get_threads(const BoardId& board)
{
    cpr::Response r = cpr::Get(cpr::Url{s_base_url + "/" + board + "/catalog.json"});

    if (r.status_code != 200)
    {
        return GetThreadsResult(CallbackStatus::failed);
    }

    auto parsed = nlohmann::json::parse(r.text);
    std::vector<Thread> threads;

    int i = 0;

    for (auto page: parsed)
    {
        for (auto thread: page["threads"])
        {
            if (!thread.contains("no"))
                continue;

            if (!thread.contains("com"))
                continue;

            i++;

            Thread th;

            th.id = std::to_string((int)thread["no"]);

            if (thread.contains("sub"))
            {
                th.title = thread["sub"];
            }

            if (thread.contains("tim") && thread.contains("ext") && thread.contains("w") && thread.contains("h"))
            {
                th.attachment = std::to_string((uint64_t)thread["tim"]) + std::string(thread["ext"]);

                th.attachment_width = thread["w"];
                th.attachment_height = thread["h"];
            }

            th.comment = preprocess_html(std::string(thread["com"]));

            threads.push_back(std::move(th));
        }
    }

    uint32_t sz = threads.size();
    return GetThreadsResult(CallbackStatus::ok, std::move(threads), sz);
}

GetThreadResult FourChan::get_thread(const BoardId& board, const ThreadId& thread)
{
    cpr::Response r = cpr::Get(cpr::Url{s_base_url + "/" + board + "/thread/" + thread + ".json"});

    if (r.status_code != 200)
    {
        return GetThreadResult(CallbackStatus::failed);
    }

    auto parsed = nlohmann::json::parse(r.text);
    std::vector<Post> posts;

    int i = 0;

    for (auto post: parsed["posts"])
    {
        if (!post.contains("no"))
            continue;

        if (!post.contains("com"))
            continue;

        i++;

        Post ps;

        ps.id = std::to_string((int)post["no"]);

        if (post.contains("sub"))
        {
            ps.title = post["sub"];
        }

        if (post.contains("tim") && post.contains("ext") && post.contains("w") && post.contains("h"))
        {
            ps.attachment = std::to_string((uint64_t)post["tim"]) + std::string(post["ext"]);

            ps.attachment_width = post["w"];
            ps.attachment_height = post["h"];
        }

        ps.comment = preprocess_html(std::string(post["com"]));

        posts.push_back(std::move(ps));
    }

    uint32_t sz = posts.size();
    return GetThreadResult(CallbackStatus::ok, std::move(posts), sz);
}