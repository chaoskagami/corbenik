#include <common.h>

#include <stdint.h>
#include <stddef.h>

char*
strdup_self(const char* str)
{
    size_t l = strlen(str);
    char* new_st = malloc(l+1);
    memcpy(new_st, str, l+1); // Copy the nul byte
    return new_st;
}

int
atoi(const char *str)
{
    int res = 0;

    while (str[0] && str[0] >= '0' && str[0] <= '9') {
        res *= 10;
        res += str[0] - '0';
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
