#include "text_ui.h"
#include "system.h"
#include <string.h>

uint8_t text_x, text_y, text_color;

void text_ui_init(void)
{
}

void text_ui_puts_at(uint8_t x, uint8_t y, const char* s)
{
    text_ui_write_at(x, y, s, strlen(s));
}

char* text_ui_buffer_partition(char *buf, uint16_t buflen, uint8_t allowed_width)
{
#if CHARACTERS_PER_CELL == (2)
    allowed_width = allowed_width << 1; // two chars per cell
#endif
    uint8_t exceeded_len = buflen > allowed_width;
    uint8_t len = exceeded_len ? allowed_width : buflen;
    char* next_line = memchr(buf, '\n', len);
    if (next_line)
    {
        return next_line;
    }
    if (exceeded_len)
    {
        char *p = memrchr(buf, ' ', len);
        return p ? p : buf + len;
    }
    return buf + len;
}

void text_ui_color(uint8_t color) __z88dk_fastcall
{
    text_color = color;
}
