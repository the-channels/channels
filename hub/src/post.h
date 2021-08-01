#ifndef CHANNEL_HUB_POST_H
#define CHANNEL_HUB_POST_H

#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include "proto_objects.h"

typedef std::string PostId;

struct Post
{
    PostId id;
    std::string title;
    std::string comment;
    std::string attachment;
    uint16_t attachment_width = 0;
    uint16_t attachment_height = 0;
    std::vector<PostId> replies;
    time_t date;

    ChannelObject* write() const
    {
        std::string stripped_comment = comment;
        if (stripped_comment.length() > 1024)
        {
            stripped_comment = stripped_comment.substr(0, 1021) + "...";
        }

        uint16_t replies_count = replies.size();

        declare_str_property_on_stack(id_, OBJ_PROPERTY_ID, id.c_str(), nullptr);
        declare_str_property_on_stack(title_, OBJ_PROPERTY_TITLE, title.c_str(), &id_);
        declare_str_property_on_stack(comment_, OBJ_PROPERTY_COMMENT, stripped_comment.c_str(), &title_);
        declare_arg_property_on_stack(attachment_width_, 'w', attachment_width, &comment_);
        declare_arg_property_on_stack(attachment_height_, 'h', attachment_height, &attachment_width_);
        declare_arg_property_on_stack(replies_, 'r', replies_count, &attachment_height_);

        return channel_object_allocate(&replies_);
    }
};

#endif
