#ifndef _TNFS_H
#define _TNFS_H

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
 * TNFS structs and definitions
 *
 * */

#include <stdint.h>
#include <dirent.h>
#include <time.h>

#ifdef UNIX
#include <arpa/inet.h>
#endif
#ifdef WIN32
#include <windows.h>
#define WIN32_CHAR_P (char *)
#else
#define WIN32_CHAR_P
#endif

#ifndef in_addr_t
#define in_addr_t uint32_t
#endif

#ifndef socklen_t
#define socklen_t int32_t
#endif

#include "config.h"

/* tnfs command IDs */
#define TNFS_MOUNT	0x00
#define TNFS_UMOUNT	0x01

#define TNFS_OPENDIR	0x10
#define TNFS_READDIR	0x11
#define TNFS_CLOSEDIR	0x12
#define TNFS_MKDIR      0x13
#define TNFS_RMDIR      0x14
#define TNFS_TELLDIR    0x15
#define TNFS_SEEKDIR    0x16
#define TNFS_OPENDIRX   0x17
#define TNFS_READDIRX   0x18

#define TNFS_OPENFILE_OLD 0x20
#define	TNFS_READBLOCK	0x21
#define TNFS_WRITEBLOCK	0x22
#define TNFS_CLOSEFILE	0x23
#define TNFS_STATFILE	0x24
#define TNFS_SEEKFILE	0x25
#define TNFS_UNLINKFILE	0x26
#define TNFS_CHMODFILE	0x27
#define TNFS_RENAMEFILE	0x28
#define TNFS_OPENFILE	0x29

/* command classes etc. */
#define CLASS_SESSION	0x00
#define CLASS_DIRECTORY	0x10
#define CLASS_FILE	0x20

#define NUM_SESSCMDS 2
#define NUM_DIRCMDS	9
#define NUM_FILECMDS 10

#define TNFS_DIRENTRY_DIR 0x01
#define TNFS_DIRENTRY_HIDDEN 0x02
#define TNFS_DIRENTRY_SPECIAL 0x04

// Extended directory entry contents
struct _dir_entry
{
    uint8_t flags;
    uint32_t size;
    uint32_t mtime;
    uint32_t ctime;
    char entrypath[MAX_FILENAME_LEN];
} __attribute__((packed));

typedef struct _dir_entry directory_entry;

struct _dir_entry_list_node
{
	directory_entry entry;
	struct _dir_entry_list_node *next;
} __attribute__((packed));

typedef struct _dir_entry_list_node directory_entry_list_node;
typedef directory_entry_list_node * directory_entry_list;

typedef struct _dir_handle
{
	DIR *handle;
	char path[MAX_TNFSPATH];
	uint16_t entry_count;
	directory_entry_list entry_list;
	directory_entry_list_node * current_entry;
} dir_handle;

typedef struct _session
{
	time_t last_contact; /* timestamp of last received request */
	uint16_t sid;			/* session ID */
	in_addr_t ipaddr;		/* client addr */
	uint8_t seqno;			/* last sequence number */
	int fd[MAX_FD_PER_CONN];	/* file descriptors */
	//DIR *dhnd[MAX_DHND_PER_CONN];	/* directory handles */
	//char dpaths[MAX_DHND_PER_CONN][MAX_TNFSPATH]; /* directory path for each handle */
	dir_handle dhandles[MAX_DHND_PER_CONN];
	char *root;			/* requested root dir */
	unsigned char lastmsg[MAXMSGSZ];/* last message sent */
#ifdef USAGELOG
	char lastpath[MAX_TNFSPATH];    /* last path visited */
#endif
	int lastmsgsz;			/* last message's size inc. hdr */
	uint8_t lastseqno;		/* last sequence number */
	uint8_t isTCP;			/* uses the TCP transport */
} Session;

typedef struct _header
{
	uint16_t sid;			/* session id */
	uint8_t seqno;			/* sequence number */
	uint8_t cmd;			/* command */
	uint8_t status;			/* command's status */
	in_addr_t ipaddr;		/* client address */
	uint16_t port;			/* client port address */
} Header;

typedef	void(*tnfs_cmdfunc)(Header *hdr, Session *sess,
				unsigned char *buf, int bufsz);

#endif

