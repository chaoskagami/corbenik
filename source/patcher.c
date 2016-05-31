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
extern int patch_aadowngrade();

extern int doing_autoboot;

void
wait()
{
    if (config.options[OPTION_TRACE] && !doing_autoboot) {
        fprintf(stderr, "                                 [WAIT]");
        wait_key();
    }
    fprintf(stderr, "\r                                       \r");
}

int
patch_firm_all()
{
    // FIXME - Linker is bork at the moment.
    execp(PATH_PATCHES "/example.vco");

    //	wait();

    // Use builtin signature patcher?

    if (config.options[OPTION_SIGPATCH]) {
        // TODO - Patch menu. This is okay-ish for now.
        //		if(execp(PATH_PATCHES "/signatures.vco")) {
        if (patch_signatures()) {
            abort("Fatal. Sigpatch has failed.");
        }

	    wait();
    }

    if (config.options[OPTION_FIRMPROT]) {
        if (patch_firmprot()) {
            abort("Fatal. Firmprot has failed.");
        }

	    wait();
    }

    // Replace loader?
    if (config.options[OPTION_LOADER]) {
        if (patch_modules()) {
            abort("Fatal. Service patch has failed.");
        }
        // This requires OPTION_SIGPATCH.
	    wait();
    }

    // Inject services?
    if (config.options[OPTION_SERVICES]) {
        if (patch_services()) {
            abort("Fatal. Service patch has failed.");
        }
	    wait();
    }

    // Use ARM9 hook thread?
    if (config.options[OPTION_ARM9THREAD]) {
        // Yes.

        // FIXME - NYI
		wait();
    }

	if (config.options[OPTION_AADOWNGRADE]) {
        if (patch_aadowngrade()) {
            abort("Anti-anti-downgrade patch failed.");
        }

		wait();
	}

    return 0;
}
