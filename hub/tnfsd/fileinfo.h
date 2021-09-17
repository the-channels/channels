#ifndef _FILEINFO_H
#define _FILEINFO_H

#include <stdint.h>

#define FILEINFOFLAG_DIRECTORY 0x01
#define FILEINFOFLAG_HIDDEN 0x02
#define FILEINFOFLAG_SPECIAL 0x04

#ifdef WIN32
#define FILEINFO_PATHSEPARATOR '\\'
#else
#define FILEINFO_PATHSEPARATOR '/'
#endif

struct fileinfo
{
    uint8_t flags;
    uint32_t size;
    uint32_t m_time;
    uint32_t c_time;

};
typedef struct fileinfo fileinfo_t;

int get_fileinfo(const char *path, fileinfo_t *fi);

#endif // _FILEINFO_H
