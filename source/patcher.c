#include <stdint.h>
#include "std/unused.h"
#include "std/memory.h"
#include "firm/firm.h"
#include "config.h"
#include "common.h"

// TODO - Basically all this needs to move to patcher programs.

uint32_t wait_key();
int execp(char* path);

// A portion of this file is inherited from Luma3DS.
/*u32 getLoader(u8 *pos, u32 *loaderSize) {
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
*/

/* int patch_signatures() {
    //Look for signature checks

	uint8_t pat1[] = {0xC0, 0x1C, 0x76, 0xE7};
	uint8_t pat2[] = {0xB5, 0x22, 0x4D, 0x0C};

	// The code segment.
	uint8_t *firm_mem = (uint8_t*)firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
	uint32_t size = firm_p9_exefs->fileHeaders[0].size;

    uint8_t *off  = memfind(firm_mem, size, pat1, 4);

	// We're subbing one because the code goes back 1.
	// Unique patterns, etc.
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

	// See asm/sigpatches.s for the code here
	uint8_t sigpatch[] = {0x00, 0x20, 0x70, 0x47};

	memcpy(off,  sigpatch, 2);
	memcpy(off2, sigpatch, 4);

	fprintf(stderr, "Signature patch succeded.\n");

	return 0;
} */

int patch_firmprot() {
	uint8_t *firm_mem = (uint8_t*)firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
	uint32_t size = firm_p9_exefs->fileHeaders[0].size;

    // We look for 'exe:' first; this string is close to what we patch
    uint8_t* off = memfind(firm_mem, size, (uint8_t*)"exe:", 4);

	if(off == NULL) {
		fprintf(stderr, "Couldn't find 'exe:' string.\n");
		return 1;
	}

	fprintf(stderr, "Firmprot: 'exe:' string @ %x\n", (uint32_t)off);

    uint8_t pattern[] = {0x00, 0x28, 0x01, 0xDA};

    uint8_t* firmprot = memfind(off - 0x100, 0x100, pattern, 4);

	if(firmprot == NULL) {
		fprintf(stderr, "Couldn't find firmprot code.\n");
		return 2;
	}

	fprintf(stderr, "Firmprot: %x\n", (uint32_t)firmprot);

	uint8_t patch[] = {0x00, 0x20, 0xC0, 0x46};
	memcpy(firmprot, patch, 4);

	fprintf(stderr, "Applied firmprot patch.\n");

	return 0;
}

void wait() {
	if (config.options[OPTION_TRACE]) {
		fprintf(stderr, "[press key]\n");
		wait_key();
	}
}

int patch_firm_all() {
	// FIXME - Linker is bork at the moment.
	execp(PATH_PATCHES "/example.vco");

	wait();

	// Use builtin signature patcher?

	// TODO - Obviously these get moved to external patchers.
	fprintf(stderr, "Sigpatch: %s\n", ((config.options[OPTION_SIGPATCH]) ? "yes" : "no" ));
	fprintf(stderr, "Protect: %s\n",  ((config.options[OPTION_FIRMPROT]) ? "yes" : "no" ));

	wait();

	if (config.options[OPTION_SIGPATCH]) {
		// TODO - Patch menu. This is okay-ish for now.
		if(execp(PATH_PATCHES "/signatures.vco")) {
			abort("Fatal. Sigpatch has failed.");
		}
	}

	wait();

	if (config.options[OPTION_FIRMPROT]) {
		if(patch_firmprot()) {
			abort("Fatal. Firmprot has failed.");
		}
	}

	wait();

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

	return 0;
}
