#include "zxgui.h"
#include "system.h"

enum input_state_t
{
    waiting,
    key_pressed,
    key_pressed_w_sym,
};

static enum input_state_t input_state = waiting;
static char last_key_pressed = 0;
extern struct gui_scene_t* current_scene;

uint8_t is_alt_key_pressed()
{
    return in_KeyPressed(0x4000);
}

static void process_last_key()
{
    struct gui_event_key_pressed e;
    e.key = last_key_pressed;
    if (zxgui_scene_dispatch_event(current_scene, GUI_EVENT_KEY_PRESSED, &e) == 0)
    {
        if (current_scene->key_pressed)
        {
            current_scene->key_pressed(last_key_pressed);
        }
    }
}

void update_keyboard()
{
    switch (input_state)
    {
        case waiting:
        {
            last_key_pressed = (char)in_Inkey();
            if (last_key_pressed)
            {
                process_last_key();

                if (in_KeyPressed(0x4000) == 0 && in_KeyPressed(0x2000) == 0)
                {
                    input_state = key_pressed;
                }
                else
                {
                    input_state = key_pressed_w_sym;
                }
            }
            break;
        }
        case key_pressed_w_sym:
        {
            if (0 == (char)in_Inkey())
            {
                input_state = waiting;
            }
            break;
        }
        case key_pressed:
        {
            char key = (char)in_Inkey();
            if (key != last_key_pressed)
            {
                input_state = waiting;
            }
            break;
        }
    }
}