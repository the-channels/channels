
#include <stdint.h>
typedef uint8_t uchar;

uint8_t* get_gui_tiles()
{
#   include "tiles.h"
    return tiles;
}