#include "zxgui.h"
#include "scenes.h"
#include "system.h"
#include <string.h>
#include "alert.h"

static struct channels_alert_buf_t* alert = NULL;

void return_to_callback(struct gui_button_t* this)
{
    if (alert->back_callback)
        alert->back_callback();
}

static void init_alert(struct channels_alert_buf_t* a)
{
    memset(a, 0, sizeof(struct channels_alert_buf_t));

    alert = a;

    zxgui_scene_init(&alert->scene, NULL);

    zxgui_form_init(&alert->alert,
#ifdef STATIC_SCREEN_SIZE
        XYWH(0, 10, 31, 4),
#else
        XYWH(SCREEN_WIDTH > 64 ? ((SCREEN_WIDTH / 2) - 24) : 8 , (SCREEN_HEIGHT / 2) - 2, SCREEN_WIDTH > 64 ? 48 : SCREEN_WIDTH - 16, 4),
#endif
        alert->alert_message, FORM_STYLE_DEFAULT);

    {
        zxgui_button_init(&alert->button_cancel, XYWH(0, 1, 8, 1), 13, GUI_ICON_RETURN, "OK", return_to_callback);
        zxgui_form_add_child(&alert->alert, &alert->button_cancel);
    }

    zxgui_scene_add(&alert->scene, &alert->alert);
}

void switch_alert(struct channels_alert_buf_t* a, const char* message, void (*callback)())
{
    init_alert(a);

    strcpy(alert->alert_message, message);
    alert->back_callback = callback;
    zxgui_scene_set(&alert->scene);
}