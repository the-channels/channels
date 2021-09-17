#ifndef CHANNELS_MEMORY_LAYOUT
#define CHANNELS_MEMORY_LAYOUT (1)

#define HEAP_SIZE (2200)

#define PAGE_A_PTR (0x1000)
#define PAGE_B_PTR (0x2000)

#define SPECTRANET_PROTO_PAGE (0xC0)
#define SPECTRANET_FIRST_USER_PAGE (0xC4)
#define SPECTRANET_PAGES_AMOUNT (28)
#define SPECTRANET_BLOB_SIZE (0x0400)
#define SPECTRANET_PAGE_SIZE (0x1000)
#define SPECTRANET_BLOB_PER_PAGE (SPECTRANET_PAGE_SIZE / SPECTRANET_BLOB_SIZE)
#define SPECTRANET_BLOBS_AMOUNT (SPECTRANET_BLOB_PER_PAGE * SPECTRANET_PAGES_AMOUNT)

#endif