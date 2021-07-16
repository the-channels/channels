#include "heap.h"

#include <stdint.h>
#include <stdlib.h>
#include "proto_asserts.h"
#include "system.h"

uint8_t heap_data[2048];

uint8_t next_allocated_blob = 0;

void reset_heap_blobs()
{
    next_allocated_blob = 0;
}

uint8_t allocate_heap_blob()
{
    proto_assert_str(next_allocated_blob < SPECTRANET_BLOBS_AMOUNT, "No free heap blobs");
    return next_allocated_blob++;
}

uint8_t* open_heap_blob(uint8_t blob_num)
{
    uint8_t page = SPECTRANET_FIRST_USER_PAGE + (blob_num / SPECTRANET_BLOB_PER_PAGE);
    uint16_t offset = (blob_num % SPECTRANET_BLOB_PER_PAGE) * SPECTRANET_BLOB_SIZE;
    pagein();
    setpageb(page);
    return (uint8_t*)0x2000 + offset;
}

void close_heap_blob()
{
    pageout();
}

void reset_heap()
{
    mallinit();
    sbrk(&heap_data, sizeof(heap_data));
}