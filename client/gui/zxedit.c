
#include <string.h>
#include "zxgui.h"
#include "text_ui.h"
#include <stdint.h>
#include <spectrum.h>

static uint32_t blink = 0;

static void _render_blink_even(uint8_t *addr) __z88dk_fastcall __naked
{
#asm
    ld b, 8
_render_blink_even_loop:
    ld a, (hl)
    xor $0F
    ld (hl), a
    inc h
    dec b
    jp nz, _render_blink_even_loop
    ret
#endasm
}

static void _render_blink_odd(uint8_t *addr) __z88dk_fastcall __naked
{
#asm
    ld b, 8
_render_blink_odd_loop:
    ld a, (hl)
    xor $F0
    ld (hl), a
    inc h
    dec b
    jp nz, _render_blink_odd_loop
    ret
#endasm
}

static void _edit_render(uint8_t x, uint8_t y, struct gui_edit_t* this, struct gui_scene_t* scene)
{
    x += this->x + 1;
    y += this->y + 1;

    uint8_t flags = is_object_invalidated(this);
    if (flags)
    {
        uint8_t c = (scene->focus == (void*)this) ? (INK_YELLOW | PAPER_BLACK) : (INK_WHITE | PAPER_BLACK);

        if (flags & GUI_FLAG_DIRTY)
        {
            zxgui_rectangle(BRIGHT | c,
                x - 1, y - 1, this->w, this->h, GUI_EDIT_LEFT_BOTTOM_CORNER);

            zxgui_screen_color(c);
            zxgui_screen_clear(x, y, this->w - 1, 1);
        }

        zxgui_screen_color(c);
        zxgui_screen_clear(x, y, this->w - 1, 1);

        if (this->value[0])
        {
            text_ui_color(BRIGHT | c);
            text_ui_puts_at(x, y, this->value);
        }

        blink = 100;
    }

    if (scene->focus == (void*)this)
    {
        if (blink++ > 100)
        {
            blink = 0;
            uint8_t w = strlen((char *) this->value);
            uint8_t xx = x + (w >> 1);
            uint8_t* addr = zx_cxy2saddr(xx, y);

            if (w & 0x01)
            {
                _render_blink_even(addr);
            }
            else
            {
                _render_blink_odd(addr);
            }

            *zx_cxy2aaddr(xx, y) = INK_YELLOW | BRIGHT | PAPER_BLACK;
        }
    }


}

static uint8_t _edit_event(enum gui_event_type event_type, void* event, struct gui_edit_t* this, struct gui_scene_t* scene)
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
                        blink = 200;
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
                    if (len >= this->value_size - 1)
                    {
                        return 0;
                    }
                    this->value[len] = ev->key;
                    len++;
                    this->value[len] = 0;
                    blink = 200;
                    object_invalidate(this, GUI_FLAG_DIRTY_INTERNAL);
                    return 1;
                }
            }
        }
    }

    return 0;
}

void zxgui_edit_init(struct gui_edit_t* edit, xywh_t xywh,
    char* buffer, uint16_t buffer_size) ZXGUI_CDECL
{
    zxgui_base_init(edit, xywh, _edit_render, _edit_event);

    edit->flags = GUI_FLAG_DIRTY;
    edit->value = buffer;
    edit->value_size = buffer_size;
}