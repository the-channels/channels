#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __SPECTRUM
#include <input.h>
#include <spectranet.h>
#include <sockpoll.h>
#include <netdb.h>
#include <sys/socket.h>

#include <spectrum.h>

#define COLOR_FG_BLACK      INK_BLACK
#define COLOR_FG_BLUE       INK_BLUE
#define COLOR_FG_RED        INK_RED
#define COLOR_FG_MAGENTA    INK_MAGENTA
#define COLOR_FG_GREEN      INK_GREEN
#define COLOR_FG_CYAN       INK_CYAN
#define COLOR_FG_YELLOW     INK_YELLOW
#define COLOR_FG_WHITE      INK_WHITE

#define COLOR_BRIGHT        BRIGHT

#define COLOR_BG_BLACK      PAPER_BLACK
#define COLOR_BG_BLUE       PAPER_BLUE
#define COLOR_BG_RED        PAPER_RED
#define COLOR_BG_MAGENTA    PAPER_MAGENTA
#define COLOR_BG_GREEN      PAPER_GREEN
#define COLOR_BG_CYAN       PAPER_CYAN
#define COLOR_BG_YELLOW     PAPER_YELLOW
#define COLOR_BG_WHITE      PAPER_WHITE

#define set_border_color    zx_border

extern void _render_blink_even(uint8_t *addr) __z88dk_fastcall __naked;
extern void _render_blink_odd(uint8_t *addr) __z88dk_fastcall __naked;
extern void clear_blink(struct gui_edit_t* this) __z88dk_fastcall;

#define int_to_string(i, s) itoa(i, s, 10)

#define HEAP_SIZE (2200)
#define ANIMATION_SPEED (128)
#define CHARACTERS_PER_CELL (2)
#define STATIC_SCREEN_SIZE (1)
#define TWO_CHARACTERS_FIT_IN (1)
#define SCREEN_WIDTH        (32)
#define SCREEN_HEIGHT       (24)
#define BLINK_INTERVAL      (100)

#else
#ifdef __DESKTOP

#define int_to_string(i, s) sprintf(s, "%d", i)

#define COLOR_FG_BLACK      (0x00)
#define COLOR_FG_BLUE       (0x01)
#define COLOR_FG_RED        (0x02)
#define COLOR_FG_MAGENTA    (0x03)
#define COLOR_FG_GREEN      (0x04)
#define COLOR_FG_CYAN       (0x05)
#define COLOR_FG_YELLOW     (0x06)
#define COLOR_FG_WHITE      (0x07)

#define COLOR_BRIGHT        (1 << 7)

#define COLOR_BG_BLACK      (0x00)
#define COLOR_BG_BLUE       (0x10)
#define COLOR_BG_RED        (0x20)
#define COLOR_BG_MAGENTA    (0x30)
#define COLOR_BG_GREEN      (0x40)
#define COLOR_BG_CYAN       (0x50)
#define COLOR_BG_YELLOW     (0x60)
#define COLOR_BG_WHITE      (0x70)


extern void set_border_color(uint8_t color);
extern void *memrchr (void const *s, int c_in, size_t n);

extern uint8_t terminal_width;
extern uint8_t terminal_height;

#define HEAP_SIZE           (64000)
#define CHARACTERS_PER_CELL (1)
#define TWO_CHARACTERS_FIT_IN (2)
#define SCREEN_WIDTH        (terminal_width)
#define SCREEN_HEIGHT       (terminal_height)
#define WIDE_SCREEN         (1)
#define BLINK_INTERVAL      (150)
#define HAS_SYS_TIME        (1)
#define ANIMATION_SPEED     (200)
#define HAS_SYSTEM_OBJECT_INVALIDATED_CB (1)
#define HAS_SYSTEM_UPDATE_CB (1)

#else
#error "Unknown platform"
#endif
#endif

struct proto_process_t;

extern void system_init();
extern uint8_t test_network_capabilities();
extern void clear_screen_with(uint8_t color);
extern void allocate_proto_process_struct();
extern struct proto_process_t* get_proto_process_struct();
extern void get_device_unique_key(char* to);
extern void get_default_connect_address(char* into);

#ifdef HAS_SYSTEM_OBJECT_INVALIDATED_CB
extern void system_object_invalidated();
#endif

#ifdef HAS_SYSTEM_UPDATE_CB
extern void system_update();
#endif

#endif