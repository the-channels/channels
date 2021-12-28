#include "zxgui.h"
#include "zxgui_tiles.h"
#include "system.h"
#include <string.h>

void zxgui_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source, const uint8_t* colors) __z88dk_callee
{
    for (uint8_t j = y; j < y + h; j++)
    {
        for (uint8_t i = x; i < x + w; i++)
        {
            uint8_t* addr = zx_cxy2saddr(i, j);

            for (uint8_t ii = 0; ii < 8; ii++)
            {
                *addr = *source++;
                addr += 256;
            }

            *zx_cxy2aaddr(i, j) = *colors++;
        }
    }
}

void zxgui_line(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t c) __z88dk_callee
{
    uint8_t* data = get_gui_tiles() + c * 8;
    uint8_t* addr = zx_cxy2saddr(x, y);

    for (uint8_t i = 0; i < 8; i++)
    {
        memset(addr, *data++, w);
        addr += 256;
    }

    memset(zx_cxy2aaddr(x, y), form_color, w);
}
