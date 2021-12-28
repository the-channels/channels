#include <string.h>
#include "system.h"
#include "desktop.h"
#include "zxgui.h"
#include <wchar.h>
#include <stdio.h>

void zxgui_screen_put(uint8_t x, uint8_t y, uint8_t ch) __z88dk_callee
{
    *get_character_data_at(x, y) = tile_to_char((char)ch);
    *get_color_data_at(x, y) = screen_color;
    set_screen_dirty();
}

void zxgui_screen_clear(uint8_t x, uint8_t y, uint8_t w, uint8_t h) __z88dk_callee
{
    for (uint8_t j = y; j < y + h; j++)
    {
        wchar_t* c = get_character_data_at(x, j);
        wmemset(c, ' ', w);
        memset(get_color_data_at(x, j), screen_color, w);
    }

    set_screen_dirty();
}

void zxgui_screen_recolor(uint8_t x, uint8_t y, uint8_t w, uint8_t h) __z88dk_callee
{
    for (uint8_t j = y; j < y + h; j++)
    {
        memset(get_color_data_at(x, j), screen_color, w);
    }
    set_screen_dirty();
}