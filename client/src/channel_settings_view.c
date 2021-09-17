#include "zxgui.h"
#include "scenes.h"
#include "system.h"
#include "heap.h"
#include <stdlib.h>
#include <string.h>
#include "proto_asserts.h"

#include "channels_proto.h"
#include "channels.h"

enum setting_type_t
{
    SETTING_STRING = 0,
    SETTING_BOOL,
    SETTING_INT,
    SETTING_UNKNOWN
};

struct channel_setting_def_t
{
    char* id;
    char* description;
    char value[128];
    struct gui_object_t* editor;
};

#define MAX_SETTING_DEFS (8)

struct scene_objects_t
{
    struct gui_scene_t scene;
    struct gui_label_t title;
    struct gui_button_t button_back;
    struct gui_button_t button_next;
    struct gui_button_t button_prev;
    struct channel_setting_def_t defs[MAX_SETTING_DEFS];
    uint8_t settings_count;
    uint8_t current_setting;
    uint8_t ignore_first_entry;
    char label_title[64];
};

static struct scene_objects_t* scene_objects = NULL;

static void process_def(ChannelObject* object)
{
    if (scene_objects->settings_count >= MAX_SETTING_DEFS)
    {
        return;
    }

    if (scene_objects->ignore_first_entry)
    {
        scene_objects->ignore_first_entry = 0;
        return;
    }

    ChannelObjectProperty* id_ = find_property(object, OBJ_PROPERTY_ID);
    ChannelObjectProperty* description_ = find_property(object, OBJ_PROPERTY_TITLE);
    ChannelObjectProperty* value_ = find_property(object, 'v');

    if (id_ && description_)
    {
        struct channel_setting_def_t* def = &scene_objects->defs[scene_objects->settings_count++];

        def->id = alloc_heap(id_->value_size + 1);
        def->description = alloc_heap(description_->value_size + 1);
        proto_assert_str(def->id && def->description, "cannot allocate");

        memcpy(def->id, id_->value, id_->value_size);
        def->id[id_->value_size] = 0;

        if (value_)
        {
            memcpy(def->value, value_->value, value_->value_size);
            def->value[value_->value_size] = 0;
        }
        else
        {
            def->value[0] = 0;
        }

        memcpy(def->description, description_->value, description_->value_size);
        def->description[description_->value_size] = 0;
    }
}

static void get_defs_response(struct proto_process_t* proto)
{
    if (scene_objects->settings_count == 0)
    {
        struct gui_label_t* no_settings = alloc_heap(sizeof(struct gui_label_t));
        proto_assert_str(no_settings, "Cannot allocate settings");

        zxgui_label_init(no_settings, XYWH(0, 2, 32, 1),
            "This channel has no settings.", INK_YELLOW | PAPER_BLACK, 0);
        zxgui_scene_add(&scene_objects->scene, no_settings);
    }
    else
    {
        uint8_t offset = 2;

        for (uint8_t i = 0; i < scene_objects->settings_count; i++)
        {
            struct channel_setting_def_t* def = &scene_objects->defs[i];

            struct gui_label_t* description = alloc_heap(sizeof(struct gui_label_t));
            proto_assert_str(description, "Cannot allocate description");
            zxgui_label_init(description,
                get_xywh(0, offset, 32, 1), def->description, INK_WHITE | PAPER_BLACK, 0);
            zxgui_scene_add(&scene_objects->scene, description);

            offset += 1;

            struct gui_edit_t* value_edit = alloc_heap(sizeof(struct gui_edit_t));
            proto_assert_str(description, "Cannot allocate value_edit");
            def->editor = (struct gui_object_t*)value_edit;
            zxgui_edit_init(value_edit, get_xywh(0, offset, 30, 2), def->value, 128);
            zxgui_scene_add(&scene_objects->scene, value_edit);

            if (scene_objects->scene.focus == NULL)
            {
                zxgui_scene_set_focus(&scene_objects->scene, value_edit);
            }

            offset += 3;
        }
    }

    zxgui_scene_set(&scene_objects->scene);
}

static void get_defs_error(const char* error)
{
    switch_alert(error, switch_channel_view);
}

static void process_save_settings_def(ChannelObject* object)
{

}

static void get_save_settings_response(struct proto_process_t* proto)
{
    switch_channel_view();
}

static void save_settings(struct gui_button_t* this)
{
    ChannelStackObjectProperty props[MAX_SETTING_DEFS];

    declare_str_property_on_stack(boards_, OBJ_PROPERTY_ID, "save_settings", NULL);
    declare_str_property_on_stack(channel_, 'c', channels_get_channel(), &boards_);
    ChannelStackObjectProperty* last = &channel_;

    for (uint8_t i = 0; i < scene_objects->settings_count; i++)
    {
        struct channel_setting_def_t* def = &scene_objects->defs[i];
        uint8_t id_len = strlen(def->id);
        uint8_t value_len = strlen(def->value);
        memmove(def->value + id_len + 1, def->value, value_len + 1);
        memcpy(def->value, def->id, id_len);
        def->value[id_len] = '=';

        ChannelStackObjectProperty* p = &props[i];
        p->key = OBJ_PROPERTY_PAYLOAD;
        p->value = def->value;
        p->value_size = value_len + id_len + 1;
        p->prev = last;

        last = p;
    }

    declare_object_on_stack(request, 128, last);

    if (channels_send_request(request, process_save_settings_def, get_save_settings_response, get_defs_error))
    {
        return;
    }

    switch_progress("Saving Settings", NULL);
}

static void select_entry(uint8_t new_entry)
{
    struct channel_setting_def_t* def = &scene_objects->defs[scene_objects->current_setting];
    object_invalidate(def->editor, GUI_FLAG_DIRTY);
    def = &scene_objects->defs[new_entry];
    scene_objects->current_setting = new_entry;
    zxgui_scene_set_focus(&scene_objects->scene, def->editor);
    object_invalidate(def->editor, GUI_FLAG_DIRTY);
}

static void next_entry(struct gui_button_t* this)
{
    if (scene_objects->current_setting + 1 < scene_objects->settings_count)
    {
        select_entry(scene_objects->current_setting + 1);
    }
    else
    {
        select_entry(0);
    }
}

static void prev_entry(struct gui_button_t* this)
{
    if (scene_objects->current_setting > 0)
    {
        select_entry(scene_objects->current_setting - 1);
    }
    else
    {
        select_entry(scene_objects->settings_count - 1);
    }
}

extern void switch_channel_settings_view()
{
    reset_heap();

    scene_objects = alloc_heap(sizeof(struct scene_objects_t));
    proto_assert_str(scene_objects, "Cannot allocate");

    const char* channel = channels_get_channel();
    uint8_t len_channel = strlen(channel);
    scene_objects->ignore_first_entry = 1;
    strcpy(scene_objects->label_title, channel);
    strcpy(scene_objects->label_title + len_channel, " SETTINGS");
    scene_objects->settings_count = 0;
    scene_objects->current_setting = 0;

    zxgui_scene_init(&scene_objects->scene, NULL);

    {
        zxgui_label_init(&scene_objects->title, XYWH(0, 0, 32, 1), scene_objects->label_title, INK_BLACK | PAPER_WHITE, 0);
        zxgui_scene_add(&scene_objects->scene, &scene_objects->title);
    }

    {
        zxgui_button_init(&scene_objects->button_back, XYWH(0, 23, 4, 1), 13, GUI_ICON_RETURN, "SAVE", save_settings);
        zxgui_scene_add(&scene_objects->scene, &scene_objects->button_back);
    }

    {
        zxgui_button_init(&scene_objects->button_next, XYWH(4, 23, 1, 1), 10, GUI_ICON_MORE_TO_FOLLOW, "NEXT", next_entry);
        zxgui_scene_add(&scene_objects->scene, &scene_objects->button_next);
    }

    {
        zxgui_button_init(&scene_objects->button_prev, XYWH(8, 23, 1, 1), 11, GUI_ICON_LESS_TO_FOLLOW, "PREV", prev_entry);
        zxgui_scene_add(&scene_objects->scene, &scene_objects->button_prev);
    }

    declare_str_property_on_stack(boards_, OBJ_PROPERTY_ID, "setting_defs", NULL);
    declare_str_property_on_stack(channel_, 'c', channels_get_channel(), &boards_);

    declare_object_on_stack(request, 128, &channel_);

    if (channels_send_request(request, process_def, get_defs_response, get_defs_error))
    {
        return;
    }

    switch_progress("Fetching Settings", NULL);
}