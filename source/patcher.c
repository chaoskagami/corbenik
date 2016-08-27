#include <common.h>
#include <stdint.h>

// TODO - Basically all this needs to move to patcher programs.

extern int patch_services();
extern int patch_modules();
extern int patch_reboot();

extern int doing_autoboot;

extern uint8_t *enable_list;

extern struct options_s* patches;

void
patch_cache_func(char* fpath)
{
    FILINFO f2;
    if (f_stat(fpath, &f2) != FR_OK)
        return;

    if (!(f2.fattrib & AM_DIR)) {
        struct system_patch p;
        read_file(&p, fpath, sizeof(struct system_patch));

        if (memcmp(p.magic, "AIDA", 4))
            return;

        if (enable_list[p.uuid]) {
            // Patch is enabled. Cache it.
            if (execb(fpath, 1)) {
                abort("Failed to cache:\n  %s\n", fpath);
            }

            wait();
        }
    }
}

int
generate_patch_cache()
{
    // Remove cache
    rrmdir(PATH_LOADER_CACHE);
    f_mkdir(PATH_LOADER_CACHE);

    recurse_call(PATH_PATCHES, patch_cache_func);

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
