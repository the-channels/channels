#ifndef CHANNELS_PROTO_H
#define CHANNELS_PROTO_H

#include <stdint.h>
#include "proto_objects.h"

#ifdef __cplusplus
extern "C" {
#endif

enum proto_process_state_t
{
    proto_process_recv_header = 0,
    proto_process_recv_object_size,
    proto_process_recv_object,
};

struct proto_process_t
{
    enum proto_process_state_t state;
    uint16_t recv_consumed;
    uint16_t recv_size;
    uint16_t request_id;
    uint16_t recv_object_size;
    uint8_t process_buffer[2048];
    uint16_t total_received;
    uint16_t total_consumed;
    uint8_t recv_objects_num;
    void* user;
};

typedef void (*disconnected_callback_f)(void);
typedef void* (*proto_start_request_callback_f)(int socket, struct proto_process_t* proto);
typedef void (*proto_next_object_callback_f)(int socket, ChannelObject* object, void* user);
typedef const char* (*proto_complete_request_callback_f)(int socket, struct proto_process_t* proto, void* user);

#ifdef CHANNELS_PROTO_CLIENT

extern int channels_proto_connect(const char* host, int port, disconnected_callback_f disconnected);
extern void channels_proto_client_process(
    proto_start_request_callback_f start_request,
    proto_next_object_callback_f next,
    proto_complete_request_callback_f complete_request);
#endif
#ifdef CHANNELS_PROTO_SERVER
extern int channels_proto_listen(int port);
extern int channels_proto_server_process(int socket, struct proto_process_t* proto,
    proto_start_request_callback_f start_request,
    proto_next_object_callback_f next,
    proto_complete_request_callback_f complete_request);
#endif

extern int channels_proto_send(int socket, ChannelObject** objects, uint8_t amount, uint16_t request_id);

#ifdef __cplusplus
}
#endif

#endif