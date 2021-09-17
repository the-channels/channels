#include "proto_objects.h"
#include "proto_asserts.h"

#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <Winsock2.h>
#else
#include <sys/socket.h>
#ifndef __SPECTRUM
#include <arpa/inet.h>
#endif
#endif


void channel_object_assign(ChannelObject* obj, uint16_t buffer_available, ChannelStackObjectProperty* last_property)
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

    uint16_t offsets = sizeof(ChannelObject) + (number_of_properties + 1) * sizeof(ChannelObjectProperty*) + 2;
    proto_assert_str(object_size + offsets <= buffer_available, "overrun");

    uint8_t* raw_data = (uint8_t*)obj + offsets;
    obj->object_size = object_size;
    ChannelObjectProperty** res_prop = obj->properties;

    property = last_property;
    while (property)
    {
        if (property->value_size)
        {
            ChannelObjectProperty *target_property = (ChannelObjectProperty *) raw_data;
            *res_prop++ = target_property;

            target_property->key = property->key;
            target_property->value_size = property->value_size;
            memcpy(target_property->value, property->value, property->value_size);

            raw_data += target_property->value_size + sizeof(ChannelObjectProperty);
        }
        property = property->prev;
    }

    *res_prop = NULL;
}

uint8_t channel_object_read(ChannelObject* obj, uint16_t buffer_available, uint16_t object_size, const uint8_t* buffer_from)
{
    uint8_t number_of_properties = 0;
    {
        // find out number of properties
        const uint8_t* it = buffer_from;
        const uint8_t* end = it + object_size;
        while (it < end)
        {
            ++number_of_properties;
            uint16_t value_size = *(uint16_t*)it;
            it += sizeof(ChannelObjectProperty) + value_size;
        }

        if (it != end)
        {
            return 1;
        }
    }

    if (sizeof(ChannelObject) + (number_of_properties + 1) * sizeof(ChannelObjectProperty*) > buffer_available)
    {
        return 2;
    }

    obj->object_size = object_size;

    ChannelObjectProperty** properties = obj->properties;

    {
        // initialize pointers
        const uint8_t* it = buffer_from;
        const uint8_t* end = it + object_size;
        while (it < end)
        {
            *properties++ = (ChannelObjectProperty*)it;
            uint16_t value_size = ((ChannelObjectProperty*)it)->value_size;
            it += sizeof(ChannelObjectProperty) + value_size;
        }
    }

    *properties = NULL;
    return 0;
}

ChannelObjectProperty* find_property(ChannelObject* o, uint8_t key)
{
    ChannelObjectProperty** prop = o->properties;
    while (*prop)
    {
        if ((*prop)->key == key)
        {
            return *prop;
        }
        prop++;
    }

    return NULL;
}

ChannelObjectProperty* find_property_match(ChannelObject* o, uint8_t key, const char* match)
{
    uint8_t len = strlen(match);

    ChannelObjectProperty** prop = o->properties;
    while (*prop)
    {
        ChannelObjectProperty* p = *prop;
        if (p->key == key && p->value_size >= len && (memcmp(p->value, match, len) == 0))
        {
            return p;
        }
        prop++;
    }

    return NULL;
}

uint16_t get_uint16_property(ChannelObject* o, uint8_t key, uint16_t def)
{
    ChannelObjectProperty** prop = o->properties;
    while (*prop)
    {
        if ((*prop)->key == key && (*prop)->value_size == sizeof(uint16_t))
        {
            uint16_t result;
            memcpy(&result, (*prop)->value, sizeof(uint16_t));
            return result;
        }
        prop++;
    }

    return def;
}

uint8_t get_uint8_property(ChannelObject* o, uint8_t key, uint8_t def)
{
    ChannelObjectProperty** prop = o->properties;
    while (*prop)
    {
        if ((*prop)->key == key && (*prop)->value_size == sizeof(uint8_t))
        {
            uint8_t result;
            memcpy(&result, (*prop)->value, sizeof(uint8_t));
            return result;
        }
        prop++;
    }

    return def;
}

uint8_t* channel_object_data(ChannelObject* o)
{
    uint8_t number_of_properties = 0;

    while(o->properties[number_of_properties])
    {
        number_of_properties++;
    }

    uint8_t* d = (uint8_t*)o + sizeof(ChannelObject) + (number_of_properties + 1) * sizeof(ChannelObjectProperty*);
    /*
     * This little trick uses the 2 reserved bytes on allocations as a way to send object at one call without reallocations
     * Those two bytes are getting updated with object size prior to sending. Thus, when sending, you should send object_size + 2
     */
    *(uint16_t*)d = o->object_size;
    return d;
}
