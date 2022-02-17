#include "system.h"
#include <string.h>

void clear_screen_with(uint8_t color)
{
    zx_colour(color);
    zx_cls();
}

void system_init()
{
    // never page out
    pagein();
}

static void get_mounted_path(char* into) __z88dk_fastcall __naked
{
#asm
    ld (hl), $00            ;// reset a string to 0
    push hl

    ld de, $01ff            ;// AM_CONFIG_SECTION
    ld hl, $FE02            ;// CFG_FINDSECTION
    rst $28                 ;// MODULECALL_NOPAGE
    jr c, no_config	        ;// we don't have a section, jump forward.

    pop hl
    ld de, hl               ;// load 'into' into de
    ld a, ($3F6F)	        ;// get 'v_vfs_curmount'
    ld hl, $FE03            ;// CFG_GETCFSTRING
    rst $28                 ;// MODULECALL_NOPAGE
    jr c, no_option
    ret
    no_config:
    pop hl
    no_option:
    ret
#endasm
}

void get_default_connect_address(char* into)
{
    get_mounted_path(into);
}

uint8_t test_network_capabilities()
{
    return spectranet_detect();
}