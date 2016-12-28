#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blib.h"

size_t fsize(FILE* file) {
	size_t at = ftell(file);

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, at, SEEK_SET);

	return size;
}

int read_alloc_file(const char* filename, void** ptr, size_t* size) {
	FILE* handle   = fopen(filename, "rb");
	if (!handle)
		return -READALLOC_NOFILE;
	
	size_t alloc_size = *size;

	*size = fsize(handle);
	if (size <= 0) {
		fclose(handle);
		*size = 0;
		return -READALLOC_BADSIZE;
	}
	
	if (alloc_size) {
		*ptr = malloc(alloc_size);
		if (alloc_size < *size) {
			*size = alloc_size;
		}
	} else {
		*ptr = malloc(*size);
	}

	if (*ptr == NULL) {
		fclose(handle);
		*ptr = NULL;
		*size = 0;
		return -READALLOC_OOM;
	}
	
	if (fread(*ptr, 1, *size, handle) != *size) {
		free(*ptr);
		fclose(handle);
		*ptr = NULL;
		*size = 0;
		return -READALLOC_READERR;
	}
	
	if (alloc_size && alloc_size > *size) {
		*size = alloc_size;
	}
	
	fclose(handle);
	
	return 0;
}

#define ALPHABET_LEN 256
#define NOT_FOUND patlen
#define max(a, b) ((a < b) ? b : a)

// Quick Search algorithm, adapted from
// http://igm.univ-mlv.fr/~lecroq/string/node19.html#SECTION00190
const void *
memfind(const void *input, uint32_t size, const void *pattern, uint32_t patternSize)
{
    const uint8_t *startPos = (const uint8_t *)input;
    const uint8_t *patternc = (const uint8_t *)pattern;

    // Preprocessing
    uint32_t table[ALPHABET_LEN];

    for (uint32_t i = 0; i < ALPHABET_LEN; ++i)
        table[i] = patternSize + 1;
    for (uint32_t i = 0; i < patternSize; ++i)
        table[patternc[i]] = patternSize - i;

    // Searching
    size_t j = 0;

    while (j <= size - patternSize) {
        if (memcmp(patternc, startPos + j, patternSize) == 0)
            return startPos + j;
        j += table[startPos[j + patternSize]];
    }

    return NULL;
}

