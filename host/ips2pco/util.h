#ifndef __LIBC2PATCHER_OLDVM_UTIL__
#define __LIBC2PATCHER_OLDVM_UTIL__

size_t fsize(FILE* file);

const void* memfind(const void *input, uint32_t size, const void *pattern, uint32_t patternSize);

#define READALLOC_NOFILE  1
#define READALLOC_BADSIZE 2
#define READALLOC_OOM     3
#define READALLOC_READERR 4

int read_alloc_file(const char* filename, void** ptr, size_t* size);

#endif
