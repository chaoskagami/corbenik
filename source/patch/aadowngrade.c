#include "patch_file.h"

/*
   rel p9_exefs
   find 6, 0x89, 0x0a, 0x81, 0x42, 0x02, 0xD2
   fwd 5
   set 1, 0xE0
 */

PATCH(aadowngrade)
{
	exefs_h* firm_p9_exefs = get_firm_proc9_exefs();

    uint8_t* firm_mem = (uint8_t*)firm_p9_exefs + sizeof(exefs_h) +
                        firm_p9_exefs->fileHeaders[0].offset;
    uint32_t size = firm_p9_exefs->fileHeaders[0].size;

	const uint8_t pattern[] = {0x89, 0x0A, 0x81, 0x42, 0x02, 0xD2};

	uint8_t *off = memfind(firm_mem, size, pattern, 6);

	if(off == NULL)
		return 1; // Not found.

	fprintf(stderr, "aadowngrade: %x\n", (uint32_t)off);

	off[5] = 0xE0;

	return 0;
}
