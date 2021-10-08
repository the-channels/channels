
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

static void _clear_blink_even(uint8_t *addr) __z88dk_fastcall __naked
{
#asm
    ld b, 8
_clear_blink_even_loop:
    ld a, (hl)
    and $F0
    ld (hl), a
    inc h
    dec b
    jp nz, _clear_blink_even_loop
    ret
#endasm
}

static void _clear_blink_odd(uint8_t *addr) __z88dk_fastcall __naked
{
#asm
    ld b, 8
_clear_blink_odd_loop:
    ld a, (hl)
    and $0F
    ld (hl), a
    inc h
    dec b
    jp nz, _clear_blink_odd_loop
    ret
#endasm
}

static void clear_blink(struct gui_edit_t* this) __z88dk_fastcall
{
    if (this->cursor_even)
    {
        _clear_blink_even(this->cursor_pixels_addr);
    }
    else
    {
        _clear_blink_odd(this->cursor_pixels_addr);
    }
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
        }

        zxgui_screen_color(PAPER_BLACK | INK_BLACK);

        if (this->value[0])
        {
            text_ui_color(BRIGHT | c);

            if (this->flags & GUI_FLAG_MULTILINE)
            {
                char* c = (char*)this->value;
                const char* end = c + strlen(c);
                uint8_t w = this->w - 1;
                uint8_t offset_y = 0;
                uint8_t max_offset_y = this->h - 1;
                uint8_t last_line_len = 0;

                do
                {
                    char* next_line = text_ui_buffer_partition(c, (uint16_t) (end - c), w);

                    if (c != next_line)
                    {
                        last_line_len = next_line - c;
                        uint8_t text_l = (last_line_len >> 1) + (last_line_len & 0x01);
                        zxgui_screen_clear(x + text_l, y + offset_y, this->w - 1 - text_l, 1);
                        text_ui_write_at(x, y + offset_y, c, last_line_len);
                    }

                    if (*next_line == '\n')
                    {
                        next_line++;
                        last_line_len = 0;
                    }

                    offset_y++;
                    if (offset_y >= max_offset_y)
                    {
                        break;
                    }
                    c = next_line;
                } while (c < end);

                uint8_t xx = x + (last_line_len >> 1);

                if (offset_y < max_offset_y)
                {
                    zxgui_screen_clear(x, y + offset_y, this->w - 1, 1);
                }

                this->last_text_height = offset_y;

                if (last_line_len != 0)
                {
                    offset_y--;
                }

                this->cursor_even = (last_line_len & 0x01);
                this->cursor_pixels_addr = zx_cxy2saddr(xx, y + offset_y);
                this->cursor_color_addr = zx_cxy2aaddr(xx, y + offset_y);
            }
            else
            {

                uint8_t w = strlen((char *) this->value);

                uint8_t half_w = (w >> 1);
                uint8_t xx = x + half_w;

                zxgui_screen_clear(x + half_w, y, this->w - 1 - half_w, 1);
                text_ui_write_at(x, y, this->value, w);

                this->cursor_even = (w & 0x01);
                this->cursor_pixels_addr = zx_cxy2saddr(xx, y);
                this->cursor_color_addr = zx_cxy2aaddr(xx, y);
            }
        }
        else
        {
            zxgui_screen_clear(x, y, this->w - 1, this->h - 1);

            this->cursor_even = 0;
            this->cursor_pixels_addr = zx_cxy2saddr(x, y);
            this->cursor_color_addr = zx_cxy2aaddr(x, y);
        }

        blink = 100;
    }

    if (scene->focus == (void*)this)
    {
        if (blink++ > 100)
        {
            blink = 0;

            if (this->cursor_even)
            {
                _render_blink_even(this->cursor_pixels_addr);
            }
            else
            {
                _render_blink_odd(this->cursor_pixels_addr);
            }

            (*this->cursor_color_addr) = INK_YELLOW | BRIGHT | PAPER_BLACK;
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
            char key = ev->key;

            switch (key)
            {
                // backspace
                case 12:
                {
                    int len = strlen(this->value);
                    if (len)
                    {
                        clear_blink(this);

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
                    if ((this->flags & GUI_FLAG_MULTILINE) == 0)
                    {
                        return 0;
                    }
                    else
                    {
                        if (zxgui_label_text_height(this->w - 1, this->value, strlen(this->value), this->h)
                                >= this->h - 2)
                        {
                            return 0;
                        }
                    }
                    /* fall through */
                    key = '\n';
                }
                default:
                {
                    int len = strlen(this->value);
                    if (len >= this->value_size - 1)
                    {
                        return 0;
                    }
                    if ((this->flags & GUI_FLAG_MULTILINE) == 0)
                    {
                        if (len >= ((this->w - 1) << 1))
                        {
                            return 0;
                        }
                    }

                    clear_blink(this);

                    this->value[len] = key;
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

void zxgui_edit_init(struct gui_edit_t* edit, xywh_t xywh, char* buffer, uint16_t buffer_size) ZXGUI_CDECL
{
    zxgui_base_init(edit, xywh, _edit_render, _edit_event);

    edit->flags = GUI_FLAG_DIRTY;
    edit->value = buffer;
    edit->value_size = buffer_size;
}

void zxgui_multiline_edit_init(struct gui_edit_t* edit, xywh_t xywh, char* buffer, uint16_t buffer_size) ZXGUI_CDECL
{
    zxgui_edit_init(edit, xywh, buffer, buffer_size);

    edit->flags |= GUI_FLAG_MULTILINE;
}