#include "channel.h"
#include "proto_objects.h"

ChannelObject* Channel::write() const
{
    declare_str_property_on_stack(id_, OBJ_PROPERTY_ID, get_name().c_str(), nullptr);
    declare_str_property_on_stack(title_, OBJ_PROPERTY_TITLE, get_title().c_str(), &id_);

    return channel_object_allocate(&title_);
}

std::string Channel::cache_key(const std::string& key)
{
    return "cache/" + m_name + "_" + key;
}
