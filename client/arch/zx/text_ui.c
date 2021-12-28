#include <stdint.h>
#include <spectrum.h>
#include <string.h>
#include "system.h"
#include "text_ui.h"

void text_ui_write_at(uint8_t x, uint8_t y, const char* buf, uint8_t buflen) __z88dk_callee
{
    text_x = x;
    text_y = y;
    text_ui_write(buf, buflen);

    uint8_t* c = zx_cxy2aaddr(x, y);
    memset(c, text_color, (buflen / CHARACTERS_PER_CELL) + (buflen & 0x01));
}