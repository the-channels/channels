#include "system.h"
#include "zxgui.h"
#include "zxgui_internal.h"
#include "scenes.h"

static struct gui_scene_t scene;
static struct gui_form_t connecting;
static struct gui_button_t button_cancel;
static struct gui_animated_icon_t loading;
static void (*cancel_progress_callback)();

void cancel_connection(struct gui_button_t* this)
{
    if (cancel_progress_callback)
    {
        cancel_progress_callback();
    }
}

void init_progress()
{
    zxgui_scene_init(&scene, NULL);

    zxgui_form_init(&connecting, 8, 8, 15, 7, "", FORM_STYLE_EMPTY);

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
        zxgui_animated_icon_init(&loading, 6, 1, 2, 2, 2, INK_GREEN | BRIGHT | PAPER_BLACK, frames, 128);
        zxgui_form_add_child(&connecting, &loading);
    }

    {
        zxgui_button_init(&button_cancel, 0, 4, 8, 1, 'c', GUI_ICON_C, "CANCEL", cancel_connection);
        zxgui_form_add_child(&connecting, &button_cancel);
    }

    zxgui_scene_add(&scene, &connecting);
}

void switch_progress(const char* progress_message, void (*cancel)(void))
{
    loading.time = 254;
    cancel_progress_callback = cancel;
    if (cancel)
    {
        button_cancel.flags &= ~GUI_FLAG_HIDDEN;
    }
    else
    {
        button_cancel.flags |= GUI_FLAG_HIDDEN;
    }
    connecting.title = progress_message;
    zxgui_scene_set(&scene);
}