#ifndef __FCRAM_H
#define __FCRAM_H

// File to keep track of all the fcram offsets in use.
// It provides an easy overview of all that is used.

#include <stdint.h>

#include "../std/unused.h"

extern void *fcram_temp;

// Space between most of the locations
#define FCRAM_SPACING 0x100000

// Start of the space we use
#define FCRAM_START 0x24000000

// firm.c
#define FCRAM_FIRM_LOC FCRAM_START
#define FCRAM_TWL_FIRM_LOC (FCRAM_FIRM_LOC + FCRAM_SPACING) // Double size
#define FCRAM_AGB_FIRM_LOC (FCRAM_TWL_FIRM_LOC + FCRAM_SPACING * 2)

// Location patches get loaded to.
#define FCRAM_PATCH_LOC (FCRAM_AGB_FIRM_LOC + FCRAM_SPACING * 2)

// Throwaway temporary space. Don't expect it to stay sane.
#define FCRAM_JUNK_LOC (FCRAM_PATCH_LOC + FCRAM_SPACING)

// Path that the patch menu is created at.
#define FCRAM_MENU_LOC (FCRAM_JUNK_LOC + FCRAM_SPACING)

// Path that the patch enable list is located at.
#define FCRAM_PATCHLIST_LOC (FCRAM_MENU_LOC + (FCRAM_SPACING / 2))

// Path that the font will be loaded at.
#define FCRAM_FONT_LOC (FCRAM_PATCHLIST_LOC + (FCRAM_SPACING / 2))

#endif
