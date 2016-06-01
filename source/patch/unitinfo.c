#include "patch_file.h"

// This patch makes the console think it is a developer unit.
// Note that this is generally invasive and not useful to users;
// usually the ErrDisp patch in loader should be good enough for
// debugging crashes.

PATCH(unitinfo)
{
	firm_section_h* arm9_section;
	int found_sect = 0;

	for (arm9_section = firm_loc->section;
         arm9_section < firm_loc->section + 4; arm9_section++) {
        if (arm9_section->type == FIRM_TYPE_ARM9) {
			found_sect = 1;
			break;
		}
	}

	if (!found_sect) {
		fprintf(stderr, "unitinfo: no arm9 section?\n");
		return 1;
	}

    uint8_t* firm_mem = (uint8_t*)firm_loc + arm9_section->offset;
    uint32_t size = arm9_section->size;

	const uint8_t pattern[] = {0x01, 0x10, 0xA0, 0x13};

    // We look for 'exe:' first; this string is close to what we patch
    uint8_t* off = memfind(firm_mem, size, pattern, 4);

    if (off == NULL) {
        fprintf(stderr, "unitinfo: Couldn't find UNITINFO.\n");
        return 1;
    }

	off[3] = 0xE3;

    fprintf(stderr, "unitinfo: Applied\n");

    return 0;
}
