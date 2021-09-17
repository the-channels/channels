#ifndef CHANNELS_HEAP
#define CHANNELS_HEAP (1)

#include <stdint.h>
#include "memory_layout.h"

extern void* alloc_heap(uint16_t size) __z88dk_fastcall;

extern void reset_heap_blobs();
extern uint8_t allocate_heap_blob();
extern uint8_t* open_heap_blob(uint8_t blob_num) __z88dk_fastcall;

extern void reset_heap();

#endif