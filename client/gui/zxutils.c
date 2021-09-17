#include <spectrum.h>
#include <string.h>
#include "zxgui.h"
#include "zxgui_tiles.h"

uint8_t screen_color = 0;

void zxgui_screen_put(uint8_t x, uint8_t y, uint8_t ch) __z88dk_callee
{
    uint8_t* data = get_gui_tiles() + ch * 8;

    uint8_t* addr = zx_cxy2saddr(x, y);

    for (uint8_t i = 0; i < 8; i++)
    {
        *addr = *data++;
        addr += 256;
    }

    *zx_cxy2aaddr(x, y) = screen_color;
}

void zxgui_screen_clear(uint8_t x, uint8_t y, uint8_t w, uint8_t h) __z88dk_callee
{
    uint8_t* c = zx_cxy2aaddr(x, y);

    for (uint8_t yh = y, yhh = y + h; yh < yhh; yh++)
    {
        uint8_t* addr = zx_cxy2saddr(x, yh);

        uint8_t i = 8;
        while (--i)
        {
            memset(addr, 0, w); addr += 256;
        }

        memset(c, screen_color, w);
        c += 32;
    }
}

void zxgui_screen_recolor(uint8_t x, uint8_t y, uint8_t w, uint8_t h) __z88dk_callee
{
    uint8_t* c = zx_cxy2aaddr(x, y);

    for (uint8_t yh = y, yhh = y + h; yh < yhh; yh++)
    {
        memset(c, screen_color, w);
        c += 32;
    }
}