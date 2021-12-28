
#include <string.h>
#include "zxgui.h"
#include "text_ui.h"
#include <stdint.h>
#include "system.h"

#ifdef HAS_SYS_TIME
#include <sys/time.h>
time_t blink = 0;
#else
static int blink = 0;
#endif

#ifndef __SPECTRUM
static uint32_t blink_flip = 0;
#endif


static void set_cursor_addresses(struct gui_edit_t* this, uint8_t x, uint8_t y) __z88dk_callee
{
#ifdef __SPECTRUM
    this->cursor_pixels_addr = zx_cxy2saddr(x, y);
    this->cursor_color_addr = zx_cxy2aaddr(x, y);
#else
    this->cursor_x = x;
    this->cursor_y = y;
#endif
}

static void _edit_render(uint8_t x, uint8_t y, struct gui_edit_t* this, struct gui_scene_t* scene)
{
    x += this->x + 1;
    y += this->y + 1;

    uint8_t flags = is_object_invalidated(this);
    if (flags)
    {
        uint8_t col = (scene->focus == (void*)this) ? (COLOR_BRIGHT | COLOR_FG_YELLOW | COLOR_BG_BLACK) : (COLOR_BRIGHT | COLOR_FG_WHITE | COLOR_BG_BLACK);

        if (flags & GUI_FLAG_DIRTY)
        {
            zxgui_rectangle(col,
                x - 1, y - 1, this->w, this->h, GUI_EDIT_LEFT_BOTTOM_CORNER);
        }

        zxgui_screen_color(COLOR_BG_BLACK | COLOR_FG_BLACK);
        text_ui_color(col);

        if ((this->flags & GUI_FLAG_MULTILINE) && this->value[0])
        {
            char* c = (char*)this->value;
            const char* end = c + strlen(c);
            uint8_t w = this->w - 2;
            uint8_t offset_y = y;
            uint8_t max_offset_y = y + this->h - 1;
            uint8_t last_line_len = 0;

            do
            {
                char* next_line = text_ui_buffer_partition(c, (uint16_t) (end - c), w);

                if (c != next_line)
                {
                    last_line_len = next_line - c;
#if CHARACTERS_PER_CELL == (2)
                    uint8_t text_l = (last_line_len >> 1) + (last_line_len & 0x01);
#else
                    uint8_t text_l = last_line_len;
#endif
                    zxgui_screen_clear(x + text_l, offset_y, this->w - 1 - text_l, 1);
                    text_ui_write_at(x, offset_y, c, last_line_len);
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

#if CHARACTERS_PER_CELL == (2)
            uint8_t xx = x + (last_line_len >> 1);
#else
            uint8_t xx = x + last_line_len;
#endif

            if (offset_y < max_offset_y)
            {
                zxgui_screen_clear(x, offset_y, this->w - 1, 1);
            }

            if (last_line_len != 0)
            {
                offset_y--;
            }

#ifdef __SPECTRUM
            this->cursor_even = (last_line_len & 0x01);
#endif
            set_cursor_addresses(this, xx, offset_y);
        }
        else
        {
            uint8_t w = strlen((char *) this->value);

#if CHARACTERS_PER_CELL == (2)
            uint8_t half_w = (w >> 1);
#else
            uint8_t half_w = w;
#endif
            uint8_t xx = x + half_w;

            zxgui_screen_clear(xx, y, this->w - 1 - half_w, 1);
            if (this->value[0])
            {
                text_ui_write_at(x, y, this->value, w);
            }
#ifdef __SPECTRUM
            this->cursor_even = (w & 0x01);
#endif
            set_cursor_addresses(this, xx, y);
        }
        blink = BLINK_INTERVAL;
    }

    if (scene->focus == (void*)this)
    {
#ifdef HAS_SYS_TIME
        struct timeval now;
        gettimeofday(&now, NULL);
        time_t time_now = now.tv_sec * 1000 + (now.tv_usec / 1000);

        if (time_now > blink + BLINK_INTERVAL)
#else
        if (blink++ > BLINK_INTERVAL)
#endif

        {
#ifdef HAS_SYS_TIME
            blink = time_now;
#else
            blink = 0;
#endif

#ifdef __SPECTRUM
            if (this->cursor_even)
            {
                _render_blink_even(this->cursor_pixels_addr);
            }
            else
            {
                _render_blink_odd(this->cursor_pixels_addr);
            }

            (*this->cursor_color_addr) = COLOR_FG_YELLOW | COLOR_BRIGHT | COLOR_BG_BLACK;
#else
            blink_flip = !blink_flip;
            zxgui_screen_color(COLOR_BG_BLACK | COLOR_FG_YELLOW | COLOR_BRIGHT);
            zxgui_screen_put(this->cursor_x, this->cursor_y, blink_flip ? GUI_EDIT_CURSOR : GUI_ICON_EMPTY);
#endif
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
                case GUI_KEY_CODE_BACKSPACE:
                {
                    int len = strlen(this->value);
                    if (len)
                    {
#ifdef __SPECTRUM
                        clear_blink(this);
#endif

                        len--;
                        this->value[len] = 0;
                        blink = BLINK_INTERVAL;

                        object_invalidate(this, GUI_FLAG_DIRTY_INTERNAL);
                    }
                    return 1;
                }
                // enter
                case GUI_KEY_CODE_RETURN:
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
#if CHARACTERS_PER_CELL == (2)
                        if (len >= ((this->w - 1) << 1))
#else
                        if (len >= this->w - 1)
#endif
                        {
                            return 0;
                        }
                    }

#ifdef __SPECTRUM
                    clear_blink(this);
#endif

                    this->value[len] = key;
                    len++;
                    this->value[len] = 0;
                    blink = BLINK_INTERVAL;

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