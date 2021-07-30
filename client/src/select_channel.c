#include "zxgui.h"
#include "zxgui_internal.h"
#include "scenes.h"
#include "system.h"
#include "netlog.h"
#include "heap.h"

#include "channels_proto.h"
#include "channels.h"

static struct gui_scene_t scene;
static struct gui_form_t selection;
static struct gui_select_t channel;
static char selected_channel[64];

static void channel_selected(struct gui_select_t* this, struct gui_select_option_t* selected)
{
    strcpy(selected_channel, selected->value);
}

static void proceed_with_selection(struct gui_button_t* this)
{
    channels_set_channel(selected_channel);
    switch_select_board();
}

void init_select_channel()
{
    zxgui_scene_init(&scene, NULL);

    zxgui_form_init(&selection, 8, 7, 15, 11, "Select Channel", FORM_STYLE_DEFAULT);

    {
        zxgui_select_init(&channel, 0, 0, 13, 6, channel_selected);
        zxgui_form_add_child(&selection, &channel);
    }

    {
        static struct gui_button_t button_cancel;
        zxgui_button_init(&button_cancel, 0, 8, 8, 1, 13, GUI_ICON_RETURN, "SELECT", proceed_with_selection);
        zxgui_form_add_child(&selection, &button_cancel);
    }


    zxgui_scene_add(&scene, &selection);
}

static void process_channel(ChannelObject* object)
{
    ChannelObjectProperty* prop = find_property(object, OBJ_PROPERTY_ID);
    if (prop)
    {
        zxgui_select_add_option(&channel, prop->value, prop->value_size, 0);

        if (channel.first == channel.last)
        {
            channel_selected(&channel, channel.first);
        }
    }
}

static void get_channels_response(struct proto_process_t* proto)
{
    zxgui_scene_set_focus(&scene, &channel);
    zxgui_scene_set(&scene);
}

static void get_channels_error(const char* error)
{
    switch_alert(error, switch_connect_to_proxy);
}

void switch_select_channel()
{
    reset_heap();

    channel.first = NULL;
    channel.last = NULL;

    declare_str_property_on_stack(key, OBJ_PROPERTY_ID, "channels", NULL);
    declare_object_on_stack(request, 128, &key);

    if (channels_send_request(request, process_channel, get_channels_response, get_channels_error))
    {
        return;
    }

    switch_progress("Fetching channels", NULL);
}