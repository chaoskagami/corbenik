#include "patch_file.h"

// Do you like examples?

/* In bytecode assembly:

  aadowngrade:
    rel firm_mem
    mov4 r1, pattern
    mov4 r2, 6
    call memfind
    jmpz notfound

  found:
    add r1, 5
    mov1 [r1], 0xE0
    mov4 r1, 0
    return

  notfound:
    mov4 r1, 1
    return

  pattern:
    .byte 0x89
    .byte 0x0A
    .byte 0x81
    .byte 0x42
    .byte 0x02
    .byte 0xD2
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
