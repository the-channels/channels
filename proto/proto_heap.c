#include "proto_objects.h"
#include "proto_asserts.h"

#include <stdlib.h>
#include <string.h>

ChannelObject* channel_object_allocate(ChannelStackObjectProperty* last_property)
{
    uint16_t object_size = 0;
    ChannelStackObjectProperty* property = last_property;
    uint8_t number_of_properties = 0;

    while (property)
    {
        if (property->value_size)
        {
            object_size += property->value_size + sizeof(ChannelObjectProperty);
            number_of_properties++;
        }
        property = property->prev;
    }

    uint8_t* raw_malloc = malloc(sizeof(ChannelObject) + (number_of_properties + 1) * sizeof(ChannelObjectProperty*) + object_size + 2);
    ChannelObject* obj = (ChannelObject*)raw_malloc;
    raw_malloc += sizeof(ChannelObject) + (number_of_properties + 1) * sizeof(ChannelObjectProperty*) + 2;
    obj->object_size = object_size;
    ChannelObjectProperty** res_prop = obj->properties;

    property = last_property;
    while (property)
    {
        if (property->value_size)
        {
            ChannelObjectProperty *target_property = (ChannelObjectProperty *) raw_malloc;
            *res_prop++ = target_property;

            target_property->key = property->key;
            target_property->value_size = property->value_size;
            memcpy(target_property->value, property->value, property->value_size);

            raw_malloc += target_property->value_size + sizeof(ChannelObjectProperty);
        }
        property = property->prev;
    }

    *res_prop = NULL;
    return obj;
}

ChannelObject* channel_object_copy(ChannelObject* obj)
{
    uint8_t number_of_properties = 0;
    {
        ChannelObjectProperty** p = obj->properties;

        while (*p)
        {
            number_of_properties++;
            p++;
        }
    }

    uint8_t* raw_malloc = malloc(sizeof(ChannelObject) + (number_of_properties + 1) * sizeof(ChannelObjectProperty*) + obj->object_size + 2);
    ChannelObject* copy = (ChannelObject*)raw_malloc;
    uint8_t* dst_data = raw_malloc + sizeof(ChannelObject) + (number_of_properties + 1) * sizeof(ChannelObjectProperty*) + 2;

    copy->object_size = obj->object_size;


    {
        uint8_t* it = dst_data;

        ChannelObjectProperty** cp = copy->properties;
        ChannelObjectProperty** p = obj->properties;

        while (*p)
        {
            ChannelObjectProperty* prop = (ChannelObjectProperty*)it;
            memcpy(it, *p, (*p)->value_size + sizeof(ChannelObjectProperty));
            it += (*p)->value_size + sizeof(ChannelObjectProperty);
            *cp++ = prop;
            p++;
        }

        *cp = NULL;
    }

    return copy;
}