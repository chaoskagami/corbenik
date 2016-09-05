#ifndef __STD_MEMORY_H
#define __STD_MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

int atoi(const char *str);

uint8_t *memfind(uint8_t *startPos, uint32_t size, const void *pattern, uint32_t patternSize);

/* Basically strdup, because newlib's memory handling is crap.
 *
 * Note that the caller is reponsible for freeing memory.
 *
 * \param str String to duplicate
 * \return Duplicated string.
 */
char* strdup_self(const char* str);

char* strdupcat(const char* str, const char *cat);

#endif
