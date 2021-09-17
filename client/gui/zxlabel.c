
#include <string.h>
#include "zxgui.h"
#include <spectrum.h>
#include <fzx_ui.h>

static void _label_render(uint8_t x, uint8_t y, struct gui_label_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (!is_object_invalidated(this))
        return;

    zxgui_screen_color(this->color);
    zxgui_screen_clear(x, y, this->w, this->h);
    fzx_ui_color(this->color);

    uint16_t w = this->w * 8;

    if (this->flags & GUI_FLAG_MULTILINE)
    {
        char* c = (char*)this->title;
        const char* end = c + strlen(c);
        uint8_t offset_x = x * 8 + 1;
        uint8_t offset_y = y * 8 + 1;
        uint8_t max_offset_y = offset_y + this->h * 8;
        uint8_t new_line = 1;

        do
        {
            while (*c == ' ')
            {
                // skip spaces at beginning of the line
                c++;
            }

            if (new_line)
            {
                if (*c == '>' && (*(c + 1) != '>'))
                {
                    fzx_ui_color(INK_GREEN | PAPER_BLACK);
                }
                else
                {
                    fzx_ui_color(this->color);
                }
            }

            char* next_line = fzx_ui_buffer_partition_ww(c, (uint16_t)(end - c), w);

            if (next_line == c)
            {
                next_line = fzx_ui_buffer_partition(c, (uint16_t)(end - c), w);
            }

            if (c != next_line)
            {
                fzx_ui_write_at(offset_x, offset_y, c, next_line - c);
            }

            if (*next_line == '\n')
            {
                new_line = 1;
                next_line++;
            }
            else
            {
                new_line = 0;
            }

            offset_y += 8;
            if (offset_y >= max_offset_y)
            {
                break;
            }
            c = next_line;
        } while (c < end);
    }
    else
    {
        fzx_ui_puts_at(x * 8 + 1, y * 8 + 1, this->title);
    }
}

uint8_t zxgui_label_text_height(uint8_t w, const char* title, uint16_t len) ZXGUI_CDECL
{
    w--;

    char* c = (char*)title;
    const char* end = c + len;
    uint8_t res = 0;

    do
    {
        while (*c == ' ')
        {
            // skip spaces at beginning of the line
            c++;
        }

        char* next_line = fzx_ui_buffer_partition_ww(c, (uint16_t)(end - c), w);

        if (next_line == c)
        {
            next_line = fzx_ui_buffer_partition(c, (uint16_t)(end - c), w);
        }

        if (next_line == c)
        {
            next_line = memchr(c, '\n', (uint16_t)(end - c));
            if (next_line == NULL)
            {
                return 3;
            }
        }

        if (*next_line == '\n')
        {
            next_line++;
        }

        res++;

        if (res >= 32)
        {
            return 3;
        }

        c = next_line;
    } while (c < end);

    return res;
}

void zxgui_label_init(struct gui_label_t* label, xywh_t xywh,
    const char* title, uint8_t color, uint8_t flags) ZXGUI_CDECL
{
    zxgui_base_init(label, xywh, _label_render, NULL);

    label->flags = GUI_FLAG_DIRTY | flags;
    label->title = title;
    label->color = color;
}

void _dynamic_label_render(uint8_t x, uint8_t y, struct gui_dynamic_label_t* this, struct gui_scene_t* scene)
{
    this->parent.title = this->obtain_data(this);
    _label_render(x, y, (struct gui_label_t*)this, scene);
}

void zxgui_dynamic_label_init(struct gui_dynamic_label_t* label, xywh_t xywh,
    uint8_t color, uint8_t flags, gui_label_obtain_title_data_f obtain_data, void* user) ZXGUI_CDECL
{
    zxgui_label_init((struct gui_label_t*)label, xywh, NULL, color, flags);
    label->parent.render = (gui_render_f)_dynamic_label_render;
    label->obtain_data = obtain_data;
    label->user = user;
}