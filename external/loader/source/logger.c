#include <3ds.h>
#include "patcher.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include <option.h>
#include <patch_format.h>

Handle log_file_hdl;
int logger_is_initd = 0;

extern struct config_file config;

void
openLogger()
{
    if (config.options[OPTION_SAVE_LOGS] == 0) {
        logger_is_initd = -1; // Setting -1 effectively disables logs.
    }
    Result r;

    if (logger_is_initd)
        return;

    r = fileOpen(&log_file_hdl, ARCHIVE_SDMC, PATH_LOADERLOG, FS_OPEN_WRITE | FS_OPEN_READ | FS_OPEN_CREATE);

    if (R_FAILED(r)) {
        logger_is_initd = -1;
    }

    logger_is_initd = 1;
}

void
logstr(const char *str)
{
    if (logger_is_initd == -1)
        return; // Errored during init. Don't bother.

    u32 len = strlen(str);
    u64 size;
    u32 wrote;
    Result r;

    // Get current size.
    r = FSFILE_GetSize(log_file_hdl, &size);
    if (R_FAILED(r))
        return;

    // Expand file size.
    r = FSFILE_SetSize(log_file_hdl, size + len);
    if (R_FAILED(r))
        return;

    // Write data.
    FSFILE_Write(log_file_hdl, &wrote, size, str, len, FS_WRITE_FLUSH);
}

void
logu64(u64 progId)
{
    char str[] = "Title: 0000000000000000\n";
    u32 i = 22;
    while (progId) {
        static const char hexDigits[] = "0123456789ABCDEF";
        str[i--] = hexDigits[(u32)(progId & 0xF)];
        progId >>= 4;
    }

    logstr(str);
}

void
closeLogger()
{
    FSFILE_Close(log_file_hdl);
    logger_is_initd = 0;
}

void
panicstr(const char *str)
{
    logstr(str);
    closeLogger();
    svcBreak(USERBREAK_ASSERT);
}

