#include "channels_proto.h"
#include "proto_objects.h"
#include "proto_asserts.h"

#ifdef WIN32
#include <Winsock2.h>
#else
#ifndef __SPECTRUM
#   include <sys/socket.h>
#   include <poll.h>
#   include <netdb.h>
#   include <arpa/inet.h>
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
void channels_proto_disconnect()
{
    if (client_socket)
    {
        sockclose(client_socket);
        client_socket = 0;
    }
}

int channels_proto_connect(const char* host, int port, disconnected_callback_f disconnected)
{
    channels_proto_disconnect();

    client_disconnected = disconnected;

    struct sockaddr_in remoteaddr;

#ifdef __SPECTRUM
    remoteaddr.sin_port = port;

    struct hostent *he = gethostbyname((char*)host);
    if(!he)
    {
        return -1;
    }

    remoteaddr.sin_addr.s_addr = (in_addr_t)he->h_addr;
#else
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_port = htons(port);
    remoteaddr.sin_addr.s_addr = inet_addr(host);
#endif

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        return -2;
    }

    if(connect(sock, (const struct sockaddr *)&remoteaddr, sizeof(struct sockaddr_in)) < 0)
    {
        sockclose(sock);
        client_disconnected = NULL;
        return -3;
    }

    client_socket = sock;

    return sock;
}
#endif

#ifdef STACKLESS_PROCESS
static int process_socket;
static struct proto_process_t* process_proto;
static proto_start_request_callback_f process_start_request;
static proto_next_object_callback_f process_next;
static proto_complete_request_callback_f process_complete_request;
#endif

static uint8_t process(
#ifndef STACKLESS_PROCESS
    int process_socket,
    struct proto_process_t* process_proto,
    proto_start_request_callback_f process_start_request,
    proto_next_object_callback_f process_next,
    proto_complete_request_callback_f process_complete_request
#endif
)
{
    process_proto->total_consumed = 0;
    uint16_t available = process_proto->total_received;
    uint8_t* data = (uint8_t*)process_proto->process_buffer;

    while (1)
    {
        switch (process_proto->state)
        {
            case proto_process_recv_header:
            {
                if (available >= 4)
                {
                    process_proto->recv_size = *(uint16_t*)data;
                    data += 2;
                    process_proto->request_id = *(uint16_t*)data;
                    data += 2;
                    process_proto->recv_consumed = 0;
                    process_proto->recv_objects_num = 0;
                    process_proto->total_consumed += 4;
                    available -= 4;
                    process_proto->state = proto_process_recv_object_size;
                    process_proto->user = process_start_request(process_socket, process_proto);
                    /* fallthrough */
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
                    process_proto->recv_object_size = *(uint16_t*)data;
                    process_proto->total_consumed += 2;
                    available -= 2;
                    data += 2;
                    process_proto->recv_consumed += 2;
                    process_proto->state = proto_process_recv_object;
                    /* fallthrough */
                }
                else
                {
                    return 0;
                }
            }
            case proto_process_recv_object:
            {
                if (available >= process_proto->recv_object_size)
                {

                    {
                        uint8_t object_buffer[128];
                        ChannelObject* obj = (ChannelObject*)object_buffer;
                        uint8_t res = channel_object_read(obj, 128, process_proto->recv_object_size, data);
                        proto_assert_str(res == 0, "Header overrun");
                        process_next(process_socket, obj, process_proto->user);
                    }

                    process_proto->recv_objects_num++;
                    process_proto->total_consumed += process_proto->recv_object_size;
                    available -= process_proto->recv_object_size;
                    data += process_proto->recv_object_size;
                    process_proto->recv_consumed += process_proto->recv_object_size;

                    if (process_proto->recv_consumed >= process_proto->recv_size)
                    {
                        proto_assert_str(process_proto->recv_objects_num, "Empty response");
                        proto_assert_str(process_proto->recv_consumed == process_proto->recv_size, "Overconsumed");

                        const char* err = process_complete_request(process_socket, process_proto, process_proto->user);

                        if (err)
                        {
                            declare_str_property_on_stack(error, OBJ_PROPERTY_ERROR, err, NULL);
                            declare_object_on_stack(response_object, 256, &error);

                            ChannelObject* objs[1];
                            objs[0] = response_object;

                            if (channels_proto_send(process_socket, objs, 1, process_proto->request_id))
                            {
                                proto_assert_str(0, "Cannot send a response");
                                return -1;
                            }
                        }

                        process_proto->state = proto_process_recv_header;
                    }
                    else
                    {
#ifdef __SPECTRUM
                        {
                            char av1[8];
                            char av2[8];
                            int_to_string((unsigned char)process_proto->recv_consumed, av1);
                            int_to_string((unsigned char)process_proto->recv_object_size, av2);
                            netlog_3("recv_object >=sz total<recv_sz ", av1, av2);
                        }
#endif

                        process_proto->state = proto_process_recv_object_size;
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

void atoh(uint8_t *ascii_ptr, char *hex_ptr,int len)
{
    const char* hex_data = "0123456789abcdef";
    for (int i = 0; i < len; i++)
    {
        char byte = ascii_ptr[i];
        *hex_ptr++ = hex_data[(byte & 0xF0) >> 4];
        *hex_ptr++ = hex_data[byte & 0x0F];
    }
}

static int recv_process(
#ifndef STACKLESS_PROCESS
    int process_socket,
    struct proto_process_t* process_proto,
    proto_start_request_callback_f process_start_request,
    proto_next_object_callback_f process_next,
    proto_complete_request_callback_f process_complete_request
#endif
)
{
    uint16_t afford = sizeof(process_proto->process_buffer) - process_proto->total_received;
    if (afford > 64) afford = 64;

    int n_received = recv(process_socket, process_proto->process_buffer + process_proto->total_received, afford, 0);
    if (n_received > 0)
    {
        process_proto->total_received += n_received;

        uint8_t res = process(
#ifndef STACKLESS_PROCESS
            process_socket, process_proto, process_start_request, process_next, process_complete_request
#endif
        );
        if (res)
        {
            return -1;
        }

        if (process_proto->total_consumed == process_proto->total_received)
        {
            process_proto->total_consumed = 0;
            process_proto->total_received = 0;
        }
        else
        {
            memmove(process_proto->process_buffer, process_proto->process_buffer + (uint16_t)process_proto->total_consumed,
                process_proto->total_received - (uint16_t)process_proto->total_consumed);
            process_proto->total_received -= (uint16_t)process_proto->total_consumed;
        }
    }
    if (n_received <= 0)
    {
        return -2;
    }
    return 0;
}

#ifdef CHANNELS_PROTO_CLIENT

void channels_proto_client_process(struct proto_process_t* proto,
    proto_start_request_callback_f start_request,
    proto_next_object_callback_f next,
    proto_complete_request_callback_f complete_request)
{
#ifdef __SPECTRUM
    int polled = poll_fd(client_socket);
    if (polled & POLLIN)
    {

#ifdef STACKLESS_PROCESS
        process_socket = client_socket;
        process_proto = proto;
        process_start_request = start_request;
        process_next = next;
        process_complete_request = complete_request;
#endif

        if (recv_process(
#ifndef STACKLESS_PROCESS
            process_socket, process_proto, process_start_request, process_next, process_complete_request
#endif
        ) < 0)
        {
            sockclose(client_socket);
            if (client_disconnected)
            {
                client_disconnected();
            }
            return;
        }
    }
    if (polled & POLLHUP)
    {
        sockclose(client_socket);
        if (client_disconnected)
        {
            client_disconnected();
        }
    }
#else
    struct pollfd fds[1];
    fds[0].fd = client_socket;
    fds[0].events = POLLIN;

    int ret = poll((struct pollfd*)&fds, 1, 1);

    if (ret == -1)
    {
        return;
    }

    if ( fds[0].revents & POLLIN )
    {
#ifdef STACKLESS_PROCESS
        process_socket = client_socket;
        process_proto = proto;
        process_start_request = start_request;
        process_next = next;
        process_complete_request = complete_request;
#endif

        if (recv_process(
#ifndef STACKLESS_PROCESS
            client_socket, proto, start_request, next, complete_request
#endif
        ) < 0)
        {
            sockclose(client_socket);
            if (client_disconnected)
            {
                client_disconnected();
            }
            return;
        }
    }

    if ( fds[0].revents & POLLHUP )
    {
        sockclose(client_socket);
        if (client_disconnected)
        {
            client_disconnected();
        }
    }

    fds[0].revents = 0;

#endif
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


#ifdef COMBINE_OBJECTS_UPON_SENDING
    int write_size = 4;

    for (uint8_t i = 0; i < amount; i++)
    {
        ChannelObject* object = objects[i];
        write_size += object->object_size + 2;
    }

    uint8_t* data = malloc(write_size);

    memcpy(data, (uint8_t*)&req_size, 2);
    memcpy(data + 2, (uint8_t*)&request_id, 2);
    write_size = 4;

    for (uint8_t i = 0; i < amount; i++)
    {
        ChannelObject* object = objects[i];
        memcpy(data + write_size, (void*)channel_object_data(object), object->object_size + 2);
        write_size += object->object_size + 2;
    }

    if (send(socket, (void*)data, write_size, 0) < 0)
    {
        free(data);
        return 1;
    }

    free(data);
#else

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
#endif

    return 0;
}