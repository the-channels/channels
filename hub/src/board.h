#ifndef CHANNEL_HUB_BOARD_H
#define CHANNEL_HUB_BOARD_H

#include <string>
#include "proto_objects.h"

typedef std::string BoardId;

struct Board
{
    BoardId id;
    std::string title;
    std::string description;

    ChannelObject* write() const
    {
        declare_str_property_on_stack(id_, OBJ_PROPERTY_ID, id.c_str(), nullptr);
        declare_str_property_on_stack(title_, OBJ_PROPERTY_TITLE, title.c_str(), &id_);
        declare_str_property_on_stack(description_, OBJ_PROPERTY_COMMENT, description.c_str(), &title_);

        return channel_object_allocate(&description_);
    }
};

#endif
