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

#ifdef MALLOC_DEBUG

/* Prints stats for allocation to stderr.
 */
void print_alloc_stats(void);

/* Allocate memory for use (debugging only, don't call)
 *
 * \param size Size in bytes to allocate.
 * \param info Info to store about malloc
 */
void* malloc_chkd(size_t size, const char* info);

/* Free in-use memory allocated by malloc (debugging only, don't call)
 *
 * \param ptr Pointer to free.
 * \param info Info to store about free
 */
void  free_chkd(void* ptr, const char* info);

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define malloc(x) malloc_chkd( x , __FILE__ ":" TOSTRING(__LINE__) )
#define free(x)   free_chkd( x , __FILE__ ":" TOSTRING(__LINE__) )

#else

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

/* Reallocates memory to size. Guaranteed to preserve the original data.
 *
 * \param ptr Pointer to reallocate
 * \param size Size to reallocate as
 */
void *realloc(void* ptr, size_t size);

#endif

#endif
