#ifndef CHANNEL_HUB_POST_H
#define CHANNEL_HUB_POST_H

#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include "proto_objects.h"

typedef std::string PostId;

struct Attachment
{
    std::string url;

    explicit Attachment(const std::string& url) :
        url(url) {}
};

struct Post
{
    PostId id;
    std::string title;
    std::string comment;
    std::vector<Attachment> attachments;
    std::vector<PostId> replies;
    time_t date;

    ChannelObject* write() const;
};

#endif
