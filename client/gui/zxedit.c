
#include <string.h>
#include "zxgui.h"
#include "zxgui_internal.h"
#include <font/fzx.h>
#include "system.h"
#include <stdint.h>

static uint32_t blink = 0;

static void edit_render(uint8_t x, uint8_t y, struct gui_edit_t* this, struct gui_scene_t* scene)
{
    x += this->x + 1;
    y += this->y + 1;

    uint8_t flags = is_object_invalidated(this);
    if (flags)
    {
        if (flags & GUI_FLAG_DIRTY)
        {
            zxgui_rectangle(INK_YELLOW | BRIGHT | PAPER_BLACK,
                x - 1, y - 1, this->w, this->h, GUI_EDIT_LEFT_BOTTOM_CORNER);

        }

        zxgui_screen_color(INK_WHITE | PAPER_BLACK);
        zxgui_screen_clear(x, y, this->w - 1, 1);

        if (this->value[0])
        {
            font_state.fgnd_attr = INK_YELLOW | BRIGHT | PAPER_BLACK;
            fzx_at(&font_state, x * 8, y * 8 + 1);
            fzx_puts(&font_state, this->value);
        }

        blink = 200;

    }

    if (scene->focus == (void*)this)
    {
        if (blink++ > 200)
        {
            blink = 0;

            int w = 2 + fzx_string_extent(font_state.font, (char*)this->value);

            font_state.fzx_draw = _fzx_draw_xor;
            font_state.fgnd_attr = INK_YELLOW | BRIGHT | PAPER_BLACK;
            fzx_at(&font_state, x * 8 + w, y * 8 + 1);
            fzx_puts(&font_state, "_");
            font_state.fzx_draw = _fzx_draw_or;
        }
    }


}

static uint8_t edit_event(enum gui_event_type event_type, void* event, struct gui_edit_t* this, struct gui_scene_t* scene)
{
    if (scene->focus != (void*)this)
        return 0;

    switch (event_type)
    {
        case GUI_EVENT_KEY_PRESSED:
        {
            struct gui_event_key_pressed* ev = event;

            switch (ev->key)
            {
                // backspace
                case 12:
                {
                    int len = strlen(this->value);
                    if (len)
                    {
                        len--;
                        this->value[len] = 0;
                        object_invalidate(this, GUI_FLAG_DIRTY_INTERNAL);
                    }
                    return 1;
                }
                // enter
                case 13:
                {
                    return 0;
                }
                default:
                {
                    int len = strlen(this->value);
                    if (len >= sizeof(this->value) - 1)
                    {
                        return 0;
                    }
                    this->value[len] = ev->key;
                    len++;
                    this->value[len] = 0;
                    object_invalidate(this, GUI_FLAG_DIRTY_INTERNAL);
                    return 1;
                }
            }
        }
    }

    return 0;
}


void zxgui_edit_init(struct gui_edit_t* edit, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    edit->render = (gui_render_f)edit_render;
    edit->event = (gui_event_f)edit_event;
    edit->flags = GUI_FLAG_DIRTY;
    edit->next = NULL;
    memset(edit->value, 0, sizeof(edit->value));
    edit->x = x;
    edit->y = y;
    edit->w = w;
    edit->h = h;
}