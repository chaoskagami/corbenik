#include <common.h>

/* Not possible to be implemented as bytecode. Hey, can't win em all. */

int
patch_modules()
{
    // TODO - load other module cxis here
    FILE *f = fopen(PATH_MODULES "/loader.cxi", "r");
    if (!f) {
        fprintf(stderr, "Module: loader.cxi not found on FS\n");
        return 2;
    }

    size_t size = fsize(f);
    uint8_t* temp = malloc(size);
    fread(temp, 1, size, f);
    fclose(f);

    int section_index = 0;
    firm_section_h *sysmodule_section = &firm_loc->section[0];

    ncch_h *module = (ncch_h *)temp;
    ncch_h *sysmodule = (ncch_h *)((uint32_t)firm_loc + sysmodule_section->offset);

    // Check if we want to replace an existing sysmodule
    while (sysmodule->magic == NCCH_MAGIC) {
        if (memcmp(sysmodule->programID, module->programID, 8) == 0) {
            // Expand firmware module size if needed to accomodate replacement.
            if (module->contentSize > sysmodule->contentSize) {
                uint32_t need_units = (module->contentSize - sysmodule->contentSize);

                memmove((uint8_t *)sysmodule + module->contentSize * 0x200, (uint8_t *)sysmodule + sysmodule->contentSize * 0x200,
                        ((uint32_t)firm_loc + firm_size) - ((uint32_t)sysmodule + (module->contentSize * 0x200)));

                sysmodule_section->size += 0x200 * need_units;
                for (int i = 1; i < 4; i++) {
                    if (firm_loc->section[i].size != 0) { // The last section (3) is usually empty.
                        firm_loc->section[i].offset += 0x200 * need_units;
                        firm_loc->section[i].size += 0x200 * need_units;
                    }
                }

                fprintf(stderr, "module: Grow %lu units\n", need_units);
            }

            // Move the remaining modules closer
            else if (module->contentSize < sysmodule->contentSize) {
                // NOTE - This doesn't change the sysmodule section size; it isn't needed to do so.
                fprintf(stderr, "Module: Shrink %lu units\n", sysmodule->contentSize - module->contentSize);
                uint32_t remaining_size =
                    sysmodule_section->size - (((uint32_t)sysmodule + sysmodule->contentSize * 0x200) - ((uint32_t)firm_loc + sysmodule_section->offset));
                // Sysmodule section size - (End location of this sysmodule -
                // Sysmodule section) =>
                memmove((uint8_t *)sysmodule + module->contentSize * 0x200, (uint8_t *)sysmodule + sysmodule->contentSize * 0x200, remaining_size);
                // Move end of section to be adjacent
            }

            fprintf(stderr, "Module: Injecting module\n");
            // Copy the module into the firm
            memcpy(sysmodule, module, module->contentSize * 0x200);
        }
        sysmodule = (ncch_h *)((uint32_t)sysmodule + sysmodule->contentSize * 0x200);
    }

    fprintf(stderr, "Module: injected modules.\n");

    free(temp);

    return 0;
}
