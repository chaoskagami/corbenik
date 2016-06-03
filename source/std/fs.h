#ifndef __STD_FS_H
#define __STD_FS_H

#include "types.h"
#include "memory.h"
#include "../fatfs/ff.h"

#define MAX_FILES_OPEN 64

typedef struct
{
    FIL handle;
    uint32_t mode;
    size_t size;
    size_t at;
    uint8_t is_open;
} __attribute__((packed)) FILE;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int fmount(void);
int fumount(void);

int rrmdir(char* dir_path);

FILE *fopen(const char *filename, const char *mode);

void fclose(FILE *fp);

void fseek(FILE *fp, int64_t offset, int whence);

size_t ftell(FILE *fp);

int feof(FILE *fp);

size_t fsize(FILE *fp);

size_t fwrite(const void *buffer, size_t elementSize, size_t elementCnt, FILE *fp);
size_t fread(void *buffer, size_t elementSize, size_t elementCnt, FILE *fp);

size_t write_file(void *data, char *path, size_t size);
size_t read_file(void *data, char *path, size_t size);

#endif
