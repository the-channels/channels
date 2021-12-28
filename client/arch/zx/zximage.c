#include "zxgui.h"
#include "system.h"
#include <string.h>

void _dynamic_image_render(uint8_t x, uint8_t y, struct gui_dynamic_image_t* this, struct gui_scene_t* scene)
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
    }
}
