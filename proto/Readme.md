# Channel Hub Protocol Socket API

## Overview

This page describes a protocol on which the client (viewer) and the server (hub)
 communicate over the TCP channel. 

```c
client <=> server // tcp SYN, SYN+ACK, ACK

client -> [request] -> server
client <- [response] <- server

client -/- server // tcp FIN, FIN+ACK, ACK
```

The communication lays on top of TCP and
consists of request-response pairs like so:

```c
request or response {
    uint16_t size;
    uint16_t request_id;
    // array<char, size> {
    object a;
    object b;
    object c;
    ...
    // }
}
```

`size` defines amount of bytes allocated for all objects combined on a particular request or response, excluding
the `size` and `request_id`.

## Object structure

Each object is represented by generic key-value structure with following syntax:

```c
object {
    uint16_t object_size;
    // array<char, object_size> {
    object_property a; 
    object_property b; 
    object_property c;
    ...
    // }
}
```

`object_size` defines amount of bytes allocated for all properties combined. Each property is defined like so:

```c
object_property
{
    uint8_t key;
    uint16_t value_size;
    array<char, value_size> value;
}
```

On the documentation below, each object is described like so:
```c
{
    KEY1 "value"
    KEY2 "value1"
    KEY2 "value2"
}
```

An object can have multiple properties with the same key.
On all API calls below, a request-response transaction is documented as follows:
```c
-> [{
    OBJ_PROPERTY_ID "test"
}]
<- [{
    OBJ_PROPERTY_ID "hello"
    OBJ_PROPERTY_TITLE "a"
}, {
    OBJ_PROPERTY_ID "hello"
    OBJ_PROPERTY_TITLE "b"
}]
```

That means the client sent a request with one object,
while server responded with two objects.

* The first object of the request (Request Object) defines the type of the request.
  `OBJ_PROPERTY_ID` property defines the API call of that request.
* If the first object on a response has `OBJ_PROPERTY_ERROR` key, that means
  the whole request has failed and instead of processing regular objects,
  application should process an error path:
  ```c
  -> [{
      OBJ_PROPERTY_ID "haha"
  }]
  <- [{
      OBJ_PROPERTY_ERROR "unknown_api_call"
  }]
  ```

## Table of Property Keys

| key        | type                     | description                          |
|------------|--------------------------|--------------------------------------|
| `0x00`     | OBJ_PROPERTY_PAYLOAD     | Generic Payload                      |
| `0x01`     | OBJ_PROPERTY_ERROR       | A code of an error                   |
| `0x02`     | OBJ_PROPERTY_ID          | Name or ID                           |
| `0x03`     | OBJ_PROPERTY_TITLE       | Title                                |
| `0x04`     | OBJ_PROPERTY_COMMENT     | Comment                              |
| `*`        | Any character            | Request-specific like `t` for Thread |

# API Calls

## Api Version

The first API call the client should do, is determine API version of the
server.

```c
-> [{
    OBJ_PROPERTY_ID "api"
    'k' "<key>" (optional)
}]
<- [{
    OBJ_PROPERTY_ID "<api-version>"
}]
```

* `<api-version>`: The version of the API, currently `5`
* If key (`k`) is specified, it will be then used to store client's settings. 
  Future sessions with same key will share same settings. A MAC address (6 bytes)
  is usually used as a key.

## Get Channels

```c
-> [{
    OBJ_PROPERTY_ID "channels"
}]
<- [channel_a, channel_b, channel_c, ...]
```

Each channel MUST have `OBJ_PROPERTY_ID` and `OBJ_PROPERTY_TITLE` attributes.

## Get Boards

```c
-> [{
    OBJ_PROPERTY_ID "boards"
    'c' "<channel>"
    'l' uint16_t limit (optional, default 128)
}]
<- [board_a, board_b, board_c, ...]
```

Each board MUST have `OBJ_PROPERTY_ID` and `OBJ_PROPERTY_TITLE` attributes.

## Get Threads

```c
-> [{
    OBJ_PROPERTY_ID "threads"
    'c' "<channel>"
    'b' "<board>"
    'o' uint16_t offset (optional)
    'l' uint16_t limit (optional, default 128)
    'f' uint8_t flush_cache (optional, default 0)
}]
<- [page_info, thread_a, thread_b, thread_c, ...]
```
Optional request attributes:
* `o`/`l` select a subset `[offset, offset+limit]` of threads
* `f` when defined, the connection-only cache is flushed. Take care
  with defining that while also defining offsets.

`page_info` object COULD have a `c` property with number of threads and COULD be empty.

Each `thread_*` object MUST have `OBJ_PROPERTY_ID` (thread id) attribute. 

Optional attributes:
* `OBJ_PROPERTY_TITLE`
* `OBJ_PROPERTY_COMMENT`
* `a` (uint16_t) for attachment image. Could be several.

## Get Thread

```c
-> [{
    OBJ_PROPERTY_ID "thread"
    'c' "<channel>"
    'b' "<board>"
    't' "<thread>"
    'r' "<replies_to>" (optional)
    'o' uint16_t offset (optional)
    'l' uint16_t limit (optional, default 128)
    'f' uint8_t flush_cache (optional, default 0)
}]
<- [thead_info, post_a, post_b, post_c, ...]
```

Optional request attributes:
* `o`/`l` select a subset `[offset, offset+limit]` of posts
* `r` get posts that reply only to a specific post. offsets/limits above still apply
* `f` when defined, the connection-only cache is flushed. Take care
  with defining that while also defining offsets.

`thread_info` object COULD have `c` property with number of posts and COULD be empty.

Each `post_*` object MUST have `OBJ_PROPERTY_ID` (post id) attribute. 

Optional response entry attributes:
* `OBJ_PROPERTY_TITLE`
* `OBJ_PROPERTY_COMMENT`
* `a` (uint16_t) for attachment image. Could be several.

## Get Attachment Image

```c
-> [{
    OBJ_PROPERTY_ID "image"
    'c' "<channel>"
    'i' uint16_t image_id
    'w' uint16_t target_width
    'h' uint16_t target_height
}]
<- [image_info, chunk_a, chunk_b, chunk_c, ...]
```
`encoding` (`e`) attribute defines the reencoding scheme of the received image.
Supported schemes:

| Scheme         | Description |
|----------------|-------------|
| `color_zx`     | Color ZX spectrum image. Bitmap data is received first, following by color attributes. If target_width is less than fullscreen (256) the scanlines of the image are placed linearly instead naturally for ZX spectrum. That way, the fullscreen image can be copied to `0x4000` as is, but smaller images need to be played around with. |
| `grayscale_zx` | Same as above but there is no color attribute info. |

`image_info` object MUST have these properties:

| Property        | Description |
|-----------------|-------------|
| `w`             | Target image width |
| `h`             | Target image height |
| `s`             | Target image raw data size in bytes |

Following that object, an unspecified amount of objects with raw image data SHOULD follow.
Each data object SHOULD have `OBJ_PROPERTY_PAYLOAD` property with chunk's payload.
A total combined payload sizes of these objects SHOULD match the image raw data size as advertised in `s` property.
Each chunk's payload size SHOULD NOT be greater than 1024.

## Get Channel Settings Definitions

```c
-> [{
    OBJ_PROPERTY_ID "setting_defs"
    'c' "<channel>"
}]
<- [def_a, def_b, def_c, ...]
```

This call should allow a client to obtain a list of settings that channels supports.

Each setting SHOULD contain the following:

| Property        | Description |
|-----------------|-------------|
| OBJ_PROPERTY_ID | Setting ID |
| OBJ_PROPERTY_TITLE | Setting description |
| `t`             | Setting type (e.g. `string`, `bool` or `int`) |

