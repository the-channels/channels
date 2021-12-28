#ifndef CHANNELS_PROTO_ASSERTS_H
#define CHANNELS_PROTO_ASSERTS_H

#ifdef __SPECTRUM
#include "netlog.h"
extern void proto_abort();
#define proto_assert_str(cond, msg) if (!(cond)) { netlog_1(msg " at " __FILE__); proto_abort(); }
#else
#include <assert.h>
#include <stdio.h>
#define proto_assert_str(cond, msg) if (!(cond)) { printf(msg " at %s:%d\n", __FILE__, __LINE__); assert(0); }
#define proto_assert(cond, msg, ...) if (!(cond)) { printf(msg " at %s:%d\n", __VA_ARGS__, __FILE__, __LINE__); assert(0); }
#endif

#endif //CHANNELS_PROTO_ASSERTS_H
