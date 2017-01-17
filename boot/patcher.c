#include <stdint.h>            // for uint8_t, uint64_t
#include <string.h>            // for memcmp
#include <firm/headers.h>      // for firm_h
#include <input.h>             // for wait
#include <interp.h>            // for cache_patch, execb
#include <option.h>            // for get_opt_u32, OPTION_EMUNAND, OPTION_EM...
#include <patch/emunand.h>     // for patch_emunand
#include <patcher.h>           // for patch_modules, patch_reboot, patch_svc...
#include <std/abort.h>         // for panic
#include <std/fs.h>            // for recurse_call, read_file, rrmdir
#include <structures.h>        // for system_patch, PATH_TEMP, PATH_AUX_PATCHES
#include "ctr9/io/fatfs/ff.h"  // for f_mkdir, FILINFO, f_stat, ::FR_OK, AM_DIR

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
generate_patch_cache(void)
{
    // Remove cache
    rrmdir(PATH_TEMP);
    f_mkdir(PATH_TEMP);
    f_mkdir(PATH_LOADER_CACHE);

    recurse_call(PATH_PATCHES,     patch_cache_func);
    recurse_call(PATH_AUX_PATCHES, patch_cache_func);

    return 0;
}

int
patch_firm_all(uint64_t tid, firm_h** firm, const char* module_path)
{
    int exit = 0;

    execb(tid, *firm);

    switch (tid) {
        case 0x0004013800000002LLu: // NFIRM
        case 0x0004013820000002LLu:
            // Hook firmlaunch?
            if (get_opt_u32(OPTION_REBOOT))
                patch_reboot(*firm);

            // Use EmuNAND?
            if (get_opt_u32(OPTION_EMUNAND))
                patch_emunand(*firm, get_opt_u32(OPTION_EMUNAND_INDEX));

            // Inject services?
            if (get_opt_u32(OPTION_SVCS))
                if (patch_svc_calls(*firm))
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
