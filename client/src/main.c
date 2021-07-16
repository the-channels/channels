#include "zxgui.h"
#include "scenes.h"
#include <spectranet.h>
#include <malloc.h>
#include <font/fzx.h>
#include <zxgui_internal.h>
#include "system.h"
#include "channels.h"
#include "proto_asserts.h"
#include "netlog.h"
#include "heap.h"

long heap = 0;
uint8_t fake_CRT_FONT_64 = 0;

void init_scenes()
{
    init_connect_to_proxy();
    init_progress();
    init_alert();
    init_select_channel();
    init_select_board();
    init_thread_view();
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
        switch_connect_to_proxy();
    }
}

int main()
{
    reset_heap();

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