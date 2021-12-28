#include "system.h"
#include "string.h"
#include "channels_proto.h"

#define SPECTRANET_PROTO_PAGE (0xC0)

void allocate_proto_process_struct()
{
    setpageb(SPECTRANET_PROTO_PAGE);
    struct proto_process_t* proto = (struct proto_process_t*)0x2000;
    memset(proto, 0, sizeof(struct proto_process_t));
}

struct proto_process_t* get_proto_process_struct()
{
    setpageb(SPECTRANET_PROTO_PAGE);
    return (struct proto_process_t*)0x2000;
}

void get_device_unique_key(char* to)
{
    gethwaddr(to);
}