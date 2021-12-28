#ifndef ZXGUI_H
#define ZXGUI_H

#define ZXGUI_CDECL __z88dk_callee __stdc

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-reserved-identifier"

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
    GUI_SELECTED_ENTRY,
    GUI_ICON_REPLIES,
    GUI_ICON_REPLY,
    GUI_ICON_H,
    GUI_ICON_R,
    GUI_ICON_N,
    GUI_ICON_SYM,
    GUI_ICON_5,
    GUI_ICON_T,
    GUI_ICON_B,
    GUI_ICON_EMPTY,
    GUI_ICON_ESCAPE,
};

enum gui_event_type {
    GUI_EVENT_KEY_PRESSED,
};

enum gui_event_keycodes {
    GUI_KEY_CODE_DOWN = 10,
    GUI_KEY_CODE_UP = 11,
    GUI_KEY_CODE_BACKSPACE = 12,
    GUI_KEY_CODE_RETURN = 13,
    GUI_KEY_CODE_ESCAPE = 27,
};

struct gui_event_key_pressed
{
    char key;
};

struct gui_object_t;
struct gui_scene_t;

typedef void (*gui_render_f)(uint8_t x, uint8_t y, struct gui_object_t* this, struct gui_scene_t* scene);
typedef uint8_t (*gui_event_f)(enum gui_event_type event_type, void* event, struct gui_object_t* this, struct gui_scene_t* scene);

#define GUI_FLAG_DIRTY (0x01u)
#define GUI_FLAG_DIRTY_INTERNAL (0x02u)
#define GUI_FLAG_MULTILINE (0x04u)
#define GUI_FLAG_HIDDEN (0x08u)
#define GUI_FLAG_SYM (0x10u)

#define GUI_OBJECT_BASE uint8_t x; \
                        uint8_t y; \
                        uint8_t w; \
                        uint8_t h; \
                        struct gui_object_t* next; \
                        gui_render_f render; \
                        gui_event_f event; \
                        uint8_t flags

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
typedef void (*gui_scene_key_pressed_f)(int key);

enum gui_form_style
{
    FORM_STYLE_DEFAULT = 0,
    FORM_STYLE_FRAME,
    FORM_STYLE_EMPTY,
};

/*
 * Take care when you change any of this. Defines below use memory shenanigans as optimization.
 */

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
#ifdef __SPECTRUM
    uint8_t* cursor_pixels_addr;
    uint8_t* cursor_color_addr;
    uint8_t cursor_even;
#else
    uint8_t cursor_x;
    uint8_t cursor_y;
#endif
    uint16_t value_size;
    char* value;
};

struct gui_select_t;
typedef uint8_t* (*gui_select_obtain_data_f)(struct gui_select_t* this);

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
    gui_select_obtain_data_f obtain_data;
    void* user;
    uint16_t buffer_offset;
    struct gui_select_option_t* first;
    struct gui_select_option_t* last;
    gui_select_selected selected;
    struct gui_select_option_t* selection;
    struct gui_select_option_t* last_selection;
};

#ifdef __SPECTRUM
typedef uint8_t animation_speed_t;
#else
typedef uint32_t animation_speed_t;
#endif

struct gui_animated_icon_t
{
    GUI_OBJECT_BASE;
    uint8_t color;
    uint8_t frames;
    uint8_t current_frame;
    animation_speed_t time;
    animation_speed_t speed;
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

struct gui_dynamic_image_t
{
    GUI_OBJECT_BASE;
    gui_label_obtain_image_f obtain_data;
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

struct gui_dynamic_label_t
{
    struct gui_label_t parent;
    gui_label_obtain_title_data_f obtain_data;
    void* user;
};

struct gui_scene_t
{
    struct gui_object_t* child;
    struct gui_object_t* focus;
    gui_scene_update_f update;
    gui_scene_key_pressed_f key_pressed;
};

extern void zxgui_init(void);

extern void zxgui_scene_init(struct gui_scene_t* scene, gui_scene_update_f update);
extern void zxgui_scene_set(struct gui_scene_t* scene);
extern void zxgui_scene_add(struct gui_scene_t* scene, void* object) __z88dk_callee;
extern void zxgui_scene_set_focus(struct gui_scene_t* scene, void* object) __z88dk_callee;
extern uint8_t zxgui_scene_dispatch_event(struct gui_scene_t* scene,
    enum gui_event_type event_type, void* event) __z88dk_callee;
extern struct gui_object_t* zxgui_scene_get_last_object(struct gui_scene_t* scene);
extern void object_invalidate(void* object, uint8_t flag);
extern void zxgui_line(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t c) __z88dk_callee;
extern void zxgui_scene_iteration(void);
extern void zxgui_rectangle(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t c) __z88dk_callee;
extern void zxgui_icon(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source) __z88dk_callee;
extern void zxgui_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source, const uint8_t* colors) __z88dk_callee;
extern struct gui_object_t* zxgui_get_last_object(struct gui_object_t* object);
extern uint8_t is_object_invalidated(void* object);

typedef uint32_t xywh_t;
#define XYWH(x, y, w, h) (uint32_t)((uint8_t)(x) + ((uint8_t)(y) * 0x100) + ((uint8_t)(w) * 0x10000) + ((uint8_t)(h) * 0x1000000))
extern xywh_t get_xywh(uint8_t x, uint8_t y, uint8_t w, uint8_t h) __z88dk_callee;

extern void zxgui_base_init(void* o, xywh_t xywh, void* render, void* event) ZXGUI_CDECL;

extern void zxgui_form_init(struct gui_form_t* form, xywh_t xywh, const char* title, uint8_t style) ZXGUI_CDECL;
extern void zxgui_form_add_child(struct gui_form_t* form, void* child) ZXGUI_CDECL;
extern void zxgui_edit_init(struct gui_edit_t* edit, xywh_t xywh, char* buffer, uint16_t buffer_size) ZXGUI_CDECL;
extern void zxgui_multiline_edit_init(struct gui_edit_t* edit, xywh_t xywh, char* buffer, uint16_t buffer_size) ZXGUI_CDECL;
extern void zxgui_select_init(struct gui_select_t* select, xywh_t xywh,
    gui_select_obtain_data_f obtain_data, void* user,
    gui_select_selected selected) ZXGUI_CDECL;
extern uint8_t* zxgui_select_add_option(struct gui_select_t* select, const char* option,
    uint8_t option_len, uint16_t user_data_amount) ZXGUI_CDECL;
extern void zxgui_button_init(struct gui_button_t* button, xywh_t xywh,
    uint8_t key, uint8_t icon, const char* title, gui_button_pressed_f pressed) ZXGUI_CDECL;
extern void zxgui_label_init(struct gui_label_t* label, xywh_t xywh,
    const char* title, uint8_t color, uint8_t flags) ZXGUI_CDECL;
extern void zxgui_dynamic_label_init(struct gui_dynamic_label_t* label, xywh_t xywh,
    uint8_t color, uint8_t flags, gui_label_obtain_title_data_f obtain_data, void* user) ZXGUI_CDECL;
extern uint8_t zxgui_label_text_height(uint8_t w, const char* text, uint16_t len, uint8_t max_height) ZXGUI_CDECL;
extern void zxgui_animated_icon_init(struct gui_animated_icon_t* icon, xywh_t xywh,
    uint8_t frames, uint8_t color, const uint8_t* source, animation_speed_t speed) ZXGUI_CDECL;
extern void zxgui_image_init(struct gui_image_t* image, xywh_t xywh,
    const uint8_t* source, const uint8_t* colors) ZXGUI_CDECL;
extern void zxgui_dynamic_image_init(struct gui_dynamic_image_t* image, xywh_t xywh,
    gui_label_obtain_image_f obtain_data, void* user) ZXGUI_CDECL;

extern uint8_t screen_color;

#define zxgui_screen_color(color) screen_color = color;
extern void zxgui_screen_put(uint8_t x, uint8_t y, uint8_t ch) __z88dk_callee;
extern void zxgui_screen_clear(uint8_t x, uint8_t y, uint8_t w, uint8_t h) __z88dk_callee;
extern void zxgui_screen_recolor(uint8_t x, uint8_t y, uint8_t w, uint8_t h) __z88dk_callee;

#pragma clang diagnostic pop

#endif