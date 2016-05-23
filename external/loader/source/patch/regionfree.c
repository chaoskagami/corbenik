#include <3ds.h>
#include "../patcher.h"
#include "../ifile.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include "../config.h"
#include "../../../../source/patch_format.h"

void region_patch(u64 progId, u8 *code, u32 size) {
	static const u8 regionFreePattern[] = {0x00, 0x00, 0x55, 0xE3, 0x01, 0x10, 0xA0, 0xE3};
	static const u8 regionFreePatch[]   = {0x01, 0x00, 0xA0, 0xE3, 0x1E, 0xFF, 0x2F, 0xE1};

	//Patch SMDH region checks
	patchMemory(code, size,
		regionFreePattern,
		sizeof(regionFreePattern), -16,
		regionFreePatch,
		sizeof(regionFreePatch), 1
    );
}
