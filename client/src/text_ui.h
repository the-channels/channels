#ifndef FZX_UI_HEADER
#define FZX_UI_HEADER

#include <stdint.h>

extern void text_ui_init(void);
extern void text_ui_puts_at(uint8_t x, uint8_t y, const char* s);
extern void text_ui_write_at(uint8_t x, uint8_t y, const char* buf, uint8_t buflen);
extern char* text_ui_buffer_partition(char *buf, uint16_t buflen, uint8_t allowed_width);
extern void text_ui_color(uint8_t color) __z88dk_fastcall;

#endif