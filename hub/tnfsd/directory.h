#ifndef _TNFS_DIRECTORY_H
#define _TNFS_DIRECTORY_H

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
 * TNFS daemon directory functions
 *
 * */

#include "tnfs.h"

#define TNFS_DIROPT_NO_FOLDERSFIRST 0x01 
#define TNFS_DIROPT_NO_SKIPHIDDEN 0x02
#define TNFS_DIROPT_NO_SKIPSPECIAL 0x04
#define TNFS_DIROPT_DIR_PATTERN 0x08

#define TNFS_DIRSORT_NONE 0x01
#define TNFS_DIRSORT_CASE 0x02
#define TNFS_DIRSORT_DESCENDING 0x04
#define TNFS_DIRSORT_MODIFIED 0x08
#define TNFS_DIRSORT_SIZE 0x10

#define TNFS_DIRSTATUS_EOF 0x01

/* initialize and set the root dir */
int tnfs_setroot(char *rootdir);

/* validates a path points to an actual directory */
int validate_dir(Session *s, const char *path);
void normalize_path(char *dst, char *src, int pathsz);

/* get the root directory for the given session */
void get_root(Session *s, char *buf, int bufsz);

/* handle list of directory entries */
void dirlist_free(directory_entry_list dlist);
void dirlist_push(directory_entry_list *dlist, directory_entry_list_node *node);
directory_entry_list_node * dirlist_get_node_at_index(directory_entry_list dlist, uint32_t index);
uint32_t dirlist_get_index_for_node(directory_entry_list dlist, directory_entry_list_node *node);
void dirlist_sort(directory_entry_list *dlist, uint8_t sortopts);

/* open, read, close directories */
void tnfs_opendir(Header *hdr, Session *s, unsigned char *databuf, int datasz);
void tnfs_readdir(Header *hdr, Session *s, unsigned char *databuf, int datasz);
void tnfs_closedir(Header *hdr, Session *s, unsigned char *databuf, int datasz);
void tnfs_seekdir(Header *hdr, Session *s, unsigned char *databuf, int datasz);
void tnfs_telldir(Header *hdr, Session *s, unsigned char *databuf, int datasz);

void tnfs_opendirx(Header *hdr, Session *s, unsigned char *databuf, int datasz);
void tnfs_readdirx(Header *hdr, Session *s, unsigned char *databuf, int datasz);

/* create and remove directories */
void tnfs_mkdir(Header *hdr, Session *s, unsigned char *databuf, int datasz);
void tnfs_rmdir(Header *hdr, Session *s, unsigned char *databuf, int datasz);

#endif // _TNFS_DIRECTORY_H

