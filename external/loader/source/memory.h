#ifndef __MEMORY_H
#define __MEMORY_H

u8 *memfind(u8 *startPos, u32 size, const void *pattern, u32 patternSize);
u32 patchMemory(u8 *start, u32 size, const void *pattern, u32 patSize, int offset, const void *replace, u32 repSize, u32 count);

#endif
