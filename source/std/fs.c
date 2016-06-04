#include <stddef.h>
#include "fs.h"
#include "memory.h"
#include "../fatfs/ff.h"
#include "draw.h"
#include "memory.h"

static FATFS fs;

static FILE files[MAX_FILES_OPEN];

// This function is based on PathDeleteWorker from GodMode9.
// It was easier to just import it.
int rrmdir_back(char* fpath) {
	FILINFO fno = {.lfname = NULL};

	// this code handles directory content deletion
	if (f_stat(fpath, &fno) != FR_OK)
		return 1; // fpath does not exist

	if (fno.fattrib & AM_DIR) { // process folder contents
		DIR pdir;
		char* fname = fpath + strnlen(fpath, 255);
		if (f_opendir(&pdir, fpath) != FR_OK)
			return 1;

		*(fname++) = '/';
		fno.lfname = fname;
		fno.lfsize = fpath + 255 - fname;

		while (f_readdir(&pdir, &fno) == FR_OK) {
			if ((strncmp(fno.fname, ".", 2) == 0) || (strncmp(fno.fname, "..", 3) == 0))
				continue; // filter out virtual entries
			if (fname[0] == 0)
				strncpy(fname, fno.fname, fpath + 255 - fname);
			if (fno.fname[0] == 0)
				break;
			else // return value won't matter
				rrmdir_back(fpath);
		}

		f_closedir(&pdir);
		*(--fname) = '\0';
	}

	return f_unlink(fpath);
}

int rrmdir(char* name) {
	char fpath[256];
	strncpy(fpath, name, 256);
	return rrmdir_back(fpath);
}

int
fmount(void)
{
    if (f_mount(&fs, "0:", 1))
        return 1;

    for (int i = 0; i < MAX_FILES_OPEN; i++)
        memset(&files[i], 0, sizeof(FILE));

    return 0;
}

int
fumount(void)
{
    for (int i = 0; i < MAX_FILES_OPEN; i++)
        if (files[i].is_open)
            fclose(&files[i]);

    if (f_mount(NULL, "0:", 1))
        return 1;

    return 0;
}

FILE *
fopen(const char *filename, const char *mode)
{
    if (*mode != 'r' && *mode != 'w' && *mode != 'a')
        return NULL; // Mode not valid.

    FILE *fp;
    int i;
    for (i = 0; i < MAX_FILES_OPEN; i++) {
        if (!files[i].is_open) {
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

void
fclose(FILE *fp)
{
	if (!fp->is_open)
		return;

    f_close(&(fp->handle));

    memset(fp, 0, sizeof(FILE));
}

void
fseek(FILE *fp, int64_t offset, int whence)
{
	if (!fp->is_open)
		return;

    uint32_t fixed_offset;
    switch (whence) {
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

    f_lseek(&(fp->handle), fixed_offset + offset);
}

size_t
ftell(FILE *fp)
{
	if (!fp->is_open)
		return 0;

    return f_tell(&(fp->handle));
}

int
feof(FILE *fp)
{
	if (!fp->is_open)
		return 0;

    return f_eof(&(fp->handle));
}

size_t
fsize(FILE *fp)
{
	if (!fp->is_open)
		return 0;

    return f_size(&(fp->handle));
}

size_t
fwrite(const void *buffer, size_t elementSize, size_t elementCnt, FILE *fp)
{
	if (!fp->is_open)
		return 0;

    UINT br;
    if (f_write(&(fp->handle), buffer, elementSize * elementCnt, &br))
        return 0;
    if (elementSize != 1)
        br /= elementSize;
    return br;
}

size_t
fread(void *buffer, size_t elementSize, size_t elementCnt, FILE *fp)
{
	if (!fp->is_open)
		return 0;

    size_t br;
    if (f_read(&(fp->handle), buffer, elementSize * elementCnt, &br))
        return 0;

    if (elementSize != 1)
        br /= elementSize;
    return br;
}

size_t
write_file(void *data, char *path, size_t size)
{
    FILE *temp = fopen(path, "w");

    if (!temp || !temp->is_open)
        return 0;

    size_t wrote = fwrite(data, 1, size, temp);

    fclose(temp);

    return wrote;
}

size_t
read_file(void *data, char *path, size_t size)
{
    FILE *temp = fopen(path, "r");

    if (!temp || !temp->is_open)
        return 0;

    size_t read = fread(data, 1, size, temp);

    fclose(temp);

    return read;
}
