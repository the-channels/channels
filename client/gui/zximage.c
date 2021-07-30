#include <string.h>
#include "zxgui.h"
#include "zxgui_internal.h"
#include "system.h"

static void image_render(uint8_t x, uint8_t y, struct gui_image_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (is_object_invalidated(this))
    {
        zxgui_image(x, y, this->w, this->h, this->source, this->colors);
    }
}

void zxgui_image_init(struct gui_image_t* image, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source, const uint8_t* colors)
{
    image->render = (gui_render_f)image_render;
    image->event = NULL;
    image->flags = GUI_FLAG_DIRTY;
    image->next = NULL;
    image->source = source;
    image->colors = colors;
    image->x = x;
    image->y = y;
    image->w = w;
    image->h = h;
}

static void dynamic_image_render(uint8_t x, uint8_t y, struct gui_dynamic_image_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (is_object_invalidated(this))
    {
        uint16_t r;
        const uint8_t* d = this->obtain_data(this, &r);
        if (d == NULL)
        {
            return;
        }

        uint8_t recv_color_now = 0;
        uint8_t display_row_num = 0;
        uint16_t receive_w = this->w;
        uint8_t* display_ptr = zx_cxy2saddr(x, y);
        uint8_t* display_color_ptr = zx_cxy2aaddr(x, y);
        uint8_t* display_row_ptr = display_ptr;
        uint8_t* display_color_row_ptr = display_color_ptr;

        while (r)
        {
            if (recv_color_now)
            {
                uint8_t allow = r > receive_w ? receive_w : r;

                memcpy(display_color_ptr, d, allow);

                d += allow;
                r -= allow;
                display_color_ptr += allow;
                receive_w -= allow;

                if (receive_w == 0)
                {
                    display_color_row_ptr += 32;
                    display_color_ptr = display_color_row_ptr;
                    receive_w = this->w;
                }
            }
            else
            {
                uint8_t allow = r > receive_w ? receive_w : r;

                memcpy(display_row_ptr, d, allow);
                d += allow;
                r -= allow;
                display_row_ptr += allow;
                receive_w -= allow;

                if (receive_w == 0)
                {
                    display_row_num++;
                    display_ptr = zx_saddrpdown(display_ptr);
                    display_row_ptr = display_ptr;
                    receive_w = this->w;

                    if (display_row_num == this->h * 8)
                    {
                        recv_color_now = 1;
                    }
                }
            }
        }

        this->release_data(this);
    }
}

void zxgui_dynamic_image_init(struct gui_dynamic_image_t* image, uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    gui_label_obtain_image_f obtain_data, gui_label_release_image_f release_data, void* user)
{
    image->render = (gui_render_f)dynamic_image_render;
    image->event = NULL;
    image->flags = GUI_FLAG_DIRTY;
    image->next = NULL;
    image->x = x;
    image->y = y;
    image->w = w;
    image->h = h;
    image->obtain_data = obtain_data;
    image->release_data = release_data;
    image->user = user;
}