#include "zxgui.h"

typedef uint8_t uchar;

#include <stdint.h>
#include <string.h>
#include <spectrum.h>
#include <fzx_ui.h>
#include "zxgui_tiles.h"

void zxgui_init(void)
{
    fzx_ui_init();

    zx_border(INK_BLACK);
    zx_colour(BRIGHT | INK_GREEN | PAPER_BLACK);
    zx_cls();

    fzx_ui_set_paper(0, 0, 256, 192);
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

void zxgui_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source, const uint8_t* colors) __z88dk_callee
{
    for (uint8_t j = y; j < y + h; j++)
    {
        for (uint8_t i = x; i < x + w; i++)
        {
            uint8_t* addr = zx_cxy2saddr(i, j);

            for (uint8_t ii = 0; ii < 8; ii++)
            {
                *addr = *source++;
                addr += 256;
            }

            *zx_cxy2aaddr(i, j) = *colors++;
        }
    }
}

void zxgui_line(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t c) __z88dk_callee
{
    uint8_t* data = get_gui_tiles() + c * 8;
    uint8_t* addr = zx_cxy2saddr(x, y);

    for (uint8_t i = 0; i < 8; i++)
    {
        memset(addr, *data++, w);
        addr += 256;
    }

    memset(zx_cxy2aaddr(x, y), form_color, w);
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
}