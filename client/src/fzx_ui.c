#include "fzx_ui.h"
#include "fzx.h"
#include <spectrum.h>

static struct fzx_state font_state;
static struct r_Rect16 screen;

void fzx_ui_init(void)
{
    screen.x = 0;
    screen.width = 256;
    screen.y = 0;
    screen.height = 192;

    fzx_state_init(&font_state, &ff_utz_TinyTexanS, &screen);

    font_state.fgnd_attr = BRIGHT | INK_GREEN | PAPER_BLACK;
    font_state.fgnd_mask = 0;
    font_state.fzx_draw = _fzx_draw_or;
    font_state.left_margin = 0;

    font_state.y = 32;
    font_state.x = 0;
}

void fzx_ui_at(uint8_t x, uint8_t y)
{
    fzx_at(&font_state, x, y);
}

void fzx_ui_puts(const char* s)
{
    fzx_puts(&font_state, (char*)s);
}

void fzx_ui_puts_at(uint8_t x, uint8_t y, const char* s)
{
    fzx_at(&font_state, x, y);
    fzx_puts(&font_state, (char*)s);
}

void fzx_ui_write_at(uint8_t x, uint8_t y, const char* buf, uint16_t buflen) __z88dk_callee
{
    fzx_at(&font_state, x, y);
    fzx_write(&font_state, (char*)buf, buflen);
}

char* fzx_ui_buffer_partition_ww(char *buf, uint16_t buflen, uint16_t allowed_width)
{
    return fzx_buffer_partition_ww(font_state.font, buf, buflen, allowed_width);
}

char* fzx_ui_buffer_partition(char *buf, uint16_t buflen, uint16_t allowed_width)
{
    return fzx_buffer_partition(font_state.font, buf, buflen, allowed_width);
}

uint16_t fzx_ui_string_extent(const char *s)
{
    return fzx_string_extent(font_state.font, (char*)s);
}

void fzx_ui_switch_xor(void)
{
    font_state.fzx_draw = _fzx_draw_xor;
}

void fzx_ui_switch_or(void)
{
    font_state.fzx_draw = _fzx_draw_or;
}

void fzx_ui_set_paper(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    struct r_Rect16* paper = &font_state.paper;

    paper->x = x;
    paper->y = y;
    paper->width = w;
    paper->height = h;
}

void fzx_ui_color(uint8_t color)
{
    font_state.fgnd_attr = color;
}