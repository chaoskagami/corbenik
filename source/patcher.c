#include <stdint.h>
#include "std/unused.h"
#include "std/memory.h"
#include "firm/firm.h"
#include "config.h"
#include "common.h"

// TODO - Basically all this needs to move to patcher programs.

uint32_t wait_key();
int execp(char* path);

extern int patch_signatures();
extern int patch_firmprot();
extern int patch_services();
extern int patch_modules();

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

extern int doing_autoboot;

void wait() {
	if (config.options[OPTION_TRACE] && !doing_autoboot) {
		fprintf(stderr, "[press key]\n");
		wait_key();
	}
}

int patch_firm_all() {
	// FIXME - Linker is bork at the moment.
//	execp(PATH_PATCHES "/example.vco");

//	wait();

	// Use builtin signature patcher?

	// TODO - Obviously these get moved to external patchers.
	fprintf(stderr, "Sigpatch: %s\n", ((config.options[OPTION_SIGPATCH]) ? "yes" : "no" ));
	fprintf(stderr, "Protect: %s\n",  ((config.options[OPTION_FIRMPROT]) ? "yes" : "no" ));

	wait();

	if (config.options[OPTION_SIGPATCH]) {
		// TODO - Patch menu. This is okay-ish for now.
//		if(execp(PATH_PATCHES "/signatures.vco")) {
		if(patch_signatures()) {
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
		if(patch_modules()) {
			abort("Fatal. Service patch has failed.");
		}
		// This requires OPTION_SIGPATCH.
	}

	wait();

	// Inject services?
	if (config.options[OPTION_SERVICES]) {
		if(patch_services()) {
			abort("Fatal. Service patch has failed.");
		}
	}

	// Use ARM9 hook thread?
	if (config.options[OPTION_ARM9THREAD]) {
		// Yes.

		// FIXME - NYI
	}

	wait();

	return 0;
}
