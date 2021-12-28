#include "system.h"
#include <string.h>
#include <stdio.h>
#include "zxgui.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <locale.h>
#include <stddef.h>
#include <wchar.h>
#ifdef __linux__
    #include <arpa/inet.h>
    #include <netpacket/packet.h>
    #include <net/ethernet.h>
#else
    #include <net/if_dl.h>
#endif

#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

#endif

uint8_t terminal_width = 0;
uint8_t terminal_height = 0;
static wchar_t* character_data = NULL;
static uint8_t *color_data = NULL;
static uint8_t screen_dirty = 0;
struct termios original_term;

void bye(void)
{
    // restore cursor
    wprintf(L"\033[?25h");
    tcsetattr(0, 0, &original_term);
}

void sigint_handler(int sig)
{
    bye();
    signal(SIGINT, SIG_DFL);
    exit(0);
}

void system_init()
{
    setlocale(LC_ALL, "");

    // hide cursor
    wprintf(L"\033[?25l");
    signal(SIGINT, sigint_handler);
    atexit(bye);

    // disable echoing and make getch() nonblocking

    struct termios term;
    tcgetattr(0, &term);
    original_term = term;
    term.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(0, 0, &term);

    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    terminal_width = w.ws_col > 255 ? 255 : w.ws_col;
    terminal_height = w.ws_row > 255 ? 255 : w.ws_row;

    if (terminal_width == 0)
    {
        printf("Not running in a terminal window.\n");
        exit(1);
    }

    character_data = malloc(terminal_width * terminal_height * sizeof(wchar_t));
    color_data = malloc(terminal_width * terminal_height * sizeof(uint8_t));

    memset(color_data, COLOR_FG_WHITE | COLOR_BG_BLACK, terminal_width * terminal_height);

    clear_screen_with(0);
}

extern wchar_t* get_character_data_at(int x, int y)
{
    return character_data + y * terminal_width + x;
}

extern uint8_t* get_color_data_at(int x, int y)
{
    return color_data + y * terminal_width + x;
}

void set_screen_dirty()
{
    screen_dirty = 1;
}

#define COLOR_FG_TERM_BLACK      (30)
#define COLOR_FG_TERM_BLUE       (34)
#define COLOR_FG_TERM_RED        (31)
#define COLOR_FG_TERM_MAGENTA    (35)
#define COLOR_FG_TERM_GREEN      (32)
#define COLOR_FG_TERM_CYAN       (36)
#define COLOR_FG_TERM_YELLOW     (33)
#define COLOR_FG_TERM_WHITE      (37)

#define COLOR_BRIGHT        (1 << 7)

#define COLOR_BG_TERM_BLACK      (40)
#define COLOR_BG_TERM_BLUE       (44)
#define COLOR_BG_TERM_RED        (41)
#define COLOR_BG_TERM_MAGENTA    (45)
#define COLOR_BG_TERM_GREEN      (42)
#define COLOR_BG_TERM_CYAN       (46)
#define COLOR_BG_TERM_YELLOW     (43)
#define COLOR_BG_TERM_WHITE      (47)

uint8_t color_to_bg_term(uint8_t c)
{
    switch (c)
    {
        case COLOR_BG_BLACK: return COLOR_BG_TERM_BLACK;
        case COLOR_BG_BLUE: return COLOR_BG_TERM_BLUE;
        case COLOR_BG_RED: return COLOR_BG_TERM_RED;
        case COLOR_BG_MAGENTA: return COLOR_BG_TERM_MAGENTA;
        case COLOR_BG_GREEN: return COLOR_BG_TERM_GREEN;
        case COLOR_BG_CYAN: return COLOR_BG_TERM_CYAN;
        case COLOR_BG_YELLOW: return COLOR_BG_TERM_YELLOW;
        default: return COLOR_BG_TERM_WHITE;
    }
}

uint8_t color_to_fg_term(uint8_t c)
{
    switch (c)
    {
        case COLOR_FG_BLACK: return COLOR_FG_TERM_BLACK;
        case COLOR_FG_BLUE: return COLOR_FG_TERM_BLUE;
        case COLOR_FG_RED: return COLOR_FG_TERM_RED;
        case COLOR_FG_MAGENTA: return COLOR_FG_TERM_MAGENTA;
        case COLOR_FG_GREEN: return COLOR_FG_TERM_GREEN;
        case COLOR_FG_CYAN: return COLOR_FG_TERM_CYAN;
        case COLOR_FG_YELLOW: return COLOR_FG_TERM_YELLOW;
        default: return COLOR_FG_TERM_WHITE;
    }
}

void system_object_invalidated()
{
    set_screen_dirty();
}

void refresh_screen()
{
    wprintf(L"\e[1;1H");
    wchar_t* c = character_data;
    uint8_t* col = color_data;
    uint8_t last_color = 0xFF;
    for (int j = 0; j < terminal_height; j++)
    {
        for (int i = 0; i < terminal_width; i++)
        {
            uint8_t color = *col++;
            if (color != last_color)
            {
                uint8_t fg = color_to_fg_term(color & 0b00000111);
                uint8_t bg = color_to_bg_term(color & 0b01110000);

                wprintf(L"\033[%d;%dm", fg, bg);
                last_color = color;
            }

            wprintf(L"%lc", *c++);
        }
    }
    fflush(stdout);
}

void system_update()
{
    if (screen_dirty)
    {
        screen_dirty = 0;
        refresh_screen();
    }
}

void clear_screen_with(uint8_t color)
{
    wmemset(character_data, L' ', terminal_width * terminal_height);
    memset(color_data, COLOR_BG_BLACK | COLOR_FG_BLACK, terminal_width * terminal_height);
    screen_dirty = 1;
}

void set_border_color(uint8_t color)
{

}

uint8_t test_network_capabilities()
{
    return 0;
}

static int macaddr(char *ifname, char *macaddrstr) {
    struct ifaddrs *ifap, *ifaptr;
    unsigned char *ptr;

    if (getifaddrs(&ifap) == 0) {
        for(ifaptr = ifap; ifaptr != NULL; ifaptr = (ifaptr)->ifa_next) {
#ifdef __linux__
            if (!strcmp((ifaptr)->ifa_name, ifname) && (((ifaptr)->ifa_addr)->sa_family == AF_PACKET)) {
                struct sockaddr_ll *s = (struct sockaddr_ll*)(ifaptr->ifa_addr);
                memcpy(macaddrstr, s->sll_addr, 6);
                break;
            }
#else
            if (!strcmp((ifaptr)->ifa_name, ifname) && (((ifaptr)->ifa_addr)->sa_family == AF_LINK)) {
                ptr = (unsigned char *)LLADDR((struct sockaddr_dl *)(ifaptr)->ifa_addr);
                memcpy(macaddrstr, ptr, 6);
                break;
            }
#endif
        }
        freeifaddrs(ifap);
        return ifaptr != NULL;
    } else {
        return 0;
    }
}

void get_device_unique_key(char* to)
{
    macaddr("lo0", to);
}