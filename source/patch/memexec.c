#include "patch_file.h"

// This patch clears MPU settings which lock down memory
// execution from userland. You should NOT enable this
// unless you know you need it, because it makes an obvious
// behavioral change that can be used maliciously and/or to
// detect CFW use rather easily.

PATCH(memexec)
{
	firm_section_h* arm11_section = & firm_loc->section[1]; // Section 1, please.

    uint8_t* firm_mem = (uint8_t*)firm_loc + arm11_section->offset;
    uint32_t size = arm11_section->size;

	const uint8_t pattern[] = {0x97, 0x05, 0x00, 0x00, 0x15, 0xE4, 0x00, 0x00};

    // We look for 'exe:' first; this string is close to what we patch
    uint32_t* off = (uint32_t*)memfind(firm_mem, size, pattern, 8);

    if (off == NULL) {
        fprintf(stderr, "memexec: couldn't find pattern.\n");
        return 1;
    }

	// NOTE - Luma3DS' check here was incoherent.
	// It read as 'decrement until 0x00000000, which means on
	// failure it would cause lockup due to read of unreadable mapped areas.
	// Even worse, it could end up patching something that isn't in FCRAM.
	while(off > (uint32_t*)firm_mem && *off != 0x16416)
		off--;

	if(off == (uint32_t*)firm_mem) {
        fprintf(stderr, "memexec: beginning missing.\n");
        return 1;
	}

	*off &= ~(1 << 4); //Clear XN bit

    fprintf(stderr, "memexec: Cleared XN bit.\n");

    return 0;
}
