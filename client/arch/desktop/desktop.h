#ifndef DESKTOP_H
#define DESKTOP_H

#include <stddef.h>

enum gui_tiles;

extern void set_screen_dirty();
extern wchar_t* get_character_data_at(int x, int y);
extern uint8_t* get_color_data_at(int x, int y);
extern wchar_t tile_to_char(enum gui_tiles t);

#endif