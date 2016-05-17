#include <stdint.h>
#include "../std/unused.h"
#include "../std/memory.h"
#include "../firm/firm.h"
#include "../firm/fcram.h"
#include "../config.h"
#include "../common.h"

int patch_modules() {
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
	firm_section_h *sysmodule_section = NULL;
	for (firm_section_h *section = firm_loc->section; section < firm_loc->section + 4; section++) {
		if (section->address == 0x1FF00000 && section->type == FIRM_TYPE_ARM11) {
			sysmodule_section = section;
			break;
		}
	}

	if (!sysmodule_section) {
		fprintf(stderr, "Module: sysmodule section not found\n");
		return 1;
	}

	ncch_h *module = (ncch_h*)FCRAM_JUNK_LOCATION;
	ncch_h *sysmodule = (ncch_h *)((uint32_t)firm_loc + sysmodule_section->offset);

	// Check if we want to replace an existing sysmodule
	while (sysmodule->magic == NCCH_MAGIC) {
		if (memcmp(sysmodule->programID, module->programID, 8) == 0) {
			// Expand firmware module size if needed to accomodate replacement.
			if (module->contentSize > sysmodule->contentSize) {
				fprintf(stderr, "Module: Grow %d units\n", module->contentSize - sysmodule->contentSize);
				// Location to shuffle to.
				uint32_t need_units = (module->contentSize - sysmodule->contentSize);
				uint8_t* move_to = ((uint8_t*)sysmodule + (module->contentSize + need_units) * 0x200);
				uint8_t* move_from = ((uint8_t*)sysmodule + module->contentSize * 0x200);

				uint32_t copy_size = 0x10000; // FIXME - Add a safe way to properly calculate the half of NCCH we need to copy. This is okay for now.

				memmove(move_to, move_from, copy_size);
				// TODO - This is hackish and possibly incorrect. It needs testing.
			}

			// Move the remaining modules closer
			if (module->contentSize < sysmodule->contentSize) {
				fprintf(stderr, "Module: Shrink %d units\n", sysmodule->contentSize - module->contentSize);
				int remaining_size =
					sysmodule_section->size -
                    (((uint32_t)sysmodule + sysmodule->contentSize * 0x200) -
						((uint32_t)firm_loc + sysmodule_section->offset));
				// Sysmodule section size - (End location of this sysmodule - Sysmodule section) =>
				memmove((uint8_t*)sysmodule + module->contentSize * 0x200, (uint8_t*)sysmodule + sysmodule->contentSize * 0x200, remaining_size);
				// Move end of section to be adjacent
			}

			fprintf(stderr, "Module: Injecting %llu\n", module->programID);
			// Copy the module into the firm
			memcpy(sysmodule, module, module->contentSize * 0x200);
			break;
		}
		sysmodule = (ncch_h *)((uintptr_t)sysmodule + sysmodule->contentSize * 0x200);
	}

	fprintf(stderr, "Module: injected modules.\n");

	return 0;
}

