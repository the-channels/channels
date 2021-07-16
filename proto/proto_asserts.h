#ifndef CHANNELS_PROTO_ASSERTS_H
#define CHANNELS_PROTO_ASSERTS_H

#ifdef NDEBUG
#define proto_assert(cond, msg, ...) ((void)0)
#define proto_assert_str(cond, msg) ((void)0)
#else
#ifdef __SPECTRUM
#include "netlog.h"
#include <spectrum.h>
#define proto_assert_str(cond, msg) if (!(cond)) { netlog(msg " at %s:%d\n", __FILE__, __LINE__); while(1) { zx_border(INK_RED); zx_border(INK_BLACK);  } }
#define proto_assert(cond, msg, ...) if (!(cond)) { netlog(msg " at %s:%d\n", __VA_ARGS__, __FILE__, __LINE__); while(1) { zx_border(INK_RED); zx_border(INK_BLACK);  } }
#else
#include <assert.h>
#include <stdio.h>
#define proto_assert_str(cond, msg) if (!(cond)) { printf(msg " at %s:%d\n", __FILE__, __LINE__); assert(0); }
#define proto_assert(cond, msg, ...) if (!(cond)) { printf(msg " at %s:%d\n", __VA_ARGS__, __FILE__, __LINE__); assert(0); }
#endif
#endif

#endif //CHANNELS_PROTO_ASSERTS_H
