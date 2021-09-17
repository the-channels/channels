#include <string.h>
#include "zxgui.h"

static void _icon_render(uint8_t x, uint8_t y, struct gui_animated_icon_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (this->flags & GUI_FLAG_HIDDEN)
    {
        if (is_object_invalidated(this))
        {
            zxgui_screen_clear(x, y, this->w, this->h);
        }

        return;
    }

    if (is_object_invalidated(this) || this->time++ < this->speed)
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

void zxgui_animated_icon_init(struct gui_animated_icon_t* icon, xywh_t xywh,
    uint8_t frames, uint8_t color, const uint8_t* source, uint8_t speed) ZXGUI_CDECL
{
    zxgui_base_init(icon, xywh, _icon_render, NULL);

    icon->flags = GUI_FLAG_DIRTY;
    icon->color = color;
    icon->frames = frames;
    icon->current_frame = 0;
    icon->time = 0;
    icon->speed = speed;
    icon->source = source;
}