#include <stdint.h>
#include "../std/unused.h"
#include "../std/memory.h"
#include "../firm/firm.h"
#include "../config.h"
#include "../common.h"

uint8_t* arm11Section1 = NULL;
uint32_t *svc_tab_open = NULL, *exceptionsPage = NULL, *svcTable = NULL;
int svc_offs_init = 0;

int patch_services() {
	if (svc_offs_init == 0) {
    	arm11Section1 = (uint8_t*)firm_loc + firm_loc->section[1].offset;

    	uint8_t pattern[] = {0x00, 0xB0, 0x9C, 0xE5}; //cpsid aif

    	exceptionsPage = (uint32_t *)memfind(arm11Section1, firm_loc->section[1].size, pattern, 4) - 0xB;

    	uint32_t svcOffset = (-(((exceptionsPage)[2] & 0xFFFFFF) << 2) & (0xFFFFFF << 2)) - 8; //Branch offset + 8 for prefetch
    	svcTable = (uint32_t *)(arm11Section1 + *(uint32_t *)(arm11Section1 + 0xFFFF0008 - svcOffset - 0xFFF00000 + 8) - 0xFFF00000); //SVC handler address
    	while(*svcTable) svcTable++; //Look for SVC0 (NULL)

		// Skip to free space
		for(svc_tab_open = exceptionsPage; *svc_tab_open != 0xFFFFFFFF; svc_tab_open++);
		svc_offs_init = 1;
	}

	fprintf(stderr, "Svc Stubs:\n");
	for (int i=0; i < 0xFF; i++) {
		if(!svcTable[i]) {
			fprintf(stderr, "%d ", i);
		}
	}
	fprintf(stderr, "\n");

    // Make sure svcBackdoor is there.
    if(!svcTable[0x7B]) {
		fprintf(stderr, "svc: 0x7B (backdoor) missing.\n");

        // See extra/backdoor.s for the code to this.
        const unsigned char svcbackdoor[40] = {
            0xFF, 0x10, 0xCD, 0xE3, 0x0F, 0x1C, 0x81, 0xE3, 0x28, 0x10, 0x81, 0xE2, 0x00, 0x20, 0x91, 0xE5,
            0x00, 0x60, 0x22, 0xE9, 0x02, 0xD0, 0xA0, 0xE1, 0x30, 0xFF, 0x2F, 0xE1, 0x03, 0x00, 0xBD, 0xE8,
            0x00, 0xD0, 0xA0, 0xE1, 0x11, 0xFF, 0x2F, 0xE1
        };

        memcpy(svc_tab_open, svcbackdoor, sizeof(svcbackdoor));
        svcTable[0x7B] = 0xFFFF0000 + ((uint8_t *)svc_tab_open - (uint8_t *)exceptionsPage);

		svc_tab_open += sizeof(svcbackdoor);

		fprintf(stderr, "svc: Injected 0x7B.\n");
    } else {
		fprintf(stderr, "svc: No change needed.\n");
	}

	return 0;
}
