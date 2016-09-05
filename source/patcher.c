#include <common.h>
#include <stdint.h>

// TODO - Basically all this needs to move to patcher programs.

extern int patch_svc_calls(firm_h*);
extern int patch_modules(firm_h*);
extern int patch_reboot(firm_h*);

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
            if (cache_patch(fpath)) {
                panic("Failed to cache:\n  %s\n", fpath);
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
patch_firm_all(uint64_t tid, firm_h* firm)
{
    execb(tid, firm);

    fprintf(stderr, "VM exited without issue\n");

    // Hook firmlaunch?
    if (get_opt_u32(OPTION_REBOOT)) {
        patch_reboot(firm);

        wait();
    }

    // Use EmuNAND?
    if (get_opt_u32(OPTION_EMUNAND)) {
        // Yes.
        patch_emunand(firm, get_opt_u32(OPTION_EMUNAND_INDEX));

        wait();
    }

    // Inject services?
    if (get_opt_u32(OPTION_SVCS)) {
        if (patch_svc_calls(firm)) {
            panic("Fatal. Svc inject has failed.");
        }
        wait();
    }

    // Replace loader?
    if (get_opt_u32(OPTION_LOADER)) {
        if (patch_modules(firm)) {
            panic("Fatal. Loader inject has failed.");
        }
        // This requires OPTION_SIGPATCH.
        wait();
    }

    return 0;
}
