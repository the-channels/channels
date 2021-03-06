#include "zxgui.h"
#include "system.h"

typedef uint8_t uchar;

#include <stdint.h>
#include <string.h>
#include "system.h"
#include <text_ui.h>

uint8_t screen_color = 0;

void zxgui_init(void)
{
    text_ui_init();
    set_border_color(COLOR_FG_BLACK);
    clear_screen_with(COLOR_BRIGHT | COLOR_FG_GREEN | COLOR_BG_BLACK);
}

xywh_t get_xywh(uint8_t x, uint8_t y, uint8_t w, uint8_t h) __z88dk_callee
{
    return (xywh_t)x | ((xywh_t)y << 8) | ((xywh_t)w << 16) | ((xywh_t)h << 24);
}

void zxgui_base_init(void* o, xywh_t xywh, void* render, void* event) ZXGUI_CDECL
{
    struct gui_object_t* s = (struct gui_object_t*)o;

    s->x = (uint8_t)(xywh);
    s->y = (uint8_t)(xywh >> 8);
    s->w = (uint8_t)(xywh >> 16);
    s->h = (uint8_t)(xywh >> 24);

    s->next = NULL;
    s->render = render;
    s->event = event;
}

void zxgui_icon(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source) __z88dk_callee
{
    zxgui_screen_color(form_color);
    for (uint8_t j = y; j < y + h; j++)
    {
        for (uint8_t i = x; i < x + w; i++)
        {
            zxgui_screen_put(i, j, *source++);
        }
    }
}

void zxgui_rectangle(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t c) __z88dk_callee
{
    zxgui_screen_color(form_color);

    uint8_t yh = y + h;
    uint8_t xw = x + w;

    zxgui_screen_put(x, yh, c++);
    zxgui_screen_put(xw, yh, c++);
    zxgui_screen_put(xw, y, c++);
    zxgui_screen_put(x, y, c++);

    uint8_t top = c++;
    uint8_t left = c++;
    uint8_t bottom = c++;
    uint8_t right = c;

    zxgui_line(form_color, x + 1, y, w - 1, top);
    zxgui_line(form_color, x + 1, yh, w - 1, bottom);

    uint8_t i = y + 1;

    while (i < yh)
    {
        zxgui_screen_put(x, i, left);
        zxgui_screen_put(xw, i, right);
        i++;
    }
}

struct gui_object_t* zxgui_get_last_object(struct gui_object_t* object)
{
    struct gui_object_t* c = object->next;
    if (c == NULL)
    {
        return object;
    }
    while (c->next)
    {
        c = c->next;
    }
    return c;
}

uint8_t is_object_invalidated(void* object)
{
    struct gui_object_t* c = object;
    uint8_t f = c->flags & (GUI_FLAG_DIRTY | GUI_FLAG_DIRTY_INTERNAL);
    if (f)
    {
        c->flags &= ~(GUI_FLAG_DIRTY | GUI_FLAG_DIRTY_INTERNAL);
        return f;
    }
    return 0;
}

void object_invalidate(void* object, uint8_t flag)
{
    struct gui_object_t* c = object;
    c->flags |= flag;
#ifdef HAS_SYSTEM_OBJECT_INVALIDATED_CB
    system_object_invalidated();
#endif
}