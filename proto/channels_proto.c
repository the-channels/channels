#include "channels_proto.h"
#include "proto_objects.h"
#include "proto_asserts.h"

#ifdef WIN32
#include <Winsock2.h>
#else
#ifndef __SPECTRUM
#   include <sys/socket.h>
#   include <netdb.h>
#endif
#endif

#include <string.h>
#include <stdlib.h>

#ifdef __SPECTRUM
#include "system.h"
#else
#ifndef WIN32
#   include <unistd.h>
#endif
#define sockclose close
#endif

#ifdef CHANNELS_PROTO_CLIENT
static int client_socket = 0;
static disconnected_callback_f client_disconnected = NULL;
static struct proto_process_t client_proto = {};
#endif

#ifdef CHANNELS_PROTO_SERVER
int channels_proto_listen(int port)
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
#ifdef WIN32
        return - WSAGetLastError();
#else
        return socketfd;
#endif
    }

    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if(bind(socketfd, (const struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            sockclose(socketfd);
            return -2000;
        }
    }

    if(listen(socketfd, 1) < 0)
    {
        sockclose(socketfd);
        return -3000;
    }

    return socketfd;
}
#endif

#ifdef CHANNELS_PROTO_CLIENT
int channels_proto_connect(const char* host, int port, disconnected_callback_f disconnected)
{
    if (client_socket)
    {
        sockclose(client_socket);
        client_socket = 0;
    }

    client_disconnected = disconnected;

    struct sockaddr_in remoteaddr;
    struct hostent *he = gethostbyname((char*)host);
    if(!he)
    {
        return -1;
    }

    int socket = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0)
    {
        return -2;
    }

    remoteaddr.sin_port = port;
    remoteaddr.sin_addr.s_addr = he->h_addr;

    if(connect(socket, &remoteaddr, sizeof(struct sockaddr_in)) < 0)
    {
        sockclose(socket);
        return -3;
    }

    client_socket = socket;

    return socket;
}
#endif

static uint8_t process(int socket, struct proto_process_t* proto,
    proto_start_request_callback_f start_request,
    proto_next_object_callback_f next,
    proto_complete_request_callback_f complete_request)
{
    while (1)
    {
        uint16_t available = proto->total_received - proto->total_consumed;
        uint8_t* data = proto->process_buffer + proto->total_consumed;

        switch (proto->state)
        {
            case proto_process_recv_header:
            {
                if (available >= 4)
                {
                    proto->recv_size = *(uint16_t*)data;
                    data += 2;
                    proto->request_id = *(uint16_t*)data;
                    proto->recv_consumed = 0;
                    proto->recv_objects_num = 0;
                    proto->total_consumed += 4;
                    proto->state = proto_process_recv_object_size;
                    proto->user = start_request(socket, proto);
                    break;
                }
                else
                {
                    return 0;
                }
            }
            case proto_process_recv_object_size:
            {
                if (available >= sizeof(uint16_t))
                {
                    proto->recv_object_size = *(uint16_t*)data;
                    proto->total_consumed += 2;
                    proto->recv_consumed += 2;
                    proto->state = proto_process_recv_object;
                    break;
                }
                else
                {
                    return 0;
                }
            }
            case proto_process_recv_object:
            {
                if (available >= proto->recv_object_size)
                {
                    {
                        uint8_t object_buffer[128];
                        ChannelObject* obj = (ChannelObject*)object_buffer;
                        channel_object_read(obj, 128, proto->recv_object_size, data);
                        next(socket, obj, proto->user);
                    }

                    proto->recv_objects_num++;
                    proto->total_consumed += proto->recv_object_size;
                    proto->recv_consumed += proto->recv_object_size;

                    if (proto->recv_consumed >= proto->recv_size)
                    {
                        proto_assert_str(proto->recv_objects_num, "Empty response");
                        proto_assert(proto->recv_consumed == proto->recv_size, "Overconsumed: %d out of %d",
                            proto->recv_consumed, proto->recv_size);

                        const char* err = complete_request(socket, proto, proto->user);

                        if (err)
                        {
                            declare_str_property_on_stack(error, OBJ_PROPERTY_ERROR, err, NULL);
                            declare_object_on_stack(response_object, 128, &error);

                            ChannelObject* objs[1];
                            objs[0] = response_object;

                            if (channels_proto_send(socket, objs, 1, proto->request_id))
                            {
                                return 1;
                            }
                        }

                        proto->state = proto_process_recv_header;
                    }
                    else
                    {
                        proto->state = proto_process_recv_object_size;
                    }
                    break;
                }
                else
                {
                    return 0;
                }
            }
        }
    }
}

static int recv_process(int socket, struct proto_process_t* proto,
    proto_start_request_callback_f start_request,
    proto_next_object_callback_f next,
    proto_complete_request_callback_f complete_request)
{
    uint16_t afford = sizeof(proto->process_buffer) - proto->total_received;

    if (afford == 0)
    {
        proto_assert_str(proto->total_consumed > 0, "Cannot afford anything nor cannot move");

        memmove(proto->process_buffer, proto->process_buffer + proto->total_consumed, proto->total_received - proto->total_consumed);
        proto->total_received -= proto->total_consumed;
        proto->total_consumed = 0;

        return 0;
    }

    int n_received = recv(socket, proto->process_buffer + proto->total_received, afford, 0);
    if (n_received > 0)
    {
        proto->total_received += n_received;
        if (process(socket, proto, start_request, next, complete_request))
        {
            return -1;
        }
        if (proto->total_consumed == proto->total_received)
        {
            proto->total_consumed = 0;
            proto->total_received = 0;
        }
        else if (proto->total_consumed >= sizeof(proto->process_buffer) / 2)
        {
            memmove(proto->process_buffer, proto->process_buffer + proto->total_consumed, proto->total_received - proto->total_consumed);
            proto->total_received -= proto->total_consumed;
            proto->total_consumed = 0;
        }
    }
    if (n_received <= 0)
    {
        return -2;
    }
    return 0;
}

#ifdef CHANNELS_PROTO_CLIENT

void channels_proto_client_process(proto_start_request_callback_f start_request,
    proto_next_object_callback_f next,
    proto_complete_request_callback_f complete_request)
{
    int polled = poll_fd(client_socket);
    if (polled & POLLIN)
    {
        if (recv_process(client_socket, &client_proto, start_request, next, complete_request) < 0)
        {
            sockclose(client_socket);
            client_disconnected();
            return;
        }
    }
    if (polled & POLLHUP)
    {
        sockclose(client_socket);
        client_disconnected();
    }
}
#endif

#ifdef CHANNELS_PROTO_SERVER
int channels_proto_server_process(int socket, struct proto_process_t* proto,
    proto_start_request_callback_f start_request,
    proto_next_object_callback_f next,
    proto_complete_request_callback_f complete_request)
{
    return recv_process(socket, proto, start_request, next, complete_request);
}
#endif

int channels_proto_send(int socket, ChannelObject** objects, uint8_t amount, uint16_t request_id)
{
    uint16_t req_size = 0;

    for (uint8_t i = 0; i < amount; i++)
    {
        req_size += objects[i]->object_size + 2;
    }

    uint8_t header[4];
    memcpy(header, (uint8_t*)&req_size, 2);
    memcpy(header + 2, (uint8_t*)&request_id, 2);

    if (send(socket, (void*)header, 4, 0) < 0)
    {
        return 1;
    }

    for (uint8_t i = 0; i < amount; i++)
    {
        ChannelObject* object = objects[i];

        if (send(socket, (void*)channel_object_data(object), object->object_size + 2, 0) < 0)
        {
            return 3;
        }
    }

    return 0;
}