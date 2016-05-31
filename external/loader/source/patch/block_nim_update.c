#include "patch.h"

void
disable_nim_updates(u64 progId, u8* code, u32 size)
{
    static const u8 blockAutoUpdatesPattern[] = { 0x25, 0x79, 0x0B, 0x99 };
    static const u8 blockAutoUpdatesPatch[] = { 0xE3, 0xA0 };

    // Block silent auto-updates
    patchMemory(code, size, blockAutoUpdatesPattern,
                sizeof(blockAutoUpdatesPattern), 0, blockAutoUpdatesPatch,
                sizeof(blockAutoUpdatesPatch), 1);
}
