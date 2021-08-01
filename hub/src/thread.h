#ifndef CHANNEL_HUB_THREAD_H
#define CHANNEL_HUB_THREAD_H

#include "post.h"

typedef PostId ThreadId;

struct Thread: public Post
{
    uint16_t num_replies = 0;

    ChannelObject* write() const
    {
        std::string stripped_comment = comment;
        if (stripped_comment.length() > 1024)
        {
            stripped_comment = stripped_comment.substr(0, 1021) + "...";
        }

        declare_str_property_on_stack(id_, OBJ_PROPERTY_ID, id.c_str(), nullptr);
        declare_str_property_on_stack(title_, OBJ_PROPERTY_TITLE, title.c_str(), &id_);
        declare_str_property_on_stack(comment_, OBJ_PROPERTY_COMMENT, stripped_comment.c_str(), &title_);
        declare_arg_property_on_stack(attachment_width_, 'w', attachment_width, &comment_);
        declare_arg_property_on_stack(attachment_height_, 'h', attachment_height, &attachment_width_);
        declare_arg_property_on_stack(replies_, 'r', num_replies, &attachment_height_);

        return channel_object_allocate(&replies_);
    }
};

#endif
