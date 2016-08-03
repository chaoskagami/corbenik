#include <common.h>
#include <stdint.h>

// TODO - Basically all this needs to move to patcher programs.

uint32_t wait_key(int sleep);

extern int patch_services();
extern int patch_modules();
extern int patch_reboot();

extern int doing_autoboot;

extern uint8_t *enable_list;

extern struct options_s* patches;

void
wait()
{
    if (config->options[OPTION_TRACE] && !doing_autoboot) {
        fprintf(stderr, "[Waiting...]");
        wait_key(0); // No delay on traces.
    }
    fprintf(stderr, "            \r");
}

void list_patches_build(char *name, int desc_is_fname);

int
generate_patch_cache()
{
    // Remove cache
    rrmdir(PATH_LOADER_CACHE);
    f_mkdir(PATH_LOADER_CACHE);

    list_patches_build(PATH_PATCHES, 1);

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

    fprintf(stderr, "VM exited without issue\n");

    // Hook firmlaunch?
    if (config->options[OPTION_REBOOT]) {
        patch_reboot();

        wait();
    }

    // Use EmuNAND?
    if (config->options[OPTION_EMUNAND]) {
        // Yes.
        patch_emunand(config->options[OPTION_EMUNAND_INDEX]);

        wait();
    }

    // Inject services?
    if (config->options[OPTION_SVCS]) {
        if (patch_services()) {
            abort("Fatal. Svc inject has failed.");
        }
        wait();
    }

    // Replace loader?
    if (config->options[OPTION_LOADER]) {
        if (patch_modules()) {
            abort("Fatal. Loader inject has failed.");
        }
        // This requires OPTION_SIGPATCH.
        wait();
    }

    // Use ARM9 hook thread?
    if (config->options[OPTION_ARM9THREAD]) {
        // Yes.

        // FIXME - NYI
        wait();
    }

    return 0;
}
