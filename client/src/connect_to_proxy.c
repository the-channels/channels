#include "zxgui.h"
#include "zxgui_internal.h"
#include "scenes.h"
#include "channels.h"
#include "netlog.h"
#include <string.h>
#include "logo.h"
#include <spectrum.h>

static struct gui_scene_t scene;
static struct gui_image_t logo;
static struct gui_edit_t connect_address;
static const char* supported_api_version = "2";
static char server_api_version[8] = {};

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

    memcpy(server_api_version, id->value, id->value_size);
    server_api_version[id->value_size] = 0;
}

static void api_version_response(struct proto_process_t* proto)
{
    if (proto->recv_objects_num < 1)
    {
        switch_alert("Cannot obtain API version: empty response", switch_connect_to_proxy);
        return;
    }

    if (strcmp(server_api_version, supported_api_version))
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
    netlog_init(connect_address.value, 9468);

    switch_progress("Connecting...", NULL);
    uint8_t result = channels_proxy_connect(connect_address.value, disconnected);

    if (result)
    {
        static char reason[64];
        sprintf(reason, "Cannot connect to Proxy: %d", result);
        switch_alert(reason, switch_connect_to_proxy);
    }
    else
    {
        switch_progress("Connecting...", cancel_fetching_channels);

        declare_str_property_on_stack(api, OBJ_PROPERTY_ID, "api", NULL);
        declare_object_on_stack(request, 128, &api);

        channels_send_request(request, api_version_process_object, api_version_response, api_version_error);
    }
}

void init_connect_to_proxy()
{
    zxgui_scene_init(&scene, NULL);

    static struct gui_form_t connect_to_proxy;
    zxgui_form_init(&connect_to_proxy, 8, 8, 15, 6, "Connect To Proxy", FORM_STYLE_DEFAULT);

    {
        zxgui_edit_init(&connect_address, 0, 0, 13, 2);
        zxgui_form_add_child(&connect_to_proxy, &connect_address);
    }

    {
        static struct gui_button_t button_connect;
        zxgui_button_init(&button_connect, 0, 3, 8, 1, 13, GUI_ICON_RETURN, "CONNECT", proxy_connect);
        zxgui_form_add_child(&connect_to_proxy, &button_connect);
    }

    zxgui_image_init(&logo, 10, 1, 12, 4, tiles, tile_colors);
    zxgui_scene_add(&scene, &logo);

    zxgui_scene_add(&scene, &connect_to_proxy);
}

void switch_connect_to_proxy()
{
    strcpy(connect_address.value, "127.0.0.1");

    zxgui_scene_set(&scene);
    zxgui_scene_set_focus(&scene, &connect_address);
}