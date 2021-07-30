#ifndef ZXGUI_INTERNAL_H
#define ZXGUI_INTERNAL_H

#include "zxgui.h"

extern struct fzx_state font_state;

extern void zxgui_reset_paper();
extern void zxgui_rectangle(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t c);
extern void zxgui_line(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t c);
extern void zxgui_icon(uint8_t form_color, uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source);
extern void zxgui_image(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* source, const uint8_t* colors);
extern struct gui_object_t* zxgui_get_last_object(struct gui_object_t* object);
extern uint8_t is_object_invalidated(void* object);
extern void object_invalidate(void* object, uint8_t flag);

#endif