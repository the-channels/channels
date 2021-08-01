#include <sstream>
#include "socket_hub.h"
#include "proto_objects.h"
#include "channels_proto.h"
#ifdef WIN32
#include <Winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <thread>
#include <iostream>
#include <set>
#include <functional>

static const std::string protocol_version = "3";

static std::string get_string_property(ChannelObject* o, uint8_t key)
{
    ChannelObjectProperty* prop = find_property(o, key);

    if (prop)
    {
        return std::string(prop->value, prop->value_size);
    }

    return "";
}

SocketChannelHub::SocketChannelHub(uint16_t port) :
    ChannelHub(), m_port(port)
{
    m_handlers.emplace("api", [](int client, ChannelObject** objects, uint8_t amount, std::vector<ChannelObject*>& result)
        -> const char*
    {
        declare_str_property_on_stack(id_, OBJ_PROPERTY_ID, protocol_version.c_str(), nullptr);

        result.push_back(channel_object_allocate(&id_));
        return nullptr;
    });

    m_handlers.emplace("channels", [](int client, ChannelObject** objects, uint8_t amount, std::vector<ChannelObject*>& result)
        -> const char*
    {
        for (auto& channel: Get()->get_channels())
        {
            result.push_back(channel.second->write());
        }

        return nullptr;
    });

    m_handlers.emplace("boards", [](int client, ChannelObject** objects, uint8_t amount, std::vector<ChannelObject*>& result)
        -> const char*
    {
        if (amount == 0)
        {
            return "channel is not specified";
        }

        ChannelObject* channel_object = objects[0];

        ChannelId channel = get_string_property(channel_object, 'c');
        if (channel.empty())
        {
            return "Missing channel";
        }

        uint16_t limit = get_uint16_property(channel_object, 'l', 128);
        auto res = Get()->get_boards(client, channel, limit);

        if (res.first != CallbackStatus::ok)
        {
            return "Cannot obtain boards";
        }

        for (auto& board: res.second)
        {
            result.push_back(board.write());
        }

        return nullptr;
    });

    m_handlers.emplace("threads", [](int client, ChannelObject** objects, uint8_t amount, std::vector<ChannelObject*>& result)
        -> const char*
    {
        if (amount == 0)
        {
            return "channel is not specified";
        }

        ChannelObject* channel_object = objects[0];

        ChannelId channel = get_string_property(channel_object, 'c');
        if (channel.empty())
        {
            return "Missing channel";
        }

        BoardId board = get_string_property(channel_object, 'b');
        if (board.empty())
        {
            return "Missing board";
        }

        bool flush = get_uint8_property(channel_object, 'f', 0);
        uint16_t limit = get_uint16_property(channel_object, 'l', 128);
        uint16_t offset = get_uint16_property(channel_object, 'o', 0);

        auto res = Get()->get_threads(client, channel, board, flush);

        if (res.status != CallbackStatus::ok)
        {
            return "Cannot obtain boards";
        }

        {
            std::string threads = std::to_string(res.num_threads);
            declare_str_property_on_stack(id_, 'c', threads.c_str(), nullptr);

            result.push_back(channel_object_allocate(&id_));
        }

        if (offset < res.threads->size())
        {
            auto end = limit + offset < res.threads->size() ? res.threads->begin() + limit + offset : res.threads->end();

            for (auto ptr = res.threads->begin() + offset; ptr < end; ptr++)
            {
                result.push_back(ptr->write());
            }
        }

        return nullptr;
    });

    m_handlers.emplace("image", [](int client, ChannelObject** objects, uint8_t amount, std::vector<ChannelObject*>& result)
        -> const char*
    {
        if (amount == 0)
        {
            return "channel is not specified";
        }

        ChannelObject* channel_object = objects[0];

        ChannelId channel = get_string_property(channel_object, 'c');
        if (channel.empty())
        {
            return "Missing channel";
        }

        BoardId board = get_string_property(channel_object, 'b');
        if (board.empty())
        {
            return "Missing board";
        }

        ThreadId thread = get_string_property(channel_object, 't');
        if (thread.empty())
        {
            return "Missing thread";
        }

        PostId post = get_string_property(channel_object, 'p');
        if (post.empty())
        {
            return "Missing post";
        }

        std::string encoding_name = get_string_property(channel_object, 'e');
        if (encoding_name.empty())
        {
            return "Missing encoding";
        }

        uint16_t target_w = get_uint16_property(channel_object, 'w', 0);
        if (target_w == 0)
        {
            return "Missing target width";
        }

        uint16_t target_h = get_uint16_property(channel_object, 'h', 0);
        if (target_h == 0)
        {
            return "Missing target height";
        }

        ImageProcessing::ImageEncoding encoding = ImageProcessing::parse_encoding(encoding_name);
        if (encoding == ImageProcessing::ImageEncoding::unknown)
        {
            return "Unknown encoding";
        }

        auto res = Get()->get_image(client, channel, board, thread, post, target_w, target_h, encoding);

        if (res.status != CallbackStatus::ok || res.data == nullptr)
        {
            return "Cannot obtain image";
        }

        {
            declare_arg_property_on_stack(w_, 'w', res.w, nullptr);
            declare_arg_property_on_stack(h_, 'h', res.h, &w_);
            declare_arg_property_on_stack(data_, 's', res.data_size, &h_);

            result.push_back(channel_object_allocate(&data_));
        }

        const uint8_t* d = res.data->data();
        uint16_t l = res.data_size;

        while (l)
        {
            uint16_t size = l > 1024 ? 1024 : l;
            declare_variable_property_on_stack(p, OBJ_PROPERTY_PAYLOAD, d, size, nullptr);
            result.push_back(channel_object_allocate(&p));

            d += size;
            l -= size;
        }

        return nullptr;
    });

    m_handlers.emplace("thread", [](int client, ChannelObject** objects, uint8_t amount, std::vector<ChannelObject*>& result)
        -> const char*
    {
        if (amount == 0)
        {
            return "channel is not specified";
        }

        ChannelObject* channel_object = objects[0];

        ChannelId channel = get_string_property(channel_object, 'c');
        if (channel.empty())
        {
            return "Missing channel";
        }

        BoardId board = get_string_property(channel_object, 'b');
        if (board.empty())
        {
            return "Missing board";
        }

        ThreadId thread = get_string_property(channel_object, 't');
        if (thread.empty())
        {
            return "Missing thread";
        }

        PostId replies_to = get_string_property(channel_object, 'r');
        bool flush = get_uint8_property(channel_object, 'f', 0);
        uint16_t limit = get_uint16_property(channel_object, 'l', 128);
        uint16_t offset = get_uint16_property(channel_object, 'o', 0);

        auto res = Get()->get_thread(client, channel, board, thread, flush);

        if (res.status != CallbackStatus::ok)
        {
            return "Cannot obtain boards";
        }

        if (!replies_to.empty())
        {
            std::set<PostId> post_filter;

            for (auto& th: *res.posts)
            {
                if (th.id == replies_to)
                {
                    for (auto& reply: th.replies)
                    {
                        post_filter.insert(reply);
                    }
                    break;
                }
            }

            std::vector<const Post*> slice;

            for (auto& th: *res.posts)
            {
                if (post_filter.find(th.id) == post_filter.end())
                {
                    continue;
                }

                slice.push_back(&th);
            }

            {
                std::string posts = std::to_string(slice.size());
                declare_str_property_on_stack(id_, 'c', posts.c_str(), nullptr);

                result.push_back(channel_object_allocate(&id_));
            }

            if (offset < slice.size())
            {
                auto end = limit + offset < slice.size() ? slice.begin() + limit + offset : slice.end();

                for (auto ptr = slice.begin() + offset; ptr < end; ptr++)
                {
                    result.push_back((*ptr)->write());
                }
            }
        }
        else
        {
            {
                std::string posts = std::to_string(res.num_posts);
                declare_str_property_on_stack(id_, 'c', posts.c_str(), nullptr);

                result.push_back(channel_object_allocate(&id_));
            }

            if (offset < res.posts->size())
            {
                auto end = limit + offset < res.posts->size() ? res.posts->begin() + limit + offset : res.posts->end();

                for (auto ptr = res.posts->begin() + offset; ptr < end; ptr++)
                {
                    result.push_back(ptr->write());
                }
            }
        }

        return nullptr;
    });

}

void* SocketChannelHub::start_request(int socket, struct proto_process_t* proto)
{
    return new std::vector<ChannelObject*>();
}

void SocketChannelHub::object_recv(int socket, ChannelObject* object, void* user)
{
    auto* objects = (std::vector<ChannelObject*>*)user;
    objects->push_back(channel_object_copy(object));
};

const char* SocketChannelHub::proto_process_cb(int socket, struct proto_process_t* proto, void* user)
{
    auto* objects = (std::vector<ChannelObject*>*)user;

    ChannelObjectProperty* id;
    if (objects->size() > 0)
    {
        id = find_property((*objects)[0], OBJ_PROPERTY_ID);
    }
    else
    {
        delete objects;
        return "Expected ID";
    }

    if (!id)
    {
        delete objects;
        return "Unknown ID";
    }

    std::string api_call(id->value, id->value_size);
    std::cout << "Request: " << api_call << " (" << proto->request_id << ")" << std::endl;


    if (IsVerbose())
    {
        uint8_t i = 0;

        ChannelObjectProperty** pps = (*objects)[0]->properties;
        while (*pps)
        {
            if ((*pps)->key == OBJ_PROPERTY_PAYLOAD)
            {
                std::cout << "    " << std::to_string((*pps)->key) << " (payload)" << std::endl;
            }
            else
            {
                std::cout << "    " << std::to_string((*pps)->key) << " " << std::string((*pps)->value, (*pps)->value + (*pps)->value_size) << std::endl;
            }
            pps++;
        }
    }


    SocketChannelHub* hub = static_cast<SocketChannelHub*>(Get());

    auto it = hub->m_handlers.find(api_call);
    if (it == hub->m_handlers.end())
    {
        delete objects;
        std::cout << "Unknown handler" << std::endl;
        return "Unknown ID";
    }

    std::vector<ChannelObject*> send_back;
    const char* err = it->second(socket, objects->data(), proto->recv_objects_num, send_back);

    delete objects;

    if (err)
    {
        std::cout << "Responding with error: " << err << std::endl;
        return err;
    }

    if (!send_back.empty())
    {

        if (IsVerbose())
        {
            std::cout << "Responding with " << send_back.size() << " object(s):" << std::endl;
            uint8_t i = 0;

            for (ChannelObject* o: send_back)
            {
                std::cout << "  Object " << std::to_string(i++) << " (" << o->object_size << " b):" << std::endl;
                ChannelObjectProperty** pps = o->properties;
                while (*pps)
                {
                    if ((*pps)->key == OBJ_PROPERTY_PAYLOAD)
                    {
                        std::cout << "    " << std::to_string((*pps)->key) << " of " << (*pps)->value_size << " (payload)" << std::endl;
                    }
                    else
                    {
                        std::cout << "    " << std::to_string((*pps)->key) << " of " << (*pps)->value_size << " " << std::string((*pps)->value, (*pps)->value + (*pps)->value_size) << std::endl;
                    }
                    pps++;
                }
            }
        }
        else
        {
            std::cout << "Responding with " << send_back.size() << " object(s)" << std::endl;
        }

        channels_proto_send(socket, send_back.data(), send_back.size(), proto->request_id);

        for (ChannelObject* o: send_back)
        {
            free(o);
        }

        send_back.clear();
    }

    return nullptr;
};


void SocketChannelHub::process_socket(int socket)
{
    std::cout << "New client connected: " << socket << std::endl;

    new_client(socket);

    auto& handlers = m_handlers;
    struct proto_process_t proto = {};

    while (1)
    {
        if (channels_proto_server_process(socket, &proto, start_request, object_recv, proto_process_cb) < 0)
        {
            break;
        }

        std::this_thread::yield();
    }

    std::cout << "Client disconnected: " << socket << std::endl;

    client_released(socket);
}

void SocketChannelHub::accept()
{
    struct sockaddr_in clientname;
#ifdef WIN32
    int size;
#else
    socklen_t size;
#endif
    size = sizeof(clientname);
    int sock = ::accept(m_socket, (struct sockaddr *) &clientname, &size);

    if (sock < 0)
    {
        std::cout << "Accept error: " << sock << std::endl;
        return;
    }

    if (IsDebug())
    {
        std::cout << "Debugging mode, so running client on main thread " << sock << std::endl;
        process_socket(sock);
    }
    else
    {
        std::thread connection_thread([this, sock]
        {
            process_socket(sock);
        });
        connection_thread.detach();
    }

}

int SocketChannelHub::run()
{
    m_socket = channels_proto_listen(m_port);
    if (m_socket < 0)
    {
        std::cout << "Cannot listen port " << m_port << ": " << m_socket << std::endl;
        return m_socket;
    }

    std::cout << "Listening on port: " << m_port << std::endl;

    py::gil_scoped_release guard{};

    while (1)
    {
        accept();
    }
}