#include "patch.h"

void
secureinfo_sigpatch(u64 progId, u8* code, u32 size)
{
    static const u8 secureinfoSigCheckPattern[] = { 0x06, 0x46, 0x10, 0x48,
                                                    0xFC };
    static const u8 secureinfoSigCheckPatch[] = { 0x00, 0x26 };

    // Disable SecureInfo signature check
    patchMemory(code, size, secureinfoSigCheckPattern,
                sizeof(secureinfoSigCheckPattern), 0, secureinfoSigCheckPatch,
                sizeof(secureinfoSigCheckPatch), 1);

	logstr("  secureinfo_sigpatch\n");
}
