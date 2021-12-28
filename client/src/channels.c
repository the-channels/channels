#include "channels.h"

#include "netlog.h"
#include "proto_objects.h"
#include "channels_proto.h"
#include "system.h"

static uint16_t current_request_id = 0;
static int sockfd = -1;
static uint8_t object_num = 0;
static uint8_t notify_error = 0;
static channels_object_callback_f request_object_callback;
static channels_request_callback_f request_callback;
static channels_error_callback_f error_callback;
static char selected_channel[32] = {};
static char selected_board[32] = {};
static char selected_thread[32] = {};
static char error_str[64];

uint8_t channels_proxy_connect(const char* address, channels_disconnected_callback_f disconnected)
{
    netlog_2("Connecting: ", address);

    allocate_proto_process_struct();
    sockfd = channels_proto_connect(address, 9493, disconnected);

    if (sockfd < 0)
    {
        return 1;
    }

    netlog_1("Connected!");

    return 0;
}

void channels_proxy_disconnect()
{
    sockfd = -1;
    channels_proto_disconnect();
}

static void* channels_new_request(int socket, struct proto_process_t* proto)
{
    return NULL;
}

static void channels_object_callback(int socket, ChannelObject* object, void* user)
{
    if (object_num == 0)
    {
        ChannelObjectProperty* error = find_property(object, OBJ_PROPERTY_ERROR);
        if (error)
        {
            memcpy(error_str, error->value, error->value_size);
            error_str[error->value_size] = 0;
            notify_error = 1;
            return;
        }
    }

    object_num++;

    request_object_callback(object);
}

static const char* channels_proxy_recv(int socket, struct proto_process_t* proto, void* user)
{
    if (current_request_id != proto->request_id)
    {
        return "Unknown request id";
    }

    if (notify_error)
    {
        error_callback(error_str);
    }
    else
    {
        request_callback(proto);
    }

    return NULL;
}

void channels_proxy_update()
{
    if (sockfd < 0)
    {
        return;
    }

    struct proto_process_t* proto = get_proto_process_struct();
    channels_proto_client_process(proto,
        channels_new_request, channels_object_callback, channels_proxy_recv);
}

uint8_t channels_send_request(ChannelObject* object, channels_object_callback_f object_callback,
    channels_request_callback_f cb, channels_error_callback_f err)
{
    object_num = 0;
    notify_error = 0;
    request_callback = cb;
    request_object_callback = object_callback;
    error_callback = err;

    ChannelObject* objs[1];
    objs[0] = object;

    return channels_proto_send(sockfd, objs, 1, ++current_request_id);
}

void channels_set_channel(const char* channel)
{
    strcpy(selected_channel, channel);
}

const char* channels_get_channel()
{
    return selected_channel;
}

void channels_set_board(const char* board)
{
    strcpy(selected_board, board);
}

const char* channels_get_board()
{
    return selected_board;
}

void channels_set_thread(const char* thread)
{
    strcpy(selected_thread, thread);
}

const char* channels_get_thread()
{
    return selected_thread;
}