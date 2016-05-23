#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../source/patch_format.h"

void read_file_u64(char* name, uint64_t* to) {
	FILE* hdl = fopen(name, "rb");
	fscanf(hdl, "%llu", to);
	fclose(hdl);
}


void read_file_u32(char* name, uint32_t* to) {
	FILE* hdl = fopen(name, "rb");
	fscanf(hdl, "%u", to);
	fclose(hdl);
}

void read_str(char* name, char* to, size_t len) {
	FILE* hdl = fopen(name, "rb");
	int r = fread(to, 1, len-1, hdl);
	fclose(hdl);

	for(int i=len-1; i >= 0; i--) {
		if (to[i] == '\n') {
			to[i] = 0;
			break;
		}
	}
}

uint32_t size = 0;

uint8_t* read_file_mem(char* name) {
	FILE* hdl = fopen(name, "rb");

	fseek(hdl, 0, SEEK_END);
	size = ftell(hdl);
	rewind(hdl);

	uint8_t* mem = malloc(size);
	memset(mem, 0, size);

	int r = fread(mem, 1, size, hdl);
	fclose(hdl);

	return mem;
}

int main(int c, char** v) {
	struct system_patch patch;
	int at = 0;

	memset(&patch, 0, sizeof(patch));

	// Set magic.
	patch.magic[0] = 'A';
	patch.magic[1] = 'I';
	patch.magic[2] = 'D';
	patch.magic[3] = 'A';

	read_file_u32("meta/patch_version", & patch.patch_ver);
	read_file_u32("meta/cfw_version", & patch.load_ver);

	read_file_u64("meta/uuid", & patch.patch_id);
	read_file_u64("meta/title", & patch.tid);

	read_str("meta/name", patch.name, sizeof(patch.name));
	read_str("meta/desc", patch.desc, sizeof(patch.desc));

	uint8_t* code = read_file_mem("out/patch.bin");

	patch.patch_size = size;

	FILE* out = fopen("out/patch.vco", "wb");
	fwrite(&patch, 1, sizeof(patch),    out);
	fwrite(code,   1, patch.patch_size, out);
	fclose(out);

	free(code);

	printf("Ver:  %u\n"
			"CFW:  %u\n"
			"UUID: %llu\n"
			"TID:  %llu\n"
			"Name: %s\n"
			"Desc: %s\n"
			"Size: %u\n",
			patch.patch_ver, patch.load_ver, patch.patch_id,
			patch.tid, patch.name, patch.desc, patch.patch_size);

	return 0;
}
