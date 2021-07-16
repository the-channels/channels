#include "zxgui.h"
#include "zxgui_internal.h"
#include "scenes.h"
#include "system.h"
#include "netlog.h"
#include <stdlib.h>

#include "channels_proto.h"
#include "channels.h"

static struct gui_scene_t scene;
static struct gui_form_t description_form;
static struct gui_select_t board;
static struct gui_label_t title;
static struct gui_label_t description;

static const char* selected_board = NULL;
static char label_title[48];

static void channel_selected(struct gui_select_t* this, struct gui_select_option_t* selected)
{
    selected_board = selected->value;

    if (selected->user)
    {
        description.title = selected->user;
        object_invalidate(&description, GUI_FLAG_DIRTY);
    }
    else
    {
        description.title = "";
    }
}

static void proceed_with_selection(struct gui_button_t* this)
{
    channels_set_board(selected_board);
    switch_thread_view();
}

static void go_back(struct gui_button_t* this)
{
    switch_select_channel();
}

void init_select_board()
{
    zxgui_scene_init(&scene, NULL);

    {
        zxgui_label_init(&title, 0, 0, 32, 1, label_title, INK_BLACK | PAPER_WHITE, 0);
        zxgui_scene_add(&scene, &title);
    }

    {
        zxgui_select_init(&board, 0, 1, 13, 21, channel_selected);
        zxgui_scene_add(&scene, &board);
    }

    {
        zxgui_form_init(&description_form, 14, 1, 17, 21, "Description", FORM_STYLE_FRAME);
        zxgui_scene_add(&scene, &description_form);

        zxgui_label_init(&description, 1, 1, 14, 17, "", INK_WHITE | PAPER_BLACK, 0);
        zxgui_form_add_child(&description_form, &description);
    }

    {
        static struct gui_button_t button_cancel;
        zxgui_button_init(&button_cancel, 0, 23, 4, 1, 13, GUI_ICON_RETURN, "SELECT", proceed_with_selection);
        zxgui_scene_add(&scene, &button_cancel);
    }

    {
        static struct gui_button_t button_back;
        zxgui_button_init(&button_back, 6, 23, 4, 1, 32, GUI_ICON_SPACE, "BACK", go_back);
        zxgui_scene_add(&scene, &button_back);
    }
}

static void process_board(ChannelObject* object)
{
    ChannelObjectProperty* prop = find_property(object, OBJ_PROPERTY_ID);
    if (prop)
    {
        ChannelObjectProperty* t = find_property(object, OBJ_PROPERTY_TITLE);
        uint16_t value_size;
        if (t)
        {
            value_size = t->value_size + 1;
        }
        else
        {
            value_size = 0;
        }

        uint8_t* new_data = zxgui_select_add_option(&board, prop->value, prop->value_size, value_size);

        if (t)
        {
            memcpy(new_data, t->value, t->value_size);
            new_data[t->value_size] = 0;
        }
    }
}

static void get_boards_response(struct proto_process_t* proto)
{
    zxgui_scene_set_focus(&scene, &board);
    zxgui_scene_set(&scene);
}

static void get_boards_error(const char* error)
{
    netlog("%s\n", error);
    switch_alert(error, switch_connect_to_proxy);
}

void switch_select_board()
{
    sprintf(label_title, "%s SELECT BOARD", channels_get_channel());

    declare_str_property_on_stack(boards_, OBJ_PROPERTY_ID, "boards", NULL);
    declare_str_property_on_stack(channel_, 'c', channels_get_channel(), &boards_);

    uint16_t limit = 128;
    declare_arg_property_on_stack(limit_, 'l', limit, &channel_);

    declare_object_on_stack(request, 128, &limit_);


    if (channels_send_request(request, process_board, get_boards_response, get_boards_error))
    {
        return;
    }

    switch_progress("Fetching Boards", NULL);
}