#ifndef CHANNEL_HUB_THREAD_H
#define CHANNEL_HUB_THREAD_H

#include "post.h"

typedef PostId ThreadId;

struct Thread: public Post
{
    uint16_t num_replies = 0;

    ChannelObject* write() const;
};

#endif
