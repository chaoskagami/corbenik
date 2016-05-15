#include "common.h"
#include "firm/fcram.h"

int link_and_load_patchbin(char* path) {
	fprintf(stderr, "Prep to relocate and execute patch.\n");

	struct system_patch patch;

	fprintf(stderr, "Loading patch vco.\n");

	// Load patch from path.
	FILE* f = fopen(path, "r");
	fread(&patch, 1, sizeof(patch), f);

	fprintf(stderr, "Patch payload is %d bytes.\n", patch.patch_size);

	fread((uint8_t*)FCRAM_PATCHBIN_EXEC_LOC, 1, patch.patch_size, f);

	fclose(f);

	fprintf(stderr, "Finding relocation table.\n");
	// Now then...find the magical number.
	uint8_t magic[] = { 0xc0, 0x9b, 0xe5, 0x1c };
	uint8_t* off = memfind((uint8_t*)FCRAM_PATCHBIN_EXEC_LOC, patch.patch_size, magic, 4);

	if (off == NULL) {
		fprintf(stderr, "Relocation table missing. Abort.\n");
		return 1;
	}

	fprintf(stderr, "Relocation table is at %x\n", (uint32_t)off);

	uint32_t* link_table = (uint32_t*)(off + 4);

	// memory.c
	link_table[3] = (uint32_t)strlen;
	link_table[5] = (uint32_t)isprint;
	link_table[7] = (uint32_t)memcpy;
	link_table[9] = (uint32_t)memmove;
	link_table[11] = (uint32_t)memset;
	link_table[13] = (uint32_t)memcmp;
	link_table[15] = (uint32_t)strncpy;
	link_table[17] = (uint32_t)strncmp;
	link_table[19] = (uint32_t)atoi;
	link_table[21] = (uint32_t)memfind;

	// draw.c
	link_table[23] = (uint32_t)putc;
	link_table[25] = (uint32_t)puts;
	link_table[27] = (uint32_t)fprintf;

	fprintf(stderr, "Copied relocations. Running binary.\n");

	int (*exec)() = (void*)FCRAM_PATCHBIN_EXEC_LOC ;

	return (*exec)();
}
