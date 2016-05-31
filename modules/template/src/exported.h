#ifndef EXPORTED_H
#define EXPORTED_H

#include "symtab.h"

#include "headers.h"

#define stdout ((void*)0)
#define stderr ((void*)2)

int  (*strlen)(const char *string) = strlen_offset;
int  (*isprint)(char c) = isprint_offset;
void (*memcpy)(void *dest, const void *src, size_t size) = memcpy_offset;
void (*memmove)(void *dest, const void *src, size_t size) = memmove_offset;
void (*memset)(void *dest, const int filler, size_t size) = memset_offset;
int  (*memcmp)(const void *buf1, const void *buf2, const size_t size) = memcmp_offset;
void (*strncpy)(void *dest, const void *src, const size_t size) = strncpy_offset;
int  (*strncmp)(const void *buf1, const void *buf2, const size_t size) = strncmp_offset;
int  (*atoi)(const char *str) = atoi_offset;
uint8_t* (*memfind)(uint8_t *string, uint32_t stringlen, uint8_t *pat, uint32_t patlen) = memfind_offset;

void (*putc)(void* buf, const int c) = putc_offset;
void (*puts)(void* buf, const char *string) = puts_offset;
void (*fprintf)(void* channel, const char* format, ...) = fprintf_offset;

exefs_h* (*get_firm_proc9_exefs)() = get_firm_proc9_exefs_offset;
exefs_h* (*get_agb_proc9_exefs)() = get_agb_proc9_exefs_offset;
exefs_h* (*get_twl_proc9_exefs)() = get_twl_proc9_exefs_offset;

#endif
