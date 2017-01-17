#include <stdint.h>        // for uint32_t, uint8_t
#include <stddef.h>         // for fprintf, NULL, FILE
#include "firm/headers.h"  // for firm_section_h, firm_h
#include "std/draw.h"      // for stderr
#include "std/fs.h"        // for crclose, cropen, crread, crsize
#include "std/memory.h"    // for memfind
#include "structures.h"    // for PATH_BACKDOOR

uint8_t *arm11Section1 = NULL;
uint32_t *svc_tab_open = NULL, *exceptionsPage = NULL, *svcTable = NULL;
int svc_offs_init = 0;

// This code handles restoration of backdoor

int
patch_svc_calls(firm_h* firm_loc)
{
    if (svc_offs_init == 0) {
        arm11Section1 = (uint8_t *)firm_loc + firm_loc->section[1].offset;

        uint8_t pattern[] = { 0x00, 0xB0, 0x9C, 0xE5 }; // cpsid aif

        exceptionsPage = (uint32_t *)memfind(arm11Section1, firm_loc->section[1].size, pattern, 4) - 0xB;

        uint32_t svcOffset = (-(((exceptionsPage)[2] & 0xFFFFFF) << 2) & (0xFFFFFF << 2)) - 8; // Branch offset + 8 for prefetch
        svcTable = (uint32_t *)(arm11Section1 + *(uint32_t *)(arm11Section1 + 0xFFFF0008 - svcOffset - 0xFFF00000 + 8) - 0xFFF00000); // SVC handler address
        while (*svcTable)
            svcTable++; // Look for SVC0 (NULL)

        // Skip to free space
        for (svc_tab_open = exceptionsPage; *svc_tab_open != 0xFFFFFFFF; svc_tab_open++);
        svc_offs_init = 1;
    }

    // Make sure svcBackdoor is there.
    if (!svcTable[0x7B]) {
        fprintf(stderr, "svc: 0x7B (backdoor) missing.\n");

        FILE *data = cropen(PATH_BACKDOOR, "r");
        uint32_t size = crsize(data);

        fprintf(stderr, "Svc: backdoor is %lu bytes\n", size);
        fprintf(stderr, "Svc: Read code to %lx\n", (uint32_t)svc_tab_open);

        crread(svc_tab_open, 1, size, data);

        crclose(data);

        //        memcpy(svc_tab_open, svcbackdoor, sizeof(svcbackdoor));
        svcTable[0x7B] = 0xFFFF0000 + (uint32_t)((uint8_t *)svc_tab_open - (uint8_t *)exceptionsPage);

        svc_tab_open += size;

        fprintf(stderr, "svc: Injected 0x7B.\n");
    } else {
        fprintf(stderr, "svc: No change needed.\n");
    }

    return 0;
}
