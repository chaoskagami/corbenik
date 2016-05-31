#include "patch.h"

void
settings_string(u64 progId, u8* code, u32 size)
{
    static const u16 verPattern[] = u"Ver.";
    static const u16 verPatch[] = u".hax";

    // Patch Ver. string
    patchMemory(code, size, verPattern, sizeof(verPattern) - sizeof(u16), 0,
                verPatch, sizeof(verPatch) - sizeof(u16), 1);
}
