#ifndef __STD_MEMORY_H
#define __STD_MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

int atoi(const char *str);
uint8_t *memfind(uint8_t *startPos, uint32_t size, const void *pattern, uint32_t patternSize);
int isprint(char c);

#endif
