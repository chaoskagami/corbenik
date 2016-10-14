#include <3ds.h>
#include <string.h>
#include "patcher.h"
#include <string.h>

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include <option.h>
#include <patch_format.h>

// Quick Search algorithm, adapted from
// http://igm.univ-mlv.fr/~lecroq/string/node19.html#SECTION00190
u8 *
memfind(u8 *startPos, u32 size, const void *pattern, u32 patternSize)
{
    const u8 *patternc = (const u8 *)pattern;

    // Preprocessing
    u32 table[256];

    for (u32 i = 0; i < 256; ++i)
        table[i] = patternSize + 1;
    for (u32 i = 0; i < patternSize; ++i)
        table[patternc[i]] = patternSize - i;

    // Searching
    u32 j = 0;

    while (j <= size - patternSize) {
        if (memcmp(patternc, startPos + j, patternSize) == 0)
            return startPos + j;
        j += table[startPos[j + patternSize]];
    }

    return NULL;
}

u32
patchMemory(u8 *start, u32 size, const void *pattern, u32 patSize, int offset, const void *replace, u32 repSize, u32 count)
{
    u32 i;

    for (i = 0; i < count; i++) {
        u8 *found = memfind(start, size, pattern, patSize);

        if (found == NULL)
            break;

        // FIXME - This is throwing on Werror.
        memcpy(found + offset, replace, repSize);

        u32 at = (u32)(found - start);

        if (at + patSize > size)
            break;

        size -= at + patSize;
        start = found + patSize;
    }

    return i;
}
