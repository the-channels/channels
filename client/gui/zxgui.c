#include "zxgui.h"
#include "zxgui_internal.h"

typedef uint8_t uchar;

#include <stdint.h>
#include <font/fzx.h>
#include "system.h"

struct fzx_state font_state;
struct r_Rect16 screen = { 0, 256, 0, 192 };

void zxgui_init()
{
    zx_border(INK_BLACK);
    zx_colour(BRIGHT | INK_GREEN | PAPER_BLACK);
    zx_cls();

    fzx_state_init(&font_state, &ff_utz_TinyTexanS, &screen);

    font_state.fgnd_attr = BRIGHT | INK_GREEN | PAPER_BLACK;
    font_state.fgnd_mask = 0;
    font_state.fzx_draw = _fzx_draw_or;

    font_state.y = 32;
    font_state.x = 0;

    zxgui_reset_paper();
}

void zxgui_reset_paper()
{
    font_state.paper.x = 0;
    font_state.paper.y = 0;
    font_state.paper.width = 256;
    font_state.paper.height = 192;
}

void zxgui_icon(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source)
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

void zxgui_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source, const uint8_t* colors)
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

void zxgui_rectangle(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t c)
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
    uint8_t i = x + 1;

    while (i < xw)
    {
        zxgui_screen_put(i, y, top);
        zxgui_screen_put(i, yh, bottom);
        i++;
    }

    i = y + 1;

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