#include "memory.h"

#include <stdint.h>
#include <stddef.h>

size_t
strlen(const char *string)
{
    char *string_end = (char *)string;
    while (string_end[0])
        string_end++;
    return string_end - string;
}

size_t
strnlen(const char *string, size_t maxlen)
{
    size_t size;

    for (size = 0; *string && size < maxlen; string++, size++)
        ;

    return size;
}

int
isprint(char c)
{
    if (c >= 32 && c <= 127)
        return 1;
    return 0;
}

void
memcpy(void *dest, const void *src, size_t size)
{
    uint8_t *destc = (uint8_t *)dest;
    const uint8_t *srcc = (const uint8_t *)src;

    for(size_t i=0; i < size; i++) {
        destc[i] = srcc[i];
    }
}

void
memmove(void *dest, const void *src, size_t size)
{
    // memcpy does the job of moving backwards just fine
    if (dest < src || (uint8_t*)src + size <= (uint8_t*)dest) {
        memcpy(dest, src, size);
        return;
    }

    // Moving forward is just a reverse memcpy
    uint8_t *destc = (uint8_t *)dest;
    const uint8_t *srcc = (const uint8_t *)src;

    // Finish by copying the leftovers
    for(size_t i=size; i > 0; i--) {
        destc[i-1] = srcc[i-1];
    }
}

void
memset(void *dest, const int filler, size_t size)
{
    char *destc = (char *)dest;

    // Finish
    for(size_t i = 0; i < size; i++) {
        destc[i] = filler;
    }
}

int
memcmp(const void *buf1, const void *buf2, const size_t size)
{
    const char *buf1c = (const char *)buf1;
    const char *buf2c = (const char *)buf2;
    for (size_t i = 0; i < size; i++) {
        int cmp = buf1c[i] - buf2c[i];
        if (cmp) {
            return cmp;
        }
    }

    return 0;
}

void
strncpy(void *dest, const void *src, const size_t size)
{
    char *destc = (char *)dest;
    const char *srcc = (const char *)src;

    size_t i;
    for (i = 0; i < size && srcc[i] != 0; i++) {
        destc[i] = srcc[i];
    }

    // Make sure the resulting string is terminated.
    destc[i] = 0;
}

int
strncmp(const void *buf1, const void *buf2, const size_t size)
{
    const char *buf1c = (const char *)buf1;
    const char *buf2c = (const char *)buf2;

    size_t i;
    for (i = 0; i < size && buf1c[i] != 0 && buf2c[i] != 0; i++) {
        int cmp = buf1c[i] - buf2c[i];
        if (cmp) {
            return cmp;
        }
    }

    // Make sure the strings end at the same offset, if they end.
    if ((buf1c[i] == 0 || buf2c[i] == 0) && (buf1c[i] != 0 || buf2c[i] != 0)) {
        return -1;
    }

    return 0;
}

int
atoi(const char *str)
{
    int res = 0;
    while (str[0] && str[0] >= '0' && str[0] <= '9') {
        res = str[0] - '0' + res * 10;
        str++;
    }

    return res;
}

#define ALPHABET_LEN 256
#define NOT_FOUND patlen
#define max(a, b) ((a < b) ? b : a)

// Quick Search algorithm, adapted from
// http://igm.univ-mlv.fr/~lecroq/string/node19.html#SECTION00190
uint8_t *
memfind(uint8_t *startPos, uint32_t size, const void *pattern, uint32_t patternSize)
{
    const uint8_t *patternc = (const uint8_t *)pattern;

    // Preprocessing
    uint32_t table[ALPHABET_LEN];

    for (uint32_t i = 0; i < ALPHABET_LEN; ++i)
        table[i] = patternSize + 1;
    for (uint32_t i = 0; i < patternSize; ++i)
        table[patternc[i]] = patternSize - i;

    // Searching
    uint32_t j = 0;

    while (j <= size - patternSize) {
        if (memcmp(patternc, startPos + j, patternSize) == 0)
            return startPos + j;
        j += table[startPos[j + patternSize]];
    }

    return NULL;
}
