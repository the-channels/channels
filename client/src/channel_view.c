#include "zxgui.h"
#include "scenes.h"
#include "system.h"
#include "netlog.h"
#include "heap.h"
#include <stdlib.h>
#include <string.h>
#include "alert.h"
#include "channels_proto.h"
#include "channels.h"

struct scene_objects_t
{
    struct gui_scene_t scene;
    struct gui_form_t description_form;
    struct gui_select_t board;
    struct gui_label_t title;
    struct gui_label_t description;
    struct gui_button_t button_cancel;
    struct gui_button_t button_settings;
    struct gui_button_t button_back;
    char selected_board[64];
    char label_title[64];
    char description_data[128];
    uint8_t buffer_heap_id;
    struct channels_alert_buf_t alert;
};

static struct scene_objects_t* scene_objects = NULL;

struct channel_board_info
{
    char* id;
    char* description;
};

static void board_selected(struct gui_select_t* this, struct gui_select_option_t* selected)
{
    struct channel_board_info* info = selected->user;
    strcpy(scene_objects->description_data, info->description);
    object_invalidate(&scene_objects->description, GUI_FLAG_DIRTY);
    strcpy(scene_objects->selected_board, info->id ? info->id : selected->value);
}

static void proceed_with_selection(struct gui_button_t* this)
{
    channels_set_board(scene_objects->selected_board);
    switch_thread_view();
}

static void go_back(struct gui_button_t* this)
{
    switch_select_channel();
}

static void open_channel_settings(struct gui_button_t* this)
{
    switch_channel_settings_view();
}

static void process_board(ChannelObject* object)
{
    ChannelObjectProperty* prop = find_property(object, OBJ_PROPERTY_ID);
    ChannelObjectProperty* title_ = find_property(object, OBJ_PROPERTY_TITLE);
    ChannelObjectProperty* description_ = find_property(object, OBJ_PROPERTY_COMMENT);

    if (prop && description_)
    {
        uint16_t value_size = sizeof(struct channel_board_info) + description_->value_size + 1;
        uint8_t* new_data;

        if (title_)
        {
            value_size += prop->value_size + 1;
            new_data = zxgui_select_add_option(&scene_objects->board, title_->value, title_->value_size, value_size);
        }
        else
        {
            new_data = zxgui_select_add_option(&scene_objects->board, prop->value, prop->value_size, value_size);
        }

        struct channel_board_info* board_info = (struct channel_board_info*)new_data;
        new_data += sizeof(struct channel_board_info);
        if (title_)
        {
            board_info->id = (char*)new_data;
            memcpy(new_data, prop->value, prop->value_size);
            new_data[prop->value_size] = 0;
            new_data += prop->value_size + 1;
        }
        else
        {
            board_info->id = NULL;
        }

        board_info->description = (char*)new_data;

        {
            memcpy(board_info->description, description_->value, description_->value_size);
            board_info->description[description_->value_size] = 0;
        }

        if (scene_objects->board.last == scene_objects->board.first)
        {
            board_selected(&scene_objects->board, scene_objects->board.last);
        }
    }
}

static void get_boards_response(struct proto_process_t* proto)
{
    zxgui_scene_set_focus(&scene_objects->scene, &scene_objects->board);
    zxgui_scene_set(&scene_objects->scene);
}

static void get_boards_error(const char* error)
{
    alert_and_switch_to_connect(error);
}

uint8_t* obtain_select_buffer(struct gui_select_t* this)
{
    return open_heap_blob(scene_objects->buffer_heap_id);
}

void switch_channel_view()
{
    reset_heap();
    scene_objects = alloc_heap(sizeof(struct scene_objects_t));

    scene_objects->buffer_heap_id = allocate_heap_blob();
    strcpy(scene_objects->description_data, "");

    zxgui_scene_init(&scene_objects->scene, NULL);

    {
        zxgui_label_init(&scene_objects->title,
#ifdef STATIC_SCREEN_SIZE
            XYWH(0, 0, 32, 1),
#else
            XYWH(0, 0, SCREEN_WIDTH, 1),
#endif
            scene_objects->label_title, COLOR_FG_BLACK | COLOR_BG_WHITE, 0);

        zxgui_scene_add(&scene_objects->scene, &scene_objects->title);
    }

    {
        zxgui_select_init(&scene_objects->board,
#ifdef STATIC_SCREEN_SIZE
            XYWH(0, 1, 13, 21),
#else
            XYWH(0, 1, (SCREEN_WIDTH / 2) - 1, SCREEN_HEIGHT - 3),
#endif
            obtain_select_buffer, NULL, board_selected);

        zxgui_scene_add(&scene_objects->scene, &scene_objects->board);
    }

    {
        zxgui_form_init(&scene_objects->description_form,
#ifdef STATIC_SCREEN_SIZE
            XYWH(14, 1, 17, 21),
#else
            XYWH(SCREEN_WIDTH / 2, 1, (SCREEN_WIDTH / 2) - 1, SCREEN_HEIGHT - 3),
#endif
            "Description", FORM_STYLE_FRAME);

        zxgui_scene_add(&scene_objects->scene, &scene_objects->description_form);

        zxgui_label_init(&scene_objects->description,
#ifdef STATIC_SCREEN_SIZE
            XYWH(1, 1, 14, 17),
#else
            XYWH(1, 1, (SCREEN_WIDTH / 2) - 3, SCREEN_HEIGHT - 6),
#endif
            scene_objects->description_data, COLOR_FG_WHITE | COLOR_BG_BLACK, GUI_FLAG_MULTILINE);

        zxgui_form_add_child(&scene_objects->description_form, &scene_objects->description);
    }

    {
        zxgui_button_init(&scene_objects->button_cancel,
#ifdef STATIC_SCREEN_SIZE
            XYWH(0, 23, 4, 1),
#else
            XYWH(0, SCREEN_HEIGHT - 1, 4, 1),
#endif
            13, GUI_ICON_RETURN, "SELECT", proceed_with_selection);

        zxgui_scene_add(&scene_objects->scene, &scene_objects->button_cancel);
    }

    {
        zxgui_button_init(&scene_objects->button_back,
#ifdef STATIC_SCREEN_SIZE
            XYWH(5, 23, 4, 1),
#else
            XYWH(5 * TWO_CHARACTERS_FIT_IN, SCREEN_HEIGHT - 1, 4, 1),
#endif
            32, GUI_ICON_SPACE, "BACK", go_back);
        zxgui_scene_add(&scene_objects->scene, &scene_objects->button_back);
    }

    {
        zxgui_button_init(&scene_objects->button_settings,
#ifdef STATIC_SCREEN_SIZE
            XYWH(9, 23, 4, 1),
#else
            XYWH(9 * TWO_CHARACTERS_FIT_IN, SCREEN_HEIGHT - 1, 4, 1),
#endif
            'c', GUI_ICON_C, "CHANNEL SETTINGS", open_channel_settings);
        zxgui_scene_add(&scene_objects->scene, &scene_objects->button_settings);
    }

    scene_objects->board.first = NULL;
    scene_objects->board.last = NULL;

    const char* channel = channels_get_channel();
    uint8_t len_channel = strlen(channel);
    strcpy(scene_objects->label_title, channel);
    strcpy(scene_objects->label_title + len_channel, " SELECT BOARD");

    declare_str_property_on_stack(boards_, OBJ_PROPERTY_ID, "boards", NULL);
    declare_str_property_on_stack(channel_, 'c', channel, &boards_);

    uint16_t limit = 128;
    declare_arg_property_on_stack(limit_, 'l', limit, &channel_);

    declare_object_on_stack(request, 128, &limit_);

    if (channels_send_request(request, process_board, get_boards_response, get_boards_error))
    {
        return;
    }

    switch_progress("Fetching Boards");
}