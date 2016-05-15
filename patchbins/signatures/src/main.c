#include <stdint.h>
#include <stddef.h>
#include "exported.h"

int main() {
    //Look for signature checks
	uint8_t pat1[] = {0xC0, 0x1C, 0x76, 0xE7};
	uint8_t pat2[] = {0xB5, 0x22, 0x4D, 0x0C};

	uint8_t *firm_mem = (uint8_t*)memory_offset;
	uint32_t size = *(uint32_t*)memory_len;

    uint8_t *off  = memfind(firm_mem, size, pat1, 4);
    uint8_t *off2 = memfind(firm_mem, size, pat2, 4) - 1;

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

	uint8_t sigpatch[] = {0x00, 0x20, 0x70, 0x47};

	memcpy(off,  sigpatch, 2);
	memcpy(off2, sigpatch, 4);
	fprintf(stderr, "Signature patch succeded.\n");

	return 0;
}
