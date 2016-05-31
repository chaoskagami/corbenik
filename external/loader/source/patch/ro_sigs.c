#include "patch.h"

void
ro_sigpatch(u64 progId, u8* code, u32 size)
{
    static const u8 sigCheckPattern[] = { 0x30, 0x40, 0x2D, 0xE9,
                                          0x02, 0x50, 0xA0, 0xE1 };
    static const u8 sha256ChecksPattern1[] = { 0x30, 0x40, 0x2D, 0xE9,
                                               0x24, 0xD0, 0x4D, 0xE2 };
    static const u8 sha256ChecksPattern2[] = { 0xF8, 0x4F, 0x2D, 0xE9,
                                               0x01, 0x70, 0xA0, 0xE1 };

    // mov r0, #0; bx lr - equivalent to 'return 0;'
    static const u8 stub[] = { 0x00, 0x00, 0xA0, 0xE3, 0x1E, 0xFF, 0x2F, 0xE1 };

    // Disable CRR0 signature (RSA2048 with SHA256) check
    patchMemory(code, size, sigCheckPattern, sizeof(sigCheckPattern), 0, stub,
                sizeof(stub), 1);

    // Disable CRO0/CRR0 SHA256 hash checks (section hashes, and hash table)
    patchMemory(code, size, sha256ChecksPattern1, sizeof(sha256ChecksPattern1),
                0, stub, sizeof(stub), 1);

    patchMemory(code, size, sha256ChecksPattern2, sizeof(sha256ChecksPattern2),
                0, stub, sizeof(stub), 1);

	logstr("  ro_sigpatch\n");
}
