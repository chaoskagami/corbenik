#include <stddef.h>
#include "fs.h"
#include "memory.h"
#include "../fatfs/ff.h"

static FATFS fs;

int fmount(void) {
    if (f_mount(&fs, "0:", 1)) return 1;
    return 0;
}

int fumount(void) {
    if (f_mount(NULL, "0:", 1)) return 1;
    return 0;
}

int fopen(FILE* fp, const char *filename, const char *mode) {
    if (*mode != 'r' && *mode != 'w' && *mode != 'a')
        return 1; // Mode not valid.

	fp->mode = (*mode == 'r' ? FA_READ : (FA_WRITE | FA_OPEN_ALWAYS));

    return f_open(&(fp->handle), filename, fp->mode);
}

void fclose(FILE* fp) {
    f_close(&(fp->handle));
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
    FILE temp;
    fopen(&temp, path, "w");
    size_t written = fwrite(data, 1, size, &temp);
    fclose(&temp);
    return written;
}

size_t read_file(void* data, char* path, size_t size) {
    FILE temp;
    fopen(&temp, path, "r");

    size_t read = fread(data, 1, size, &temp);
    fclose(&temp);

    return read;
}
