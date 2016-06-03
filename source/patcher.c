#include <stdint.h>
#include "std/unused.h"
#include "std/memory.h"
#include "firm/firm.h"
#include "config.h"
#include "common.h"
#include "interp.h"

// TODO - Basically all this needs to move to patcher programs.

uint32_t wait_key();

extern int patch_services();
extern int patch_modules();

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

int generate_patch_cache() {
    // Remove cache
	rrmdir(PATH_LOADER_CACHE);
	f_mkdir(PATH_LOADER_CACHE);

	wait();

    // Loader only uses TID cache bytecode, so run through these.
    execb(PATH_PATCHES "/block_nim_update.vco", 1);
    execb(PATH_PATCHES "/block_eshop_update.vco", 1);
    execb(PATH_PATCHES "/block_cart_update.vco", 1);
    execb(PATH_PATCHES "/errdisp.vco", 1);
    execb(PATH_PATCHES "/friends_ver.vco", 1);
    execb(PATH_PATCHES "/mset_str.vco", 1);
    //	execb(PATH_PATCHES "/ns_force_menu.vco");
    execb(PATH_PATCHES "/regionfree.vco", 1);
    execb(PATH_PATCHES "/secinfo_sigs.vco", 1);
    execb(PATH_PATCHES "/ro_sigs.vco", 1);

	wait();

    // Use builtin signature patcher?
    if (config.options[OPTION_SIGPATCH]) {
        // TODO - Patch menu. This is okay-ish for now.
        if (execb(PATH_PATCHES "/sig.vco", 1)) {
            abort("Fatal. Sigpatch has failed.");
        }

        wait();
    }

    if (config.options[OPTION_FIRMPROT]) {
        if (execb(PATH_PATCHES "/prot.vco", 1)) {
            abort("Fatal. Firmprot has failed.");
        }

        wait();
    }

    if (config.options[OPTION_AADOWNGRADE]) {
        if (execb(PATH_PATCHES "/aadowngrade.vco", 1)) {
            abort("Anti-anti-downgrade patch failed.");
        }

        wait();
    }

    if (config.options[OPTION_UNITINFO]) {
        if (execb(PATH_PATCHES "/unitinfo.vco", 1)) {
            abort("UNITINFO patch failed.");
        }

        wait();
    }

    if (config.options[OPTION_MEMEXEC]) {
        if (execb(PATH_PATCHES "/memexec.vco", 1)) {
            abort("MPU execution patch failed.");
        }

        wait();
    }

	return 0;
}

int
patch_firm_all()
{
	execb(PATH_LOADER_CACHE "/BOOT", 0);

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

    return 0;
}
