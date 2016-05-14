#include <stdint.h>
#include "std/unused.h"
#include "std/memory.h"
#include "firm/firm.h"
#include "config.h"
#include "common.h"

uint32_t wait_key();

// A portion of this file is inherited from Luma3DS.
/*
u16 *getFirmWrite(u8 *pos, u32 size) {
    //Look for FIRM writing code
    u8 *const off = memsearch(pos, "exe:", size, 4);
    const u8 pattern[] = {0x00, 0x28, 0x01, 0xDA};

    return (u16 *)memsearch(off - 0x100, pattern, 0x100, 4);
}

u16 *getFirmWriteSafe(u8 *pos, u32 size) {
    //Look for FIRM writing code
    const u8 pattern[] = {0x04, 0x1E, 0x1D, 0xDB};

    return (u16 *)memsearch(pos, pattern, size, 4);
}

u32 getLoader(u8 *pos, u32 *loaderSize) {
    u8 *off = pos;
    u32 size;

    while(1)
    {
        size = *(u32 *)(off + 0x104) * 0x200;
        if(*(u32 *)(off + 0x200) == 0x64616F6C) break;
        off += size;
    }

    *loaderSize = size;

    return (u32)(off - pos);
}


// patch_location = (void *)((uintptr_t)firm + section->offset + (version->offset - section->address));

u8 *getProcess9(u8 *pos, u32 size, u32 *process9Size, u32 *process9MemAddr)
{
    u8 *off = memsearch(pos, "ess9", size, 4);

    *process9Size = *(u32 *)(off - 0x60) * 0x200;
    *process9MemAddr = *(u32 *)(off + 0xC);

    //Process9 code offset (start of NCCH + ExeFS offset + ExeFS header size)
    return off - 0x204 + (*(u32 *)(off - 0x64) * 0x200) + 0x200;
}
*/

int patch_signatures() {
    //Look for signature checks

	uint8_t pat1[] = {0xC0, 0x1C, 0x76, 0xE7};
	uint8_t pat2[] = {0xB5, 0x22, 0x4D, 0x0C};

	uint8_t *firm_mem = (uint8_t*)firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
	uint32_t size = firm_p9_exefs->fileHeaders[0].size;

    uint8_t *off  = memfind(firm_mem, size, pat1, 4);
    uint8_t *off2 = memfind(firm_mem, size, pat2, 4) - 1;

	if (off == NULL) {
		fprintf(stderr, "Signature patch failed on P0.\n");
		return 1; // Failed to find sigpatch. Ugh.
	}

	if (off2 == NULL) {
		fprintf(stderr, "Signature patch failed on P1.\n");
		return 2; // Failed to find sigpatch. Ugh.
	}

	fprintf(stderr, "Signatures[0]: 0x%x\n", (uint32_t)off);
	fprintf(stderr, "Signatures[1]: 0x%x\n", (uint32_t)off2);

	uint8_t sigpatch[] = {0x00, 0x20, 0x70, 0x47};

	memcpy(off,  sigpatch, 2);
	memcpy(off2, sigpatch, 4);
	fprintf(stderr, "Signature patch succeded.\n");

	return 0;
}

int patch_firm_all() {
	// Use builtin signature patcher?

	fprintf(stderr, "Signature patch: %s\n", ((config.options[OPTION_SIGPATCH]) ? "yes" : "no" ));
	if (config.options[OPTION_SIGPATCH]) {
		if(patch_signatures()) {
			abort("Fatal. Sigpatch has failed.");
		}
	}

	// Replace loader?
	if (config.options[OPTION_LOADER]) {
		// Yes.

		// This requires OPTION_SIGPATCH.
	}

	// Use ARM9 hook thread?
	if (config.options[OPTION_ARM9THREAD]) {
		// Yes.

		// FIXME - NYI
	}

	wait_key();

	return 0;
}
