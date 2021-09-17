#include "zxgui.h"
#include "scenes.h"
#include "system.h"
#include "netlog.h"
#include "heap.h"
#include <stdlib.h>

#include "channels_proto.h"
#include "channels.h"

struct scene_objects_t
{
    struct gui_scene_t scene;
    struct gui_form_t selection;
    struct gui_select_t channel;
    char selected_channel[64];
    uint8_t channel_data[1024];
};

static struct scene_objects_t* scene_objects = NULL;

static void channel_selected(struct gui_select_t* this, struct gui_select_option_t* selected)
{
    strcpy(scene_objects->selected_channel, selected->value);
}

static void proceed_with_selection(struct gui_button_t* this)
{
    channels_set_channel(scene_objects->selected_channel);
    switch_channel_view();
}

static void process_channel(ChannelObject* object)
{
    ChannelObjectProperty* prop = find_property(object, OBJ_PROPERTY_ID);
    if (prop)
    {
        zxgui_select_add_option(&scene_objects->channel, prop->value, prop->value_size, 0);

        if (scene_objects->channel.first == scene_objects->channel.last)
        {
            channel_selected(&scene_objects->channel, scene_objects->channel.first);
        }
    }
}

static void get_channels_response(struct proto_process_t* proto)
{
    zxgui_scene_set_focus(&scene_objects->scene, &scene_objects->channel);
    zxgui_scene_set(&scene_objects->scene);
}

static void get_channels_error(const char* error)
{
    switch_alert(error, switch_connect_to_proxy);
}

static uint8_t* obtain_channel_data(struct gui_select_t* this)
{
    return scene_objects->channel_data;
}

void switch_select_channel()
{
    reset_heap();
    scene_objects = alloc_heap(sizeof (struct scene_objects_t));

    zxgui_scene_init(&scene_objects->scene, NULL);

    zxgui_form_init(&scene_objects->selection, XYWH(8, 7, 15, 11), "Select Channel", FORM_STYLE_DEFAULT);

    {
        zxgui_select_init(&scene_objects->channel, XYWH(0, 0, 13, 6), obtain_channel_data, NULL, channel_selected);
        zxgui_form_add_child(&scene_objects->selection, &scene_objects->channel);
    }

    {
        static struct gui_button_t button_cancel;
        zxgui_button_init(&button_cancel, XYWH(0, 8, 8, 1), 13, GUI_ICON_RETURN, "SELECT", proceed_with_selection);
        zxgui_form_add_child(&scene_objects->selection, &button_cancel);
    }


    zxgui_scene_add(&scene_objects->scene, &scene_objects->selection);

    scene_objects->channel.first = NULL;
    scene_objects->channel.last = NULL;

    declare_str_property_on_stack(key, OBJ_PROPERTY_ID, "channels", NULL);
    declare_object_on_stack(request, 128, &key);

    if (channels_send_request(request, process_channel, get_channels_response, get_channels_error))
    {
        return;
    }

    switch_progress("Fetching channels", NULL);
}