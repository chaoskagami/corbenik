#include "patch_file.h"

PATCH(modules)
{
    // TODO - load module cxi here
    FILE* f = fopen(PATH_MODULES "/loader.cxi", "r");
    if (!f) {
        fprintf(stderr, "Module: loader.cxi not found on FS\n");
        return 2;
    }

    uint32_t size = fsize(f);
    fread((uint8_t*)FCRAM_JUNK_LOCATION, 1, size, f);
    fclose(f);

    // Look for the section that holds all the sysmodules
    firm_section_h* sysmodule_section = NULL;
    for (firm_section_h* section = firm_loc->section;
         section < firm_loc->section + 4; section++) {
        if (section->address == 0x1FF00000 &&
            section->type == FIRM_TYPE_ARM11) {
            sysmodule_section = section;
            break;
        }
    }

    if (!sysmodule_section) {
        fprintf(stderr, "Module: sysmodule section not found\n");
        return 1;
    }

    ncch_h* module = (ncch_h*)FCRAM_JUNK_LOCATION;
    ncch_h* sysmodule =
        (ncch_h*)((uint32_t)firm_loc + sysmodule_section->offset);

    // Check if we want to replace an existing sysmodule
    while (sysmodule->magic == NCCH_MAGIC) {
        if (memcmp(sysmodule->programID, module->programID, 8) == 0) {
            // Expand firmware module size if needed to accomodate replacement.
            if (module->contentSize > sysmodule->contentSize) {
                uint32_t need_units =
                    (module->contentSize - sysmodule->contentSize);
                fprintf(stderr, "Module: Would grow %d units but NYI\n",
                        need_units);
                continue;

                // TODO - so in a nutshell, the reason Luma works is because it
                // relocates
                // the sysmodule section to its final location while fixing the
                // size.
                // Once stuff is at the right place, it no longer cares about
                // its' size.

                // Cakes performs an in-place resize and leaves the copy for
                // later; essentially,
                // this means I need to scale EVERYTHING after the increase, and
                // adjust NCCH sections accordingly.

                // It would be hella easier to just patch at the final location
                // in memory;
                // and this was actually mentioned on Luma's issues.
            }

            // Move the remaining modules closer
            if (module->contentSize < sysmodule->contentSize) {
                fprintf(stderr, "Module: Shrink %d units\n",
                        sysmodule->contentSize - module->contentSize);
                int remaining_size =
                    sysmodule_section->size -
                    (((uint32_t)sysmodule + sysmodule->contentSize * 0x200) -
                     ((uint32_t)firm_loc + sysmodule_section->offset));
                // Sysmodule section size - (End location of this sysmodule -
                // Sysmodule section) =>
                memmove((uint8_t*)sysmodule + module->contentSize * 0x200,
                        (uint8_t*)sysmodule + sysmodule->contentSize * 0x200,
                        remaining_size);
                // Move end of section to be adjacent
            }

            fprintf(stderr, "Module: Injecting %llu\n", module->programID);
            // Copy the module into the firm
            memcpy(sysmodule, module, module->contentSize * 0x200);
            break;
        }
        sysmodule =
            (ncch_h*)((uintptr_t)sysmodule + sysmodule->contentSize * 0x200);
    }

    fprintf(stderr, "Module: injected modules.\n");

    return 0;
}
