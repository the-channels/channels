#ifndef CHANNEL_SETTING_DEF_H
#define CHANNEL_SETTING_DEF_H

#include <string>
#include "proto_objects.h"

typedef std::string SettingDefID;

struct SettingDef
{
    SettingDefID id;
    std::string description;
    std::string current_value;

    ChannelObject* write() const
    {
        declare_str_property_on_stack(id_, OBJ_PROPERTY_ID, id.c_str(), nullptr);
        declare_str_property_on_stack(description_, OBJ_PROPERTY_TITLE, description.c_str(), &id_);
        declare_str_property_on_stack(value_, 'v', current_value.c_str(), &description_);

        return channel_object_allocate(&value_);
    }
};

#endif
