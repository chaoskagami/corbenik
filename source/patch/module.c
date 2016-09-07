#include <common.h>

/* Not possible to be implemented as bytecode. Hey, can't win em all. */

firm_h* firm_modules;

void
inject_module(char* fpath)
{
    FILINFO f2;
    if (f_stat(fpath, &f2) != FR_OK)
        return;

    if (f2.fattrib & AM_DIR)
        return;

    // TODO - load other module cxis here
    FILE *f = fopen(fpath, "r");
    if (!f) {
        fprintf(stderr, "Module: %s not found\n", fpath);
        return;
    }

    size_t size = fsize(f);
    uint8_t* temp = malloc(size);
    fread(temp, 1, size, f);
    fclose(f);

    int section_index = 0;
    firm_section_h *sysmodule_section = &firm_modules->section[0];

    ncch_h *module = (ncch_h *)temp;
    ncch_h *sysmodule = (ncch_h *)((uint32_t)firm_modules + sysmodule_section->offset);

    // Check if we want to replace an existing sysmodule
    while (sysmodule->magic == NCCH_MAGIC) {
        if (memcmp(sysmodule->programID, module->programID, 8) == 0) {
            // Expand firmware module size if needed to accomodate replacement.
            if (module->contentSize > sysmodule->contentSize) {
                uint32_t need_units = (module->contentSize - sysmodule->contentSize);

                // FIXME - We're potentially corrupting memory here depending on whether we go over the theoretical maximum size of FIRM

                // SUPER FIXME 9000: DO NOT LEAVE AS IS.
                memmove((uint8_t *)sysmodule + module->contentSize * 0x200, (uint8_t *)sysmodule + sysmodule->contentSize * 0x200,
                        ((uint32_t)firm_modules + 0x100000) - ((uint32_t)sysmodule + (module->contentSize * 0x200)));

                sysmodule_section->size += 0x200 * need_units;
                for (int i = 1; i < 4; i++) {
                    if (firm_modules->section[i].size != 0) { // The last section (3) is usually empty.
                        firm_modules->section[i].offset += 0x200 * need_units;
                        firm_modules->section[i].size += 0x200 * need_units;
                    }
                }

                fprintf(stderr, "Module: Grow %lu units\n", need_units);
            }

            // Move the remaining modules closer
            else if (module->contentSize < sysmodule->contentSize) {
                // NOTE - This doesn't change the sysmodule section size; it isn't needed to do so.
                fprintf(stderr, "Module: Shrink %lu units\n", sysmodule->contentSize - module->contentSize);
                uint32_t remaining_size =
                    sysmodule_section->size - (((uint32_t)sysmodule + sysmodule->contentSize * 0x200) - ((uint32_t)firm_modules + sysmodule_section->offset));
                // Sysmodule section size - (End location of this sysmodule -
                // Sysmodule section) =>
                memmove((uint8_t *)sysmodule + module->contentSize * 0x200, (uint8_t *)sysmodule + sysmodule->contentSize * 0x200, remaining_size);
                // Move end of section to be adjacent
            }

            // Copy the module into the firm
            memcpy(sysmodule, module, module->contentSize * 0x200);

            fprintf(stderr, "Module: injected %s\n", fpath);

            goto end_inj;
        }
        sysmodule = (ncch_h *)((uint32_t)sysmodule + sysmodule->contentSize * 0x200);
    }

    fprintf(stderr, "Module: Failed to inject %s\n", fpath);

end_inj:

    free(temp);
    return;
}

int
patch_modules(firm_h* firm_loc, const char* module_path)
{
    firm_modules = firm_loc;
    recurse_call(module_path, inject_module);

    return 0;
}

