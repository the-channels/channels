#ifndef CHANNELS_ALERT_H
#define CHANNELS_ALERT_H

#include "system.h"
#include "zxgui.h"

struct channels_alert_buf_t
{
    struct gui_scene_t scene;
    struct gui_form_t alert;
    struct gui_button_t button_cancel;
    char alert_message[128];
    void (*back_callback)();
};

extern void switch_alert(struct channels_alert_buf_t* a, const char* message, void (*callback)());

#endif
