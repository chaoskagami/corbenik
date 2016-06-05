#include "patch_file.h"

// This code handles replacement of services. This includes backdoor, but not
// just backdoor.
// Any service can be replaced provided there's enough space within the
// exception page.

// Please note that the actual code for services is in `external/service`.

uint32_t *
getSvcAndExceptions(uint8_t *pos, uint32_t size, uint32_t **exceptionsPage)
{
    uint8_t pattern[] = { 0x00, 0xB0, 0x9C, 0xE5 }; // cpsid aif

    *exceptionsPage = (uint32_t *)memfind(pos, size, pattern, 4) - 0xB;

    uint32_t svcOffset = (-(((*exceptionsPage)[2] & 0xFFFFFF) << 2) & (0xFFFFFF << 2)) - 8;                             // Branch offset + 8 for prefetch
    uint32_t *svcTable = (uint32_t *)(pos + *(uint32_t *)(pos + 0xFFFF0008 - svcOffset - 0xFFF00000 + 8) - 0xFFF00000); // SVC handler address
    while (*svcTable)
        svcTable++; // Look for SVC0 (NULL)

    return svcTable;
}

uint32_t *freeSpace = NULL;

PATCH(services)
{
    // Make sure svcBackdoor is there.
    uint8_t *arm11Section1 = (uint8_t *)firm_loc + firm_loc->section[1].offset;
    uint32_t *exceptionsPage;
    uint32_t *svcTable = getSvcAndExceptions(arm11Section1, firm_loc->section[1].size, &exceptionsPage);

    fprintf(stderr, "Svc: table at %x\n", (uint32_t)svcTable);

    char str[] = PATH_SVC "/00.bin";
    char *at = str + (strlen(str) - 6);
    // FIXME - This is really slow. Some way to optimize it?
    for (uint32_t i = 0; i <= 0xf; i++) {
        // Get string for svc.
        at[0] = ("0123456789abcdef")[i];

        for (uint32_t j = 0; j < 0xf; j++) {
            // This is just hexdump. Nothing complicated.
            at[1] = ("0123456789abcdef")[j];

            FILE *data = fopen(str, "r");
            if (!data)
                continue; // No file for svc. Move on.

            uint32_t svc = (i << 4) & j;

            // Refuse to replace non-NULL services unless the user says to.
            if (svcTable[svc] && !config.options[OPTION_REPLACE_ALLOCATED_SVC]) {
                fclose(data);
                fprintf(stderr, "Svc: %x non-null, moving on\n", i);
                continue;
            }

            // TODO - We can just fread directly to freeSpace with a little reordering.

            uint32_t size = fsize(data);
            uint8_t *read_to = (void *)FCRAM_JUNK_LOC;

            fprintf(stderr, "Svc: %s, %d bytes\n", at, size);

            fread(read_to, 1, size, data);

            fclose(data);

            if (!freeSpace) {
                for (freeSpace = exceptionsPage; *freeSpace != 0xFFFFFFFF; freeSpace++)
                    ;
            }

            fprintf(stderr, "Svc: Copy code to %x\n", (uint32_t)freeSpace);

            memcpy(freeSpace, read_to, size);
            svcTable[svc] = 0xFFFF0000 + ((uint8_t *)freeSpace - (uint8_t *)exceptionsPage);

            freeSpace += size; // We keep track of this because there's more than 7B free.

            fprintf(stderr, "Svc: entry set as %x\n", svcTable[svc]);
        }
    }

    return 0;
}
