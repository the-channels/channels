#ifndef ZXGUI_H
#define ZXGUI_H

#include <stdint.h>

enum gui_tiles {
    GUI_FORM_LEFT_BOTTOM_CORNER = 0,
    GUI_FORM_RIGHT_BOTTOM_CORNER,
    GUI_FORM_RIGHT_TOP_CORNER,
    GUI_FORM_LEFT_TOP_CORNER,
    GUI_FORM_TOP,
    GUI_FORM_LEFT,
    GUI_FORM_BOTTOM,
    GUI_FORM_RIGHT,
    GUI_EDIT_LEFT_BOTTOM_CORNER,
    GUI_EDIT_RIGHT_BOTTOM_CORNER,
    GUI_EDIT_RIGHT_TOP_CORNER,
    GUI_EDIT_LEFT_TOP_CORNER,
    GUI_EDIT_TOP,
    GUI_EDIT_LEFT,
    GUI_EDIT_BOTTOM,
    GUI_EDIT_RIGHT,
    GUI_SELECT_LEFT_BOTTOM_CORNER,
    GUI_SELECT_RIGHT_BOTTOM_CORNER,
    GUI_SELECT_RIGHT_TOP_CORNER,
    GUI_SELECT_LEFT_TOP_CORNER,
    GUI_SELECT_TOP,
    GUI_SELECT_LEFT,
    GUI_SELECT_BOTTOM,
    GUI_SELECT_RIGHT,
    GUI_EDIT_CURSOR,
    GUI_ICON_RETURN,
    GUI_ICON_C,
    GUI_ICON_LOADING_A_1,
    GUI_ICON_LOADING_A_2,
    GUI_ICON_LOADING_A_3,
    GUI_ICON_LOADING_A_4,
    GUI_ICON_LOADING_B_1,
    GUI_ICON_LOADING_B_2,
    GUI_ICON_LOADING_B_3,
    GUI_ICON_LOADING_B_4,
    GUI_ICON_SPACE,
    GUI_ICON_MORE_TO_FOLLOW,
    GUI_ICON_LESS_TO_FOLLOW,
    GUI_ICON_P,
    QUESTION_MARK_1,
    QUESTION_MARK_2,
    QUESTION_MARK_3,
    QUESTION_MARK_4,
    GUI_ICON_6,
    GUI_ICON_7,
    GUI_ICON_LOADING_SMALL_1,
    GUI_ICON_LOADING_SMALL_2,
    GUI_SELECTED_ENTRY
};

enum gui_event_type {
    GUI_EVENT_KEY_PRESSED,
    GUI_EVENT_CHANGED_SCENE,
};

struct gui_event_key_pressed
{
    uint8_t key;
};

struct gui_object_t;
struct gui_scene_t;

typedef void (*gui_render_f)(uint8_t x, uint8_t y, struct gui_object_t* this, struct gui_scene_t* scene);
typedef uint8_t (*gui_event_f)(enum gui_event_type event_type, void* event, struct gui_object_t* this, struct gui_scene_t* scene);

#define GUI_FLAG_DIRTY (0x01u)
#define GUI_FLAG_DIRTY_INTERNAL (0x02u)
#define GUI_FLAG_MULTILINE (0x04u)
#define GUI_FLAG_HIDDEN (0x08u)

#define GUI_OBJECT_BASE gui_render_f render; \
                        gui_event_f event; \
                        uint8_t flags; \
                        struct gui_object_t* next; \
                        uint8_t x; \
                        uint8_t y; \
                        uint8_t w; \
                        uint8_t h

struct gui_object_t
{
    GUI_OBJECT_BASE;
};

struct gui_button_t;
struct gui_select_t;
struct gui_select_option_t;

typedef void (*gui_button_pressed_f)(struct gui_button_t* this);
typedef void (*gui_select_selected)(struct gui_select_t* this, struct gui_select_option_t* selected);
typedef void (*gui_scene_update_f)(struct gui_scene_t* scene);

enum gui_form_style
{
    FORM_STYLE_DEFAULT = 0,
    FORM_STYLE_FRAME,
    FORM_STYLE_EMPTY,
};

struct gui_form_t
{
    GUI_OBJECT_BASE;
    const char* title;
    uint8_t style;
    struct gui_object_t* child;
};

struct gui_edit_t
{
    GUI_OBJECT_BASE;
    char value[32];
};

struct gui_select_option_t
{
    struct gui_select_option_t* prev;
    struct gui_select_option_t* next;
    void* user;
    char value[];
};

struct gui_select_t
{
    GUI_OBJECT_BASE;
    struct gui_select_option_t* first;
    struct gui_select_option_t* last;
    gui_select_selected selected;
    struct gui_select_option_t* selection;
    struct gui_select_option_t* last_selection;
};

struct gui_animated_icon_t
{
    GUI_OBJECT_BASE;
    uint8_t color;
    uint8_t frames;
    uint8_t current_frame;
    uint8_t time;
    uint8_t speed;
    const uint8_t* source;
};

struct gui_image_t
{
    GUI_OBJECT_BASE;
    const uint8_t* source;
    const uint8_t* colors;
};

struct gui_dynamic_image_t;

typedef const uint8_t* (*gui_label_obtain_image_f)(struct gui_dynamic_image_t* this, uint16_t* data_size);
typedef void (*gui_label_release_image_f)(struct gui_dynamic_image_t* this);

struct gui_dynamic_image_t
{
    GUI_OBJECT_BASE;
    gui_label_obtain_image_f obtain_data;
    gui_label_release_image_f release_data;
    void* user;
};

struct gui_button_t
{
    GUI_OBJECT_BASE;
    uint8_t icon;
    const char* title;
    uint8_t key;
    gui_button_pressed_f pressed;
};

struct gui_label_t
{
    GUI_OBJECT_BASE;
    const char* title;
    uint8_t color;
};

struct gui_dynamic_label_t;

typedef const char* (*gui_label_obtain_title_data_f)(struct gui_dynamic_label_t* this);
typedef void (*gui_label_release_title_data_f)(struct gui_dynamic_label_t* this);

struct gui_dynamic_label_t
{
    struct gui_label_t parent;
    gui_label_obtain_title_data_f obtain_data;
    gui_label_release_title_data_f release_data;
    void* user;
};

struct gui_scene_t
{
    struct gui_object_t* child;
    struct gui_object_t* focus;
    gui_scene_update_f update;
};

extern void zxgui_init();

extern void zxgui_scene_init(struct gui_scene_t* scene, gui_scene_update_f update);
extern void zxgui_scene_set(struct gui_scene_t* scene);
extern void zxgui_scene_add(struct gui_scene_t* scene, void* object);
extern void zxgui_scene_set_focus(struct gui_scene_t* scene, void* object);
extern uint8_t zxgui_scene_dispatch_event(struct gui_scene_t* scene, enum gui_event_type event_type, void* event);
extern struct gui_object_t* zxgui_scene_get_last_object(struct gui_scene_t* scene);

extern void zxgui_scene_iteration();

extern void zxgui_form_init(struct gui_form_t* form, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char* title, uint8_t style);
extern void zxgui_form_add_child(struct gui_form_t* form, void* child);

extern void zxgui_edit_init(struct gui_edit_t* edit, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
extern void zxgui_select_init(struct gui_select_t* select, uint8_t x, uint8_t y, uint8_t w, uint8_t h, gui_select_selected selected);
extern uint8_t* zxgui_select_add_option(struct gui_select_t* select, const char* option, uint8_t option_len, uint16_t user_data_amount);
extern void zxgui_button_init(struct gui_button_t* button, uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    uint8_t key, uint8_t icon, const char* title, gui_button_pressed_f pressed);
extern void zxgui_label_init(struct gui_label_t* label, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const char* title, uint8_t color, uint8_t flags);
extern void zxgui_dynamic_label_init(struct gui_dynamic_label_t* label, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color,
    uint8_t flags, gui_label_obtain_title_data_f obtain_data, gui_label_release_title_data_f release_data, void* user);
extern uint8_t zxgui_label_text_height(uint8_t w, const char* text, uint16_t len);
extern void zxgui_animated_icon_init(struct gui_animated_icon_t* icon, uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    uint8_t frames, uint8_t color, const uint8_t* source, uint8_t speed);
extern void zxgui_image_init(struct gui_image_t* image, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source, const uint8_t* colors);
extern void zxgui_dynamic_image_init(struct gui_dynamic_image_t* image, uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    gui_label_obtain_image_f obtain_data, gui_label_release_image_f release_data, void* user);
extern void zxgui_screen_color(uint8_t color) __z88dk_callee;
extern void zxgui_screen_put(uint8_t x, uint8_t y, uint8_t ch) __z88dk_callee;
extern void zxgui_screen_clear(uint8_t x, uint8_t y, uint8_t w, uint8_t h) __z88dk_callee;
extern void zxgui_screen_recolor(uint8_t x, uint8_t y, uint8_t w, uint8_t h) __z88dk_callee;

#endif