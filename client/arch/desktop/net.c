#include <string.h>
#include <stdlib.h>

void get_default_connect_address(char* into)
{
    char* h = getenv("CHANNELS_HOST");
    strcpy(into, h ? h : "127.0.0.1");
}
