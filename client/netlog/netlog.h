#ifndef NETLOG_H
#define NETLOG_H

#include <string.h>
#include <stdio.h>

extern int netlog_init(int port);
extern void netlog(const char *data);
extern void netlog_1(const char *a);
extern void netlog_2(const char *a, const char *b);
extern void netlog_3(const char *a, const char *b, const char *c);

#endif