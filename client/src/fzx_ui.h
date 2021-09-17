#ifndef FZX_UI_HEADER
#define FZX_UI_HEADER

#include <stdint.h>

extern void fzx_ui_init(void);
extern void fzx_ui_at(uint8_t x, uint8_t y);
extern void fzx_ui_puts(const char* s);
extern void fzx_ui_puts_at(uint8_t x, uint8_t y, const char* s);
extern void fzx_ui_write_at(uint8_t x, uint8_t y, const char* buf, uint16_t buflen) __z88dk_callee;
extern char* fzx_ui_buffer_partition_ww(char *buf, uint16_t buflen, uint16_t allowed_width);
extern char* fzx_ui_buffer_partition(char *buf, uint16_t buflen, uint16_t allowed_width);
extern uint16_t fzx_ui_string_extent(const char *s);
extern void fzx_ui_switch_xor(void);
extern void fzx_ui_switch_or(void);
extern void fzx_ui_set_paper(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
extern void fzx_ui_color(uint8_t color);

#endif