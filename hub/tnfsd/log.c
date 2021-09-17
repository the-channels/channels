/* The MIT License
 *
 * Copyright (c) 2010 Dylan Smith
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * TNFS daemon logging functions
 *
 * */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>

#include "log.h"
#include "tnfs.h"

void die(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(-1);
}

void TNFSMSGLOG(Header *hdr, const char *msg, ...)
{
	unsigned char *ip = (unsigned char *)&hdr->ipaddr;
	char buff[128];

	va_list vargs;
	va_start(vargs, msg);

	vsnprintf(buff, sizeof(buff), msg, vargs);
	va_end(vargs);

	fprintf(stderr, "%d.%d.%d.%d s=%02x c=%02x q=%02x | %s\n", ip[0], ip[1], ip[2], ip[3], hdr->sid, hdr->cmd, hdr->seqno, buff);

#ifdef WIN32
	fflush(stderr);
#endif
}

void USGLOG(Header *hdr, const char *msg, ...)
{
	unsigned char *ip = (unsigned char *)&hdr->ipaddr;
	char buff[128];
	char sdate[20];
	struct tm *sTm;

	time_t now = time (0);
	sTm = gmtime (&now);

	strftime (sdate, sizeof(sdate), "%Y-%m-%d %H:%M:%S", sTm);

	va_list vargs;
	va_start(vargs, msg);

	vsnprintf(buff, sizeof(buff), msg, vargs);
	va_end(vargs);

	fprintf(stderr, "%s|%d.%d.%d.%d|SID=%02x|%s\n", sdate, ip[0], ip[1], ip[2], ip[3], hdr->sid, buff);

#ifdef WIN32
	fflush(stderr);
#endif
}

void MSGLOG(in_addr_t ipaddr, const char *msg, ...)
{
	unsigned char *ip = (unsigned char *)&ipaddr;
	char buff[128];

	va_list vargs;
	va_start(vargs, msg);

	vsnprintf(buff, sizeof(buff), msg, vargs);
	va_end(vargs);

	fprintf(stderr, "%d.%d.%d.%d | %s\n", ip[0], ip[1], ip[2], ip[3], buff);

#ifdef WIN32
	fflush(stderr);
#endif
}

void LOG(const char *msg, ...)
{
	va_list vargs;
	va_start(vargs, msg);

	vfprintf(stderr, msg, vargs);
	va_end(vargs);

#ifdef WIN32
	fflush(stderr);
#endif
}
