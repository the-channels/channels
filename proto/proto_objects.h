#ifndef PROTO_OBJECTS_H
#define PROTO_OBJECTS_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OBJ_PROPERTY_PAYLOAD        = 0x00,
    OBJ_PROPERTY_ERROR          = 0x01,
    OBJ_PROPERTY_ID             = 0x02,
    OBJ_PROPERTY_TITLE          = 0x03,
    OBJ_PROPERTY_COMMENT        = 0x04
} ChannelsObjectPropertyKey;

#pragma pack(push)
#pragma pack(1)

typedef struct
{
    uint16_t value_size;
    uint8_t key;
    char value[];
} ChannelObjectProperty;

typedef struct ChannelStackObjectProperty
{
    uint8_t key;
    const char* value;
    uint16_t value_size;
    struct ChannelStackObjectProperty* prev;
} ChannelStackObjectProperty;

/*
 * A very space efficient generic object witch abuses C flexible array members.
 *
 * Memory allocated to this object is always bigger than the struct itself. The actual struct size is just 2 bytes
 * (uint16_y), the rest is an array of pointers to data that comes immediately after. That way we can iterate
 * over this object very quickly, but the object was created with only one allocation.
 *
 * Layout:
 * - 2 bytes - object_size
 * - N + 1 pointers - amount of ChannelObjectProperty* for each property, and NULL for last one
 * - 2 bytes - reserved for convenient send
 * - property data - pointers above point to this data. each property comes immediately after previous one
 *
 * Freeing such an object is just a matter of one free() even though it has dynamic amount of properties,
 * they're all allocated in same blob of data.
 */

typedef struct
{
    uint16_t object_size;
    ChannelObjectProperty* properties[];
} ChannelObject;

#pragma pack(pop)


#define declare_str_property_on_stack(name, property_key, property_value, property_prev) \
    ChannelStackObjectProperty name;                                  \
    {                                                                 \
        name.key = property_key;                                      \
        name.value = (const char*)property_value;                     \
        name.value_size = strlen(name.value);                         \
        name.prev = property_prev;                                    \
    }

#define declare_arg_property_on_stack(name, property_key, property, property_prev) \
    ChannelStackObjectProperty name;                                  \
    {                                                                 \
        name.key = property_key;                                      \
        name.value = (const char*)&property;                          \
        name.value_size = sizeof(property);                           \
        name.prev = property_prev;                                    \
    }

#define declare_variable_property_on_stack(name, property_key, property_ptr, property_size, property_prev) \
    ChannelStackObjectProperty name;                                  \
    {                                                                 \
        name.key = property_key;                                      \
        name.value = (const char*)property_ptr;                       \
        name.value_size = property_size;                              \
        name.prev = property_prev;                                    \
    }

#define declare_object_on_stack(name, buffer_size, last_property)     \
    uint8_t name ## _buffer [buffer_size];                            \
    ChannelObject* name = (ChannelObject*)name ## _buffer;            \
    channel_object_assign(name, buffer_size, last_property);

/*
 * Construct an object from a single-linked list of properties (usually declared on stack via declare_property_on_stack)
 * Empty properties (zero size) are ignored.
 * This object is required to be free()'d
 */
extern ChannelObject* channel_object_allocate(ChannelStackObjectProperty* last_property);

/*
 * Make a copy of an existing object. A new blob of data is allocated and all pointers are recalculated.
 * This object is required to be free()'d
 */
extern ChannelObject* channel_object_copy(ChannelObject* obj);

/*
 * Initialize an object with properties from a single-linked list. An obj could be just a pointer to a buffer that
 * has enough data to fit whole object. No allocations happen in the process so the object can live on stack.
 * Empty properties (zero size) are ignored.
 * This is used by declare_object_on_stack
 */
extern void channel_object_assign(ChannelObject* obj, uint16_t buffer_available, ChannelStackObjectProperty* last_property);

/*
 * Read up an object by known size from some buffer. The obj could be initialized, but no data is actually copied
 * from the buffer_from (needs to be of size just to fit N + 1 property references) - the data is only referenced.
 * No allocations happen so obj could be on stack.
 * If object is required to be used past buffer_from lifetime, a copy has to be made
 */
extern void channel_object_read(ChannelObject* obj, uint16_t buffer_available, uint16_t object_size, const uint8_t* buffer_from);

/*
 * Find a property by a given key. An object can contain multiple properties with the same key,
 * this function returns only the first match.
 */
extern ChannelObjectProperty* find_property(ChannelObject* o, uint8_t key);

/*
 * Get uint16_t property value by a given key. If no property has been found, the default value is returned.
 * An object can contain multiple properties with the same key, this function returns only the first match.
 */
extern uint16_t get_uint16_property(ChannelObject* o, uint8_t key, uint16_t def);

/*
 * Get uint8_t property value by a given key. If no property has been found, the default value is returned.
 * An object can contain multiple properties with the same key, this function returns only the first match.
 */
extern uint8_t get_uint8_property(ChannelObject* o, uint8_t key, uint8_t def);

/*
 * Returns a pointer to char[object_size + 2] buffer containing the whole object including its properties and size.
 * When sending this over the net, send object_size + 2 bytes
 */
extern uint8_t* channel_object_data(ChannelObject* o);

#ifdef __cplusplus
}
#endif

#endif