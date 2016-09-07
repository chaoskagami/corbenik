#include <common.h>
#include <stdint.h>

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
patch_firm_all(uint64_t tid, firm_h* firm, const char* module_path)
{
    int exit = 0;

    execb(tid, firm);

    switch (tid) {
        case 0x0004013800000002LLu: // NFIRM
        case 0x0004013820000002LLu:
            // Hook firmlaunch?
            if (get_opt_u32(OPTION_REBOOT))
                patch_reboot(firm);

            // Use EmuNAND?
            if (get_opt_u32(OPTION_EMUNAND))
                patch_emunand(firm, get_opt_u32(OPTION_EMUNAND_INDEX));

            // Inject services?
            if (get_opt_u32(OPTION_SVCS))
                if (patch_svc_calls(firm))
                    exit |= 2;
            break;
        case 0x0004013800000102LLu:
        case 0x0004013820000102LLu:
            break;
        case 0x0004013800000202LLu:
        case 0x0004013820000202LLu:
            break;
        default:
            exit |= 4;
            break;
    }

    // Replace loader?
    if (get_opt_u32(OPTION_LOADER))
        if (patch_modules(firm, module_path))
            exit |= 1;

    return 0;
}
