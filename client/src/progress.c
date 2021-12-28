#include "system.h"
#include "zxgui.h"
#include "scenes.h"

static struct gui_scene_t scene;
static struct gui_form_t connecting;
static struct gui_animated_icon_t loading;

void init_progress()
{
    zxgui_scene_init(&scene, NULL);

    zxgui_form_init(&connecting, XYWH((SCREEN_WIDTH / 2) - 7, (SCREEN_HEIGHT / 2) - 4, 15, 7), "", FORM_STYLE_EMPTY);

    {
        static uint8_t frames[] = {
            GUI_ICON_LOADING_A_1,
            GUI_ICON_LOADING_A_2,
            GUI_ICON_LOADING_A_3,
            GUI_ICON_LOADING_A_4,

            GUI_ICON_LOADING_B_1,
            GUI_ICON_LOADING_B_2,
            GUI_ICON_LOADING_B_3,
            GUI_ICON_LOADING_B_4,
        };
        zxgui_animated_icon_init(&loading, XYWH(6, 1, 2, 2), 2, COLOR_FG_GREEN | COLOR_BRIGHT | COLOR_BG_BLACK, frames, ANIMATION_SPEED);

        zxgui_form_add_child(&connecting, &loading);
    }

    zxgui_scene_add(&scene, &connecting);
}

void switch_progress(const char* progress_message)
{
    loading.time = 254;
    connecting.title = progress_message;
    zxgui_scene_set(&scene);
}