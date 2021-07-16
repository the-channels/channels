#ifndef CHANNELS_HEAP
#define CHANNELS_HEAP (1)

#include <stdint.h>

#define SPECTRANET_FIRST_USER_PAGE (0xC3)
#define SPECTRANET_PAGES_AMOUNT (28)
#define SPECTRANET_BLOB_SIZE (0x0400)
#define SPECTRANET_PAGE_SIZE (0x1000)
#define SPECTRANET_BLOB_PER_PAGE (SPECTRANET_PAGE_SIZE / SPECTRANET_BLOB_SIZE)
#define SPECTRANET_BLOBS_AMOUNT (SPECTRANET_BLOB_PER_PAGE * SPECTRANET_PAGES_AMOUNT)

extern void reset_heap_blobs();
extern uint8_t allocate_heap_blob();
extern uint8_t* open_heap_blob(uint8_t blob_num);
extern void close_heap_blob();

extern void reset_heap();

#endif