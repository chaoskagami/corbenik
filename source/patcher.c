#include <stdint.h>
#include "std/unused.h"
#include "std/memory.h"
#include "firm/firm.h"
#include "config.h"
#include "common.h"
#include "interp.h"
#include "patch/emunand.h"

// TODO - Basically all this needs to move to patcher programs.

uint32_t wait_key();

extern int patch_services();
extern int patch_modules();
extern int patch_reboot();

extern int doing_autoboot;

extern uint8_t *enable_list;

void
wait()
{
    if (config.options[OPTION_TRACE] && !doing_autoboot) {
        fprintf(stderr, "                                 [WAIT]");
        wait_key();
    }
    fprintf(stderr, "\r                                       \r");
}

void list_patches_build(char *name, int desc_is_fname);

int
generate_patch_cache()
{
    // Remove cache
    rrmdir(PATH_LOADER_CACHE);
    f_mkdir(PATH_LOADER_CACHE);

    list_patches_build(PATH_PATCHES, 1);

    struct options_s *patches = (struct options_s *)FCRAM_MENU_LOC;

    for (int i = 0; patches[i].index != -1; i++) {
        if (enable_list[patches[i].index]) {
            // Patch is enabled. Cache it.
            if (execb(patches[i].desc, 1)) {
                abort("Failed to apply:\n  %s\n", patches[i].name);
            }

            wait();
        }
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
            abort("Fatal. Loader inject has failed.");
        }
        // This requires OPTION_SIGPATCH.
        wait();
    }

    // Hook firmlaunch?
    if (config.options[OPTION_REBOOT]) {
        patch_reboot();

        wait();
    }

    // Inject services?
    if (config.options[OPTION_SVCS]) {
        if (patch_services()) {
            abort("Fatal. Svc inject has failed.");
        }
        wait();
    }

    // Use ARM9 hook thread?
    if (config.options[OPTION_ARM9THREAD]) {
        // Yes.

        // FIXME - NYI
        wait();
    }

    // Use EmuNAND?
    if (config.options[OPTION_EMUNAND]) {
        // Yes.
        patch_emunand(config.options[OPTION_EMUNAND_INDEX]);

        wait();
    }

    return 0;
}
