
#include <string.h>
#include "zxgui.h"
#include "zxgui_internal.h"
#include <font/fzx.h>
#include <netlog.h>
#include "system.h"

static void label_render(uint8_t x, uint8_t y, struct gui_label_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (is_object_invalidated(this))
    {
        zxgui_screen_color(this->color);
        zxgui_screen_clear(x, y, this->w, this->h);

        font_state.fgnd_attr = this->color;

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
                        font_state.fgnd_attr = INK_GREEN | PAPER_BLACK;
                    }
                    else
                    {
                        font_state.fgnd_attr = this->color;
                    }
                }

                char* next_line = fzx_buffer_partition_ww(font_state.font, c, (uint16_t)(end - c), w);

                if (next_line == c)
                {
                    next_line = fzx_buffer_partition(font_state.font, c, (uint16_t)(end - c), w);
                }

                if (c != next_line)
                {
                    fzx_at(&font_state, offset_x, offset_y);
                    fzx_write(&font_state, c, next_line - c);
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
            fzx_at(&font_state, x * 8 + 1, y * 8 + 1);
            fzx_puts(&font_state, (char*)this->title);
        }
    }
}

uint8_t zxgui_label_text_height(uint8_t w, const char* title, uint16_t len)
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

        char* next_line = fzx_buffer_partition_ww(font_state.font, c, (uint16_t)(end - c), w);

        if (next_line == c)
        {
            next_line = fzx_buffer_partition(font_state.font, c, (uint16_t)(end - c), w);
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

void zxgui_label_init(struct gui_label_t* label, uint8_t x, uint8_t y, uint8_t w,
    uint8_t h, const char* title, uint8_t color, uint8_t flags)
{
    label->render = (gui_render_f)label_render;
    label->event = NULL;
    label->flags = GUI_FLAG_DIRTY | flags;
    label->next = NULL;
    label->title = title;
    label->color = color;
    label->x = x;
    label->y = y;
    label->w = w;
    label->h = h;
}

static void dynamic_label_render(uint8_t x, uint8_t y, struct gui_dynamic_label_t* this, struct gui_scene_t* scene)
{
    this->parent.title = this->obtain_data(this);
    label_render(x, y, (struct gui_label_t*)this, scene);
    this->release_data(this);
}

void zxgui_dynamic_label_init(struct gui_dynamic_label_t* label, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color,
    uint8_t flags, gui_label_obtain_title_data_f obtain_data, gui_label_release_title_data_f release_data, void* user)
{
    zxgui_label_init((struct gui_label_t*)label, x, y, w, h, NULL, color, flags);
    label->parent.render = (gui_render_f)dynamic_label_render;
    label->obtain_data = obtain_data;
    label->release_data = release_data;
    label->user = user;
}