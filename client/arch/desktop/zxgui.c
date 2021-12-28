#include "zxgui.h"
#include "system.h"
#include "desktop.h"
#include <string.h>
#include <stdio.h>
#include <wchar.h>

void zxgui_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source, const uint8_t* colors) __z88dk_callee
{
    for (uint8_t j = y; j < y + h; j++)
    {
        for (uint8_t i = x; i < x + w; i++)
        {
            //
        }
    }
}

void zxgui_line(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t c) __z88dk_callee
{
    wchar_t* d = get_character_data_at(x, y);
    wmemset(d, tile_to_char(c), w);
    memset(get_color_data_at(x, y), screen_color, w);
    set_screen_dirty();
}
