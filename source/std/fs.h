#ifndef __STD_FS_H
#define __STD_FS_H

#include "types.h"
#include "memory.h"
#include "../fatfs/ff.h"

typedef struct {
	FIL handle;
	uint32_t mode;
	size_t size;
	size_t at;
} FILE;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int    fmount   (void);
int    fumount  (void);

int    fopen     (FILE* fp, const char *filename, const char *mode);

void   fclose    (FILE* fp);

void   fseek     (FILE* fp, int64_t offset, int whence);

size_t ftell     (FILE* fp);

int    feof      (FILE* fp);

size_t fsize     (FILE* fp);

size_t fwrite    (const char *buffer, size_t elementSize, size_t elementCnt, FILE* fp);
size_t fread     (char *buffer,       size_t elementSize, size_t elementCnt, FILE* fp);

#endif
