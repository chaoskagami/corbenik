#include "common.h"
#include "firm/fcram.h"
#include "firm/firm.h"

// Yes, this is EXACTLY what it looks like. We dynamically link and
// load patches as binaries; they use functions from corbenik to do
// the work, and therefore must have a function table in them.

// See vco/template for how this magic works.

// This ensures relatively small patches while also having incredible
// flexibility unlike a 'patch format'.

extern exefs_h* firm_p9_exefs;
exefs_h* get_firm_proc9_exefs() {
	return firm_p9_exefs;
}

extern exefs_h* twl_firm_p9_exefs;
exefs_h* get_twl_proc9_exefs() {
	return twl_firm_p9_exefs;
}

extern exefs_h* agb_firm_p9_exefs;
exefs_h* get_agb_proc9_exefs() {
	return agb_firm_p9_exefs;
}

int execp(char* path) {
	int basename = 0;
	for(basename=strlen(path); basename > 0; basename--)
		if (path[basename] == '/')
			break;
	basename++;

	fprintf(stderr, "Exec: %s\n", &path[basename]);

	struct system_patch patch;

	// Load patch from path.
	FILE* f = fopen(path, "r");
	fread(&patch, 1, sizeof(patch), f);

	fprintf(stderr, "  [h]");

	fread((uint8_t*)FCRAM_PATCHBIN_EXEC_LOC, 1, patch.patch_size, f);

	fprintf(stderr, "[x]");

	fclose(f);

	fprintf(stderr, "[s]");

	uint32_t* link_table = (uint32_t*)(FCRAM_PATCHBIN_EXEC_LOC+4);

	fprintf(stderr, "[r]");

	// 0 - magic

	// memory.c
	link_table[2] = (uint32_t)strlen;
	link_table[4] = (uint32_t)isprint;
	link_table[6] = (uint32_t)memcpy;
	link_table[8] = (uint32_t)memmove;
	link_table[10] = (uint32_t)memset;
	link_table[12] = (uint32_t)memcmp;
	link_table[14] = (uint32_t)strncpy;
	link_table[16] = (uint32_t)strncmp;
	link_table[18] = (uint32_t)atoi;
	link_table[20] = (uint32_t)memfind;

	// draw.c
	link_table[22] = (uint32_t)putc;
	link_table[24] = (uint32_t)puts;
	link_table[26] = (uint32_t)fprintf;

	// Get functions
	link_table[28] = (uint32_t)get_firm_proc9_exefs;

	link_table[30] = (uint32_t)get_agb_proc9_exefs;

	link_table[32] = (uint32_t)get_twl_proc9_exefs;

	fprintf(stderr, "[b]\n");

	int (*patch_loc)() = (void*)FCRAM_PATCHBIN_EXEC_LOC ;

	int ret = (*patch_loc)();

	fprintf(stderr, "  Exit: %d\n", ret);

	return ret;
}
