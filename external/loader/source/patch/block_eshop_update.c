#include "patch.h"

void
disable_eshop_updates(u64 progId, u8* code, u32 size)
{
    static const u8 skipEshopUpdateCheckPattern[] = { 0x30, 0xB5, 0xF1, 0xB0 };
    static const u8 skipEshopUpdateCheckPatch[] = { 0x00, 0x20, 0x08,
                                                    0x60, 0x70, 0x47 };

    // Skip update checks to access the EShop
    patchMemory(code, size, skipEshopUpdateCheckPattern,
                sizeof(skipEshopUpdateCheckPattern), 0,
                skipEshopUpdateCheckPatch, sizeof(skipEshopUpdateCheckPatch),
                1);

	logstr("  disable_eshop_updates\n");
}
