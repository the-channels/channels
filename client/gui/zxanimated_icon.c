#include <string.h>
#include "zxgui.h"
#include "zxgui_internal.h"
#include "system.h"

static void icon_render(uint8_t x, uint8_t y, struct gui_animated_icon_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (++this->time)
    {
        return;
    }

    this->time = 0;
    this->current_frame++;
    if (this->current_frame >= this->frames)
    {
        this->current_frame = 0;
    }

    zxgui_icon(this->color, x, y, this->w, this->h,
        this->source + this->current_frame * this->w * this->h);
}

void zxgui_animated_icon_init(struct gui_animated_icon_t* icon, uint8_t x, uint8_t y, uint8_t w, uint8_t h,
    uint8_t frames, uint8_t color, const uint8_t* source)
{
    icon->render = (gui_render_f)icon_render;
    icon->event = NULL;
    icon->flags = GUI_FLAG_DIRTY;
    icon->next = NULL;
    icon->color = color;
    icon->x = x;
    icon->y = y;
    icon->w = w;
    icon->h = h;
    icon->frames = frames;
    icon->current_frame = 0;
    icon->time = 0;
    icon->source = source;
}