#ifndef __STD_MEMORY_H
#define __STD_MEMORY_H

#include <stdint.h>
#include <stddef.h>

int strlen(const char* string);
void memcpy(void* dest, const void* src, size_t size);
void memmove(void* dest, const void* src, size_t size);
void memset(void* dest, const int filler, size_t size);
int memcmp(const void* buf1, const void* buf2, const size_t size);
void strncpy(void* dest, const void* src, const size_t size);
int strncmp(const void* buf1, const void* buf2, const size_t size);
int atoi(const char* str);
uint8_t* memfind(uint8_t* startPos, uint32_t size, const void* pattern,
                 uint32_t patternSize);
int isprint(char c);

#endif
