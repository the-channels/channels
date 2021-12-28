#include <stdio.h>
#include <unistd.h>
#include <wchar.h>
#include "zxgui.h"

#include <linux/input.h>

extern struct gui_scene_t* current_scene;

uint8_t is_alt_key_pressed()
{
    return 0;
}

void update_keyboard()
{
    int c = getchar();

    if (c <= 0)
    {
        return;
    }

    switch (c)
    {
        case 0:
        {
            break;
        }
        case 127:
        {
            c = GUI_KEY_CODE_BACKSPACE;
            break;
        }
        case 10:
        {
            c = GUI_KEY_CODE_RETURN;
            break;
        }
        case 27:
        {
            if ((c = getchar()) <= 0)
            {
                c = GUI_KEY_CODE_ESCAPE;
                break;
            }

            switch (c)
            {
                case 91:
                {
                    if ((c = getchar()) <= 0)
                    {
                        return;
                    }

                    switch (c)
                    {
                        case 66:
                        {
                            c = GUI_KEY_CODE_DOWN;
                            break;
                        }
                        case 65:
                        {
                            c = GUI_KEY_CODE_UP;
                            break;
                        }
                        default:
                        {
                            return;
                        }
                    }

                    break;
                }
                default:
                {
                    return;
                }
            }

            break;
        }
        default:
        {
            break;
        }
    }

    struct gui_event_key_pressed e;
    e.key = (char)c;

    if (zxgui_scene_dispatch_event(current_scene, GUI_EVENT_KEY_PRESSED, &e) == 0)
    {
        if (current_scene->key_pressed)
        {
            current_scene->key_pressed((char)c);
        }
    }
}