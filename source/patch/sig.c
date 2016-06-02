#include "patch_file.h"

// This patch is responsible for fixing signature checks for the firmware.

/*
  rel  p9_exefs
  find 4, 0xc0, 0x1c, 0x76, 0xe7
  set  2, 0x00, 0x20
  rewind
  find 4, 0xb5, 0x22, 0x4d, 0x0c
  set  4, 0x00, 0x20, 0x70, 0x47
 */

extern exefs_h* firm_p9_exefs;

PATCH(signatures)
{
/*
    // Look for signature checks

    uint8_t pat1[] = { 0xC0, 0x1C, 0x76, 0xE7 };
    uint8_t pat2[] = { 0xB5, 0x22, 0x4D, 0x0C };

    // The code segment.
    uint8_t* firm_mem = (uint8_t*)firm_p9_exefs + sizeof(exefs_h) +
                        firm_p9_exefs->fileHeaders[0].offset;
    uint32_t size = firm_p9_exefs->fileHeaders[0].size;

    uint8_t* off = memfind(firm_mem, size, pat1, 4);

    // We're subbing one because the code goes back 1.
    // Unique patterns, etc.
    uint8_t* off2 = memfind(firm_mem, size, pat2, 4) - 1;

    if (off == NULL) {
        fprintf(stderr, "Signature patch failed on P0.\n");
        return 1; // Failed to find sigpatch. Ugh.
    }

    if (off2 == NULL) {
        fprintf(stderr, "Signature patch failed on P1.\n");
        return 2; // Failed to find sigpatch. Ugh.
    }

    fprintf(stderr, "Signatures[0]: 0x%x\n", (uint32_t)off);
    fprintf(stderr, "Signatures[1]: 0x%x\n", (uint32_t)off2);

    // See asm/sigpatches.s for the code here
    uint8_t sigpatch[] = { 0x00, 0x20, 0x70, 0x47 };

    memcpy(off, sigpatch, 2);
    memcpy(off2, sigpatch, 4);

    fprintf(stderr, "Signature patch succeded.\n");
*/
	execb(PATH_PATCHES "/sig.vco");

    return 0;
}
