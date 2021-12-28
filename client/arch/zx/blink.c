#include <stdint.h>
#include "zxgui.h"

void _render_blink_even(uint8_t *addr) __z88dk_fastcall __naked
{
#asm
    ld b, 8
_render_blink_even_loop:
    ld a, (hl)
    xor $0F
    ld (hl), a
    inc h
    dec b
    jp nz, _render_blink_even_loop
    ret
#endasm
}

void _render_blink_odd(uint8_t *addr) __z88dk_fastcall __naked
{
#asm
    ld b, 8
_render_blink_odd_loop:
    ld a, (hl)
    xor $F0
    ld (hl), a
    inc h
    dec b
    jp nz, _render_blink_odd_loop
    ret
#endasm
}

static void _clear_blink_even(uint8_t *addr) __z88dk_fastcall __naked
{
#asm
    ld b, 8
_clear_blink_even_loop:
    ld a, (hl)
    and $F0
    ld (hl), a
    inc h
    dec b
    jp nz, _clear_blink_even_loop
    ret
#endasm
}

static void _clear_blink_odd(uint8_t *addr) __z88dk_fastcall __naked
{
#asm
    ld b, 8
_clear_blink_odd_loop:
    ld a, (hl)
    and $0F
    ld (hl), a
    inc h
    dec b
    jp nz, _clear_blink_odd_loop
    ret
#endasm
}

void clear_blink(struct gui_edit_t* this) __z88dk_fastcall
{
    if (this->cursor_even)
    {
        _clear_blink_even(this->cursor_pixels_addr);
    }
    else
    {
        _clear_blink_odd(this->cursor_pixels_addr);
    }
}
