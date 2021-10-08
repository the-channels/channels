#include "zxgui.h"
#include "scenes.h"
#include "channels.h"
#include "netlog.h"
#include <string.h>
#include <stdlib.h>
#include "logo.h"
#include "heap.h"
#include <spectrum.h>
#include <spectranet.h>
#include "version.h"

struct scene_objects_t
{
    struct gui_scene_t scene;
    struct gui_image_t logo;
    struct gui_edit_t connect_address;
    char connect_address_buff[256];
    struct gui_form_t connect_to_proxy;
    struct gui_button_t button_connect;
    struct gui_label_t label_version;
    char server_api_version[8];
};

static char connect_to_address[64];

#define SUPPORTED_API_VERSION "5"

static struct scene_objects_t* scene_objects = NULL;

static void disconnected()
{
    switch_alert("Disconnected from Proxy", switch_connect_to_proxy);
}

static void cancel_fetching_channels()
{
    switch_alert("Disconnected: cancelled", switch_connect_to_proxy);
}

static void api_version_process_object(ChannelObject* object)
{
    ChannelObjectProperty* id = find_property(object, OBJ_PROPERTY_ID);
    if (id == NULL)
    {
        return;
    }

    memcpy(scene_objects->server_api_version, id->value, id->value_size);
    scene_objects->server_api_version[id->value_size] = 0;
}

static void api_version_response(struct proto_process_t* proto)
{
    if (proto->recv_objects_num < 1)
    {
        switch_alert("Cannot obtain API version: empty response", switch_connect_to_proxy);
        return;
    }

    if (strcmp(scene_objects->server_api_version, SUPPORTED_API_VERSION))
    {
        switch_alert("Not supported API version", switch_connect_to_proxy);
        return;
    }

    switch_select_channel();
}

static void api_version_error(const char* error)
{
    switch_alert(error, switch_connect_to_proxy);
}

static void proxy_connect(struct gui_button_t* this)
{
    switch_progress("Connecting...", NULL);
    uint8_t result = channels_proxy_connect(scene_objects->connect_address.value, disconnected);

    if (result)
    {
        switch_alert("Cannot connect to Proxy", switch_connect_to_proxy);
    }
    else
    {
        switch_progress("Connecting...", cancel_fetching_channels);

        char key[6];
        gethwaddr(key);

        declare_str_property_on_stack(api_, OBJ_PROPERTY_ID, "api", NULL);
        declare_variable_property_on_stack(key_, 'k', key, 6, &api_);
        declare_object_on_stack(request, 128, &key_);

        channels_send_request(request, api_version_process_object, api_version_response, api_version_error);
    }
}

void set_connect_to_proxy_address(const char* address)
{
    strcpy(connect_to_address, address);
}

void switch_connect_to_proxy()
{
    channels_proxy_disconnect();

    reset_heap();
    scene_objects = alloc_heap(sizeof (struct scene_objects_t));

    zxgui_scene_init(&scene_objects->scene, NULL);

    zxgui_form_init(&scene_objects->connect_to_proxy, XYWH(8, 8, 15, 6), "Connect To Proxy", FORM_STYLE_DEFAULT);

    {
        strcpy(scene_objects->connect_address_buff, "");
        zxgui_edit_init(&scene_objects->connect_address, XYWH(0, 0, 13, 2), scene_objects->connect_address_buff, 64);
        strcpy(scene_objects->connect_address.value, connect_to_address);
        zxgui_form_add_child(&scene_objects->connect_to_proxy, &scene_objects->connect_address);
    }

    {
        zxgui_button_init(&scene_objects->button_connect, XYWH(0, 3, 8, 1), 13, GUI_ICON_RETURN, "CONNECT", proxy_connect);
        zxgui_form_add_child(&scene_objects->connect_to_proxy, &scene_objects->button_connect);
    }

    zxgui_image_init(&scene_objects->logo, XYWH(10, 1, 12, 4), tiles, tile_colors);
    zxgui_scene_add(&scene_objects->scene, &scene_objects->logo);

    zxgui_scene_add(&scene_objects->scene, &scene_objects->connect_to_proxy);

    zxgui_label_init(&scene_objects->label_version, XYWH(26, 23, 6, 1), CHANNELS_VERSION, INK_WHITE | PAPER_BLACK, 0);
    zxgui_scene_add(&scene_objects->scene, &scene_objects->label_version);

    zxgui_scene_set_focus(&scene_objects->scene, &scene_objects->connect_address);
    zxgui_scene_set(&scene_objects->scene);
}