#ifndef __FCRAM_H
#define __FCRAM_H

// File to keep track of all the fcram offsets in use.
// It provides an easy overview of all that is used.

#include <stdint.h>
#include <stddef.h>

extern void *fcram_temp;

// Space between most of the locations
#define FCRAM_SPACING 0x100000

// Start of the space we use
#define FCRAM_START 0x24000000

// firm.c
#define FCRAM_FIRM_LOC FCRAM_START
#define FCRAM_TWL_FIRM_LOC (FCRAM_FIRM_LOC + FCRAM_SPACING) // Double size
#define FCRAM_AGB_FIRM_LOC (FCRAM_TWL_FIRM_LOC + FCRAM_SPACING * 2)

// Throwaway temporary space. Don't expect it to stay sane.
#define FCRAM_JUNK_LOC FCRAM_START

// Location to perform static allocations at.
#define FCRAM_STATIC_ALLOC_LOC (FCRAM_START + FCRAM_SPACING)

// Grow memory segment.
void *fake_sbrk(size_t bytes);

// Allocate memory for use.
void *malloc   (size_t size);

// Free in-use memory.
void  free     (void* ptr);

#endif
