#include "patch.h"

/*
  find 8, 0xE0, 0x1E, 0xFF, 0x2F, 0xE1, 0x01, 0x01, 0x01
  fwd 9
  set 1, 0x06
 */

void
fake_friends_version(u64 progId, u8* code, u32 size)
{
    static const u8 fpdVerPattern[] = { 0xE0, 0x1E, 0xFF, 0x2F,
                                        0xE1, 0x01, 0x01, 0x01 };
    static const u8 fpdVerPatch = 0x06; // Latest version.

    // Allow online access to work with old friends modules
    patchMemory(code, size, fpdVerPattern, sizeof(fpdVerPattern), 9,
                &fpdVerPatch, sizeof(fpdVerPatch), 1);

	logstr("  fake_friends_version\n");
}
