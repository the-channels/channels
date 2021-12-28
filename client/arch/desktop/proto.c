
#include "channels_proto.h"
#include "system.h"

static struct proto_process_t proto;

void allocate_proto_process_struct()
{
    memset(&proto, 0, sizeof(struct proto_process_t));
}

struct proto_process_t* get_proto_process_struct()
{
    return &proto;
}