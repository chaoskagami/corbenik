#include <stddef.h>
#include "fs.h"
#include "memory.h"
#include "../fatfs/ff.h"
#include "draw.h"

static FATFS fs;

static FILE files[MAX_FILES_OPEN];

int fmount(void) {
    if (f_mount(&fs, "0:", 1))
        return 1;

    for(int i=0; i < MAX_FILES_OPEN; i++)
        memset(&files[i], 0, sizeof(FILE));

    return 0;
}

int fumount(void) {
    for(int i=0; i < MAX_FILES_OPEN; i++)
        if (files[i].is_open)
            fclose(&files[i]);

    if (f_mount(NULL, "0:", 1))
        return 1;

    return 0;
}

FILE* fopen(const char *filename, const char *mode) {
    if (*mode != 'r' && *mode != 'w' && *mode != 'a')
        return NULL; // Mode not valid.

    FILE* fp;
    int i;
    for(i=0; i < MAX_FILES_OPEN; i++) {
        if(!files[i].is_open) {
           fp = &files[i];
           break;
        }
    }

    if (i == MAX_FILES_OPEN)
        return NULL; // Out of handles.

	fp->mode = (*mode == 'r' ? FA_READ : (FA_WRITE | FA_OPEN_ALWAYS));

    if (f_open(&(fp->handle), filename, fp->mode))
        return NULL;

    fp->is_open = 1;

    return fp;
}

void fclose(FILE* fp) {
    f_close(&(fp->handle));

    memset(fp, 0, sizeof(FILE));
}

void fseek(FILE* fp, int64_t offset, int whence) {
	uint32_t fixed_offset;
	switch(whence) {
		case SEEK_SET:
			fixed_offset = 0;
			break;
		case SEEK_CUR:
			fixed_offset = ftell(fp);
			break;
		case SEEK_END:
			fixed_offset = fsize(fp);
			break;
		default:
			return;
	}

    f_lseek(&(fp->handle), fixed_offset+offset);
}

size_t ftell(FILE* fp) {
	return f_tell(&(fp->handle));
}

int feof(FILE* fp) {
    return f_eof(&(fp->handle));
}

size_t fsize(FILE* fp) {
    return f_size(&(fp->handle));
}

size_t fwrite(const void *buffer, size_t elementSize, size_t elementCnt, FILE* fp) {
    UINT br;
    if(f_write(&(fp->handle), buffer, elementSize*elementCnt, &br)) return 0;
    if (br == elementSize*elementCnt) br /= elementSize; else return 0;
    return br;
}

size_t fread(void *buffer, size_t elementSize, size_t elementCnt, FILE* fp) {
    UINT br;
    if(f_read(&(fp->handle), buffer, elementSize*elementCnt, &br))
        return 0;
    if (br == elementSize*elementCnt)
        br /= elementSize;
    else
        return 0;
    return br;
}

size_t write_file(void* data, char* path, size_t size) {
    FILE* temp = fopen(path, "w");

    if (!temp)
        return 0;

    size_t wrote = fwrite(data, 1, size, temp);

    fclose(temp);

    return wrote;
}

size_t read_file(void* data, char* path, size_t size) {
    FILE* temp = fopen(path, "r");

    if (!temp)
        return 0;

    size_t read = fread(data, 1, size, temp);

    fclose(temp);

    return read;
}
