#include "zxgui.h"
#include "scenes.h"
#include <zxgui.h>
#include "system.h"
#include "channels.h"
#include "proto_asserts.h"
#include "netlog.h"
#include "heap.h"
#include "alert.h"

long heap = 0;
uint8_t fake_CRT_FONT_64 = 0;

void init_scenes()
{
    init_progress();
}

void switch_main()
{
    int detected = test_network_capabilities();
    set_border_color(COLOR_FG_BLACK);
    if (detected)
    {
        struct channels_alert_buf_t* a = alloc_heap(sizeof(struct channels_alert_buf_t));
        switch_alert(a, "Can not detect Spectranet cartridge.", NULL);
    }
    else
    {
        char mounted_path[64];
        get_default_connect_address(mounted_path);

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
            set_connect_to_proxy_address("127.0.0.1");
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
        set_border_color(COLOR_FG_RED);
#ifndef NDEBUG
        abort_loop();
#endif
        set_border_color(COLOR_FG_BLACK);
    }
}

int main()
{
    system_init();
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
#ifdef HAS_SYSTEM_UPDATE_CB
        system_update();
#endif
    }

    return 0;
}