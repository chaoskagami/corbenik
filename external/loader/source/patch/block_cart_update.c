#include <3ds.h>
#include "../memory.h"
#include "../patcher.h"
#include "../ifile.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include "../config.h"
#include "../../../../source/patch_format.h"

void disable_cart_updates(u64 progId, u8 *code, u32 size) {
	static const u8 stopCartUpdatesPattern[] = {0x0C, 0x18, 0xE1, 0xD8};
	static const u8 stopCartUpdatesPatch[]   = {0x0B, 0x18, 0x21, 0xC8};

	//Disable updates from foreign carts (makes carts region-free)
	patchMemory(code, size,
		stopCartUpdatesPattern,
		sizeof(stopCartUpdatesPattern), 0,
		stopCartUpdatesPatch,
		sizeof(stopCartUpdatesPatch), 2
	);
}

