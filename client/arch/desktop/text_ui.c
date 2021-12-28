#include <inttypes.h>
#include <string.h>
#include "desktop.h"
#include <wchar.h>
#include <text_ui.h>

void text_ui_write_at(uint8_t x, uint8_t y, const char* buf, uint8_t buflen)
{
    wchar_t* d = get_character_data_at(x, y);
    for (int i = 0; i < buflen; i++)
    {
        *d++ = (unsigned char)*buf++;
    }
    memset(get_color_data_at(x, y), text_color, buflen);
    set_screen_dirty();
}