#include <3ds.h>
#include "../patcher.h"
#include "../ifile.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include "../config.h"
#include "../../../../source/patch_format.h"

void disable_nim_updates(u64 progId, u8 *code, u32 size) {
	static const u8 blockAutoUpdatesPattern[] = {0x25, 0x79, 0x0B, 0x99};
	static const u8 blockAutoUpdatesPatch[]   = {0xE3, 0xA0};

	//Block silent auto-updates
	patchMemory(code, size,
		blockAutoUpdatesPattern,
		sizeof(blockAutoUpdatesPattern), 0,
		blockAutoUpdatesPatch,
		sizeof(blockAutoUpdatesPatch), 1
	);
}

