#include <string.h>
#include "zxgui.h"
#include "zxgui_internal.h"

static void image_render(uint8_t x, uint8_t y, struct gui_image_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (is_object_invalidated(this))
    {
        zxgui_image(x, y, this->w, this->h, this->source, this->colors);
    }
}

void zxgui_image_init(struct gui_image_t* image, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source, const uint8_t* colors)
{
    image->render = (gui_render_f)image_render;
    image->event = NULL;
    image->flags = GUI_FLAG_DIRTY;
    image->next = NULL;
    image->source = source;
    image->colors = colors;
    image->x = x;
    image->y = y;
    image->w = w;
    image->h = h;
}