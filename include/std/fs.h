#ifndef __STD_FS_H
#define __STD_FS_H

#include <ctr9/io.h>

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

int crmount(void);

int crumount(void);

void recurse_call(const char *name, void (*call_fun)(char*));

int rrmdir(const char *dir_path);

FILE *cropen(const char *filename, const char *mode);

void crclose(FILE *fp);

void crseek(FILE *fp, int64_t offset, int whence);

size_t crtell(FILE *fp);

int creof(FILE *fp);

size_t crsize(FILE *fp);

size_t crwrite(const void *buffer, size_t elementSize, size_t elementCnt, FILE *fp);

size_t crread(void *buffer, size_t elementSize, size_t elementCnt, FILE *fp);

size_t write_file(void *data, const char *path, size_t size);

size_t read_file(void *data, const char *path, size_t size);

#endif
