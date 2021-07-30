
#include <string.h>
#include "zxgui.h"
#include "zxgui_internal.h"
#include <font/fzx.h>
#include "system.h"
#include <time.h>

static void button_render(uint8_t x, uint8_t y, struct gui_button_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (this->flags & GUI_FLAG_HIDDEN)
    {
        return;
    }

    if (is_object_invalidated(this))
    {
        zxgui_screen_color(INK_WHITE | BRIGHT | PAPER_BLACK);
        zxgui_screen_put(x, y, this->icon);

        if (this->title)
        {
            x++;

            font_state.fgnd_attr = INK_WHITE | PAPER_BLACK;
            fzx_at(&font_state, x * 8 + 1, y * 8 + 1);
            fzx_puts(&font_state, (char*)this->title);
        }
    }
}

static uint8_t button_event(enum gui_event_type event_type, void* event, struct gui_button_t* this, struct gui_scene_t* scene)
{
    switch (event_type)
    {
        case GUI_EVENT_KEY_PRESSED:
        {
            struct gui_event_key_pressed* ev = event;
            if (ev->key == this->key)
            {
                this->pressed(this);
                return 1;
            }

            break;
        }
    }

    return 0;
}

void zxgui_button_init(struct gui_button_t* button, uint8_t x, uint8_t y, uint8_t w,
    uint8_t h, uint8_t key, uint8_t icon, const char* title, gui_button_pressed_f pressed)
{
    button->render = (gui_render_f)button_render;
    button->event = (gui_event_f)button_event;
    button->flags = GUI_FLAG_DIRTY;
    button->next = NULL;
    button->key = key;
    button->icon = icon;
    button->title = title;
    button->x = x;
    button->y = y;
    button->w = w;
    button->h = h;
    button->pressed = pressed;
}