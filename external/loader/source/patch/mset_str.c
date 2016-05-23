#include <3ds.h>
#include "../patcher.h"
#include "../ifile.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include "../config.h"
#include "../../../../source/patch_format.h"

void
settings_string(u64 progId, u8* code, u32 size)
{
    static const u16 verPattern[] = u"Ver.";
    static const u16 verPatch[] = u".hax";

    // Patch Ver. string
    patchMemory(code, size, verPattern, sizeof(verPattern) - sizeof(u16), 0,
                verPatch, sizeof(verPatch) - sizeof(u16), 1);
}
