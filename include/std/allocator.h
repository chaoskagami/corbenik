#ifndef __FCRAM_H
#define __FCRAM_H

#include <stdint.h>
#include <stddef.h>

#define FCRAM_SPACING 0x100000 ///< Space between most of the locations (remove me)

/* Grow program break.
 *
 * \param bytes Number of bytes to grow by.
 */
void *sbrk(size_t bytes);

/* Allocate memory for use.
 *
 * \param size Size in bytes to allocate.
 */
void *malloc   (size_t size);

/* Free in-use memory allocated by malloc.
 *
 * \param ptr Pointer to free.
 */
void  free     (void* ptr);

#endif
