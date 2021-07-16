#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>
#include "channels_proto.h"
#include "proto_objects.h"

typedef void (*channels_disconnected_callback_f)(void);
typedef void (*channels_request_callback_f)(struct proto_process_t* proto);
typedef void (*channels_object_callback_f)(ChannelObject* object);
typedef void (*channels_error_callback_f)(const char* error);

extern uint8_t channels_proxy_connect(const char* address, channels_disconnected_callback_f disconnected);
extern void channels_proxy_update();
extern void channels_set_channel(const char* channel);
extern const char* channels_get_channel();
extern void channels_set_board(const char* board);
extern const char* channels_get_board();
extern void channels_set_thread(const char* thread);
extern const char* channels_get_thread();
extern uint8_t channels_send_request(
    ChannelObject* object, channels_object_callback_f object_callback,
    channels_request_callback_f cb, channels_error_callback_f err);

#endif