#ifndef NETLOG_H
#define NETLOG_H

#include <string.h>
#include <stdio.h>

extern int netlog_init(const char* reporting_address, int port);
extern void netlog_str(const char *data);
extern char* netlog_buffer;

#define netlog(fmt, ...) sprintf(netlog_buffer, fmt, __VA_ARGS__); netlog_str(netlog_buffer)

#endif