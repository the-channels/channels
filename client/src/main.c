#include "zxgui.h"
#include "scenes.h"
#include <spectranet.h>
#include <zxgui.h>
#include "system.h"
#include "channels.h"
#include "proto_asserts.h"
#include "netlog.h"
#include "heap.h"

long heap = 0;
uint8_t fake_CRT_FONT_64 = 0;

void init_scenes()
{
    init_progress();
    init_alert();
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
    ld a, $00	            ;// get 'AM_FS0' option
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

void switch_main()
{
    int detected = spectranet_detect();
    zx_border(INK_BLACK);
    if (detected)
    {
        switch_alert("Can not detect Spectranet cartridge.", switch_main);
    }
    else
    {
        char mounted_path[64];
        get_mounted_path(mounted_path);

        if (strlen(mounted_path))
        {
            if (strncmp(mounted_path, "tnfs://", 7) == 0)
            {
                set_connect_to_proxy_address(mounted_path + 7);
            }
            else
            {
                set_connect_to_proxy_address(mounted_path);
            }
        }
        else
        {
            set_connect_to_proxy_address("localhost");
        }

        switch_connect_to_proxy();
    }
}

#ifndef NDEBUG
static void abort_loop()
{
}
#endif

void proto_abort()
{
    while(1) {
        zx_border(INK_RED);
#ifndef NDEBUG
        abort_loop();
#endif
        zx_border(INK_BLACK);
    }
}

int main()
{
    // never page out
    pagein();

    reset_heap();

    netlog_init(9468);
    netlog_1("Channels client.");

    zxgui_init();
    init_scenes();
    switch_main();

    while(1)
    {
        zxgui_scene_iteration();
        channels_proxy_update();
    }

    return 0;
}