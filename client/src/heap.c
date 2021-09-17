#include "heap.h"

#include <stdint.h>
#include "proto_asserts.h"
#include "system.h"
#include "netlog.h"

uint8_t heap_data[HEAP_SIZE];
uint8_t* heap_ptr = heap_data;

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

uint8_t* open_heap_blob(uint8_t blob_num) __z88dk_fastcall
{
    uint8_t page = SPECTRANET_FIRST_USER_PAGE + (blob_num / SPECTRANET_BLOB_PER_PAGE);
    uint16_t offset = (blob_num % SPECTRANET_BLOB_PER_PAGE) * SPECTRANET_BLOB_SIZE;
    setpagea(page);
    return (uint8_t*)PAGE_A_PTR + offset;
}

void* alloc_heap(uint16_t size) __z88dk_fastcall
{
    if (heap_ptr - heap_data + size > HEAP_SIZE)
    {
        return NULL;
    }
    uint8_t* res = heap_ptr;
    memset(res, 0, size);
    heap_ptr += size;

    return res;
}

void reset_heap()
{
    heap_ptr = heap_data;
}