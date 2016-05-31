#include <3ds.h>
#include "patcher.h"
#include "fsldr.h"
#include "internal.h"
#include "memory.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include "config.h"
#include "../../../source/patch_format.h"

#include "patch/patch.h"

Handle log_file_hdl;
int logger_is_initd = 0;

void openLogger() {
    Result r;

    if (logger_is_initd)
        return;

    r = fileOpen(&log_file_hdl, ARCHIVE_SDMC, "/corbenik/loader.log", FS_OPEN_WRITE|FS_OPEN_READ|FS_OPEN_CREATE);

    if (R_FAILED(r)) {
        logger_is_initd = -1;
    }

	logger_is_initd = 1;
}

void logstr(const char* str) {
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
	r = FSFILE_SetSize(log_file_hdl, size+len);
    if (R_FAILED(r))
		return;

	// Write data.
	FSFILE_Write(log_file_hdl, &wrote, size, str, len, 0);
}

void closeLogger() {
	FSFILE_Close(log_file_hdl);
	logger_is_initd = 0;
}
