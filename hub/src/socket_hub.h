#ifndef CHANNEL_HUB_HTTP_CHANNEL_HUB_H
#define CHANNEL_HUB_HTTP_CHANNEL_HUB_H

#include <functional>
#include <map>
#include "hub.h"

class SocketChannelHub: public ChannelHub
{
public:
    typedef std::function<std::string (int client, ChannelObject** objects, uint8_t amount, std::vector<ChannelObject*>& result)> ApiCallHandler;
public:
    SocketChannelHub(uint16_t port);
    int run() override;

private:
    void accept();
    void process_socket(int socket);

    static void* start_request(int socket, struct proto_process_t* proto);
    static void object_recv(int socket, ChannelObject* object, void* user);
    static const char* proto_process_cb(int socket, struct proto_process_t* proto, void* user);

private:

    std::map<std::string, ApiCallHandler> m_handlers;
    uint16_t m_port;
    int m_socket;
};

#endif