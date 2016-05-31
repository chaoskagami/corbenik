#ifndef __FCRAM_H
#define __FCRAM_H

// File to keep track of all the fcram offsets in use.
// It provides an easy overview of all that is used.

#include <stdint.h>

#include "../std/unused.h"

extern void* fcram_temp;

// Space between most of the locations
#define FCRAM_SPACING 0x100000

// Start of the space we use
#define FCRAM_START 0x24000000

// firm.c
// 24
#define FCRAM_FIRM_LOC FCRAM_START
// 241
#define FCRAM_TWL_FIRM_LOC (FCRAM_START + FCRAM_SPACING) // Double size
// 242
#define FCRAM_AGB_FIRM_LOC (FCRAM_START + FCRAM_SPACING * 3)

// 243
// patch.c
#define FCRAM_PATCHBIN_EXEC_LOC (FCRAM_START + FCRAM_SPACING * 4)

// 244
// Throwaway temporary space. Don't expect it to stay sane.
#define FCRAM_JUNK_LOCATION (FCRAM_START + FCRAM_SPACING * 5)

#endif
