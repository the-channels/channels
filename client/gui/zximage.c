#include <string.h>
#include "zxgui.h"
#include "system.h"

extern void _dynamic_image_render(uint8_t x, uint8_t y, struct gui_dynamic_image_t* this, struct gui_scene_t* scene);

void _image_render(uint8_t x, uint8_t y, struct gui_image_t* this, struct gui_scene_t* scene)
{
    x += this->x;
    y += this->y;

    if (is_object_invalidated(this))
    {
        zxgui_image(x, y, this->w, this->h, this->source, this->colors);
    }
}

void zxgui_image_init(struct gui_image_t* image, xywh_t xywh,
    const uint8_t* source, const uint8_t* colors) ZXGUI_CDECL
{
    zxgui_base_init(image, xywh, _image_render, NULL);

    image->flags = GUI_FLAG_DIRTY;
    image->source = source;
    image->colors = colors;
}

void zxgui_dynamic_image_init(struct gui_dynamic_image_t* image, xywh_t xywh,
    gui_label_obtain_image_f obtain_data, void* user) ZXGUI_CDECL
{
    zxgui_base_init(image, xywh, _dynamic_image_render, NULL);

    image->flags = GUI_FLAG_DIRTY;
    image->obtain_data = obtain_data;
    image->user = user;
}