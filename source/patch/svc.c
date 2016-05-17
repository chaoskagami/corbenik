#include <stdint.h>
#include "../std/unused.h"
#include "../std/memory.h"
#include "../firm/firm.h"
#include "../config.h"
#include "../common.h"

uint32_t *getSvcAndExceptions(uint8_t *pos, uint32_t size, uint32_t **exceptionsPage) {
    uint8_t pattern[] = {0x00, 0xB0, 0x9C, 0xE5}; //cpsid aif

    *exceptionsPage = (uint32_t *)memfind(pos, size, pattern, 4) - 0xB;

    uint32_t svcOffset = (-(((*exceptionsPage)[2] & 0xFFFFFF) << 2) & (0xFFFFFF << 2)) - 8; //Branch offset + 8 for prefetch
    uint32_t *svcTable = (uint32_t *)(pos + *(uint32_t *)(pos + 0xFFFF0008 - svcOffset - 0xFFF00000 + 8) - 0xFFF00000); //SVC handler address
    while(*svcTable) svcTable++; //Look for SVC0 (NULL)

    return svcTable;
}

uint32_t *freeSpace = NULL;

int patch_services() {
    // Make sure svcBackdoor is there.
    uint8_t* arm11Section1 = (uint8_t*)firm_loc + firm_loc->section[1].offset;
    uint32_t *exceptionsPage;
    uint32_t *svcTable = getSvcAndExceptions(arm11Section1, firm_loc->section[1].size, &exceptionsPage);

	fprintf(stderr, "Svc: table at %x\n", (uint32_t)svcTable);

    if(!svcTable[0x7B]) {
        // Firmware is missing svcBackdoor. Fix it.
        fprintf(stderr, "Svc: inject 0x7B (backdoor)\n");

        // See extra/backdoor.s for the code to this.
        const unsigned char svcbackdoor[40] = {
            0xFF, 0x10, 0xCD, 0xE3, 0x0F, 0x1C, 0x81, 0xE3, 0x28, 0x10, 0x81, 0xE2, 0x00, 0x20, 0x91, 0xE5,
            0x00, 0x60, 0x22, 0xE9, 0x02, 0xD0, 0xA0, 0xE1, 0x30, 0xFF, 0x2F, 0xE1, 0x03, 0x00, 0xBD, 0xE8,
            0x00, 0xD0, 0xA0, 0xE1, 0x11, 0xFF, 0x2F, 0xE1
        };

		if (!freeSpace) {
			for(freeSpace = exceptionsPage; *freeSpace != 0xFFFFFFFF; freeSpace++);
		}

		fprintf(stderr, "Svc: Copy code to %x\n", (uint32_t)freeSpace);

        memcpy(freeSpace, svcbackdoor, sizeof(svcbackdoor));
        svcTable[0x7B] = 0xFFFF0000 + ((uint8_t *)freeSpace - (uint8_t *)exceptionsPage);

		freeSpace += sizeof(svcbackdoor); // We keep track of this because there's more than 7B free.

		fprintf(stderr, "Svc: entry set as %x\n", svcTable[0x7B]);
    } else {
        fprintf(stderr, "Svc: no change\n");
	}

	return 0;
}
