#ifdef WIN32
#include <windows.h>
#endif

#include <sys/stat.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

#include "fileinfo.h"

int get_fileinfo(const char *path, fileinfo_t *fileinf)
{
    if(path == NULL || fileinf == NULL)
        return EINVAL;


    fileinf->flags = 0;
    fileinf->size = 0;
    fileinf->c_time = 0;
    fileinf->m_time = 0;

    // Find the last path seprator character
    const char *i, *namestart = NULL;
    for(i = path; *i != '\0'; i++)
    {
        if(*i == FILEINFO_PATHSEPARATOR)
            namestart = i + 1;
    }
    if(namestart == NULL)
        namestart = path;

#ifdef WIN32
    WIN32_FILE_ATTRIBUTE_DATA fdata;    
    if(true == GetFileAttributesEx(path, GetFileExInfoStandard, &fdata))
    {
        // NOTE: We're only going to return the 32bits worth of data, which means very large files will have the wrong size reported
        if(fdata.nFileSizeHigh > 0 || fdata.nFileSizeLow > 0xFFFFFFFF)
            fileinf->size = 0xFFFFFFFF;
        else
            fileinf->size = fdata.nFileSizeLow;

        // Convert time values from FILETIME to time_t
#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600LL
        uint64_t wtime = ((uint64_t)fdata.ftLastWriteTime.dwHighDateTime << 32) + fdata.ftLastWriteTime.dwLowDateTime;
        fileinf->m_time = (wtime / WINDOWS_TICK - SEC_TO_UNIX_EPOCH);

        wtime = ((uint64_t)fdata.ftCreationTime.dwHighDateTime << 32) + fdata.ftCreationTime.dwLowDateTime;
        fileinf->c_time = (wtime / WINDOWS_TICK - SEC_TO_UNIX_EPOCH);

        // Set some flags
        if(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            fileinf->flags |= FILEINFOFLAG_DIRECTORY;
        if(fdata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
            fileinf->flags |= FILEINFOFLAG_HIDDEN;
        if(fdata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
            fileinf->flags |= FILEINFOFLAG_SPECIAL;

        // Count "." and ".." as SPECIAL
        if(namestart[0] == '.')
        {
            if(namestart[1] == '\0' || (namestart[1] == '.' && namestart[2] == '\0'))
                fileinf->flags |= FILEINFOFLAG_SPECIAL;
        }
    }
    else
    {
        return GetLastError();
    }

#else
    struct stat statinfo;

    if (stat(path, &statinfo) == 0)
    {
        if (S_ISDIR(statinfo.st_mode))
        {
            fileinf->flags |= FILEINFOFLAG_DIRECTORY;
        }
        fileinf->size =  statinfo.st_size;
        fileinf->m_time = statinfo.st_mtime;
        fileinf->c_time = statinfo.st_ctime;

        if(namestart[0] == '.')
            fileinf->flags |= FILEINFOFLAG_HIDDEN;
    }
    else
    {
        return errno;
    }

#endif

    return 0;
}
