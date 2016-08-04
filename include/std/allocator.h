#ifndef __FCRAM_H
#define __FCRAM_H

#include <stdint.h>
#include <stddef.h>

// Space between most of the locations (remove me)
#define FCRAM_SPACING 0x100000

// Grow program break
void *sbrk(size_t bytes);

// Allocate memory for use.
void *malloc   (size_t size);

// Free in-use memory.
void  free     (void* ptr);

#endif
