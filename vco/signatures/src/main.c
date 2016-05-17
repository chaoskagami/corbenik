#include <stdint.h>
#include <stddef.h>
#include "exported.h"

uint8_t pat1[] = {0xC0, 0x1C, 0x76, 0xE7};
uint8_t pat2[] = {0xB5, 0x22, 0x4D, 0x0C};

// See asm/sigpatches.s for the code here
uint8_t sigpatch[] = {0x00, 0x20, 0x70, 0x47};

uint8_t *firm_mem, *off, *off2;
uint32_t size = 0;
exefs_h* firm_p9_exefs;

int main() {
    //Look for signature checks
	fprintf(stderr, "Applying signature patch\n");

	firm_p9_exefs = get_firm_proc9_exefs();

	fprintf(stderr, "ExeFS: 0x%x\n", firm_p9_exefs);

	// The code segment.
	firm_mem = (uint8_t*)firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
	size     = firm_p9_exefs->fileHeaders[0].size;

	fprintf(stderr, "Search, P0.\n");
    off  = memfind(firm_mem, size, pat1, 4);

	if (off == NULL) {
		fprintf(stderr, "Failed, P0.\n");
		return 1; // Failed to find sigpatch. Ugh.
	}

	// We're subbing one because the code goes back 1.
	// Unique patterns, etc.
	fprintf(stderr, "Search, P1.\n");
    off2 = memfind(firm_mem, size, pat2, 4) - 1;

	if (off2 == NULL) {
		fprintf(stderr, "Failed, P1.\n");
		return 2; // Failed to find sigpatch. Ugh.
	}

	fprintf(stderr, "Signatures, P0, 0x%x\n", (uint32_t)off);
	memcpy(off,  sigpatch, 2);

	fprintf(stderr, "Signatures, P1, 0x%x\n", (uint32_t)off2);
	memcpy(off2, sigpatch, 4);

	fprintf(stderr, "Succeded.\n");

	return 0;
}
