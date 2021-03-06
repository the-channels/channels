
#include <string.h>
#include "zxgui.h"
#include "system.h"
#include <text_ui.h>

static void _button_render(uint8_t x, uint8_t y, struct gui_button_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (this->flags & GUI_FLAG_HIDDEN)
    {
        return;
    }

    if (is_object_invalidated(this))
    {
        zxgui_screen_color(COLOR_FG_GREEN | COLOR_BRIGHT | COLOR_BG_BLACK);
        if (this->flags & GUI_FLAG_SYM)
        {
            zxgui_screen_put(x, y, GUI_ICON_SYM);
            x++;
        }

        zxgui_screen_put(x, y, this->icon);

        if (this->title)
        {
            x++;

            text_ui_color(COLOR_FG_WHITE | COLOR_BG_BLACK);
            text_ui_puts_at(x, y, (char *) this->title);
        }
    }
}

extern uint8_t is_alt_key_pressed();

static uint8_t _button_event(enum gui_event_type event_type, void* event, struct gui_button_t* this, struct gui_scene_t* scene)
{
    switch (event_type)
    {
        case GUI_EVENT_KEY_PRESSED:
        {
            struct gui_event_key_pressed* ev = event;
            if (ev->key == this->key)
            {
                if (this->flags & GUI_FLAG_SYM)
                {
                    if (is_alt_key_pressed() == 0)
                    {
                        break;
                    }
                }
                if (this->pressed)
                {
                    this->pressed(this);
                }
                return 1;
            }

            break;
        }
    }
    return 0;
}

void zxgui_button_init(struct gui_button_t* button, xywh_t xywh,
    uint8_t key, uint8_t icon, const char* title, gui_button_pressed_f pressed) ZXGUI_CDECL
{
    zxgui_base_init(button, xywh, _button_render, _button_event);

    button->flags = GUI_FLAG_DIRTY;
    button->key = key;
    button->icon = icon;
    button->title = title;
    button->pressed = pressed;
}