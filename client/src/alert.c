#include "zxgui.h"
#include "zxgui_internal.h"
#include "scenes.h"
#include "system.h"
#include <string.h>

static struct gui_scene_t scene;
static struct gui_form_t alert;
static void (*back_callback)();

static char alert_message[128] = {};

void return_to_callback(struct gui_button_t* this)
{
    back_callback();
}

void init_alert()
{
    zxgui_scene_init(&scene, NULL);

    zxgui_form_init(&alert, 0, 10, 31, 4, alert_message, FORM_STYLE_DEFAULT);

    {
        static struct gui_button_t button_cancel;
        zxgui_button_init(&button_cancel, 0, 1, 8, 1, 13, GUI_ICON_RETURN, "OK", return_to_callback);
        zxgui_form_add_child(&alert, &button_cancel);
    }

    zxgui_scene_add(&scene, &alert);
}

void switch_alert(const char* message, void (*callback)())
{
    strcpy(alert_message, message);
    back_callback = callback;
    zxgui_scene_set(&scene);
}