#include <stdint.h>
#include <stddef.h>
#ifndef LOADER
  #include "std/unused.h"
  #include "std/memory.h"
  #include "firm/firm.h"
  #include "config.h"
  #include "common.h"
#endif

#define OP_NOP    0x00
#define OP_REL    0x01
#define OP_FIND   0x02
#define OP_BACK   0x03
#define OP_FWD    0x04
#define OP_SET    0x05
#define OP_TEST   0x06
#define OP_JMP    0x07
#define OP_REWIND 0x08
#define OP_AND    0x09
#define OP_TITLE  0x0A

#ifdef LOADER
  #define log(a) logstr(a)
  #define abort(a) { logstr(a) ; svcBreak(USERBREAK_ASSERT) ; }
#else
  #define log(a) fprintf(stderr, a)
#endif

struct mode {
	uint8_t* memory;
	uint32_t size;
};

struct mode modes[19];
int init_bytecode = 0;

int exec_bytecode(uint8_t* bytecode, uint32_t len, int debug) {
	if (!init_bytecode) {
#ifndef LOADER
		modes[0].memory = (uint8_t*)firm_loc;
		modes[0].size   = FCRAM_SPACING; // NATIVE_FIRM

		modes[1].memory = (uint8_t*)agb_firm_loc;
		modes[1].size   = FCRAM_SPACING; // AGB_FIRM

		modes[2].memory = (uint8_t*)twl_firm_loc;
		modes[2].size   = FCRAM_SPACING; // TWL_FIRM

		// NATIVE_FIRM Process9 (This is also the default mode.)
		modes[3].memory = (uint8_t*)firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
		modes[3].size   = firm_p9_exefs->fileHeaders[0].size;
		// AGB_FIRM Process9
		modes[4].memory = (uint8_t*)agb_firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
		modes[4].size   = firm_p9_exefs->fileHeaders[0].size;
		// TWL_FIRM Process9
		modes[5].memory = (uint8_t*)twl_firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
		modes[5].size   = firm_p9_exefs->fileHeaders[0].size;

		// NATIVE_FIRM Sect 0
		modes[6].memory = (uint8_t*)&firm_loc->section[0] + firm_loc->section[0].offset;
		modes[6].size   = firm_loc->section[0].size;
		// NATIVE_FIRM Sect 1
		modes[7].memory = (uint8_t*)&firm_loc->section[1] + firm_loc->section[1].offset;
		modes[7].size   = firm_loc->section[1].size;
		// NATIVE_FIRM Sect 2
		modes[8].memory = (uint8_t*)&firm_loc->section[2] + firm_loc->section[2].offset;
		modes[8].size   = firm_loc->section[2].size;
		// NATIVE_FIRM Sect 3
		modes[9].memory = (uint8_t*)&firm_loc->section[3] + firm_loc->section[3].offset;
		modes[9].size   = firm_loc->section[3].size;

		// AGB_FIRM Sect 0
		modes[10].memory = (uint8_t*)&agb_firm_loc->section[0] + agb_firm_loc->section[0].offset;
		modes[10].size   = agb_firm_loc->section[0].size;
		// AGB_FIRM Sect 1
		modes[11].memory = (uint8_t*)&agb_firm_loc->section[1] + agb_firm_loc->section[1].offset;
		modes[11].size   = agb_firm_loc->section[1].size;
		// AGB_FIRM Sect 2
		modes[12].memory = (uint8_t*)&agb_firm_loc->section[2] + agb_firm_loc->section[2].offset;
		modes[12].size   = agb_firm_loc->section[2].size;
		// AGB_FIRM Sect 3
		modes[13].memory = (uint8_t*)&agb_firm_loc->section[3] + agb_firm_loc->section[3].offset;
		modes[13].size   = agb_firm_loc->section[3].size;

		// TWL_FIRM Sect 0
		modes[14].memory = (uint8_t*)&twl_firm_loc->section[0] + twl_firm_loc->section[0].offset;
		modes[14].size   = twl_firm_loc->section[0].size;
		// TWL_FIRM Sect 1
		modes[15].memory = (uint8_t*)&twl_firm_loc->section[1] + twl_firm_loc->section[1].offset;
		modes[15].size   = twl_firm_loc->section[1].size;
		// TWL_FIRM Sect 2
		modes[16].memory = (uint8_t*)&twl_firm_loc->section[2] + twl_firm_loc->section[2].offset;
		modes[16].size   = twl_firm_loc->section[2].size;
		// TWL_FIRM Sect 3
		modes[17].memory = (uint8_t*)&twl_firm_loc->section[3] + twl_firm_loc->section[3].offset;
		modes[17].size   = twl_firm_loc->section[3].size;
#endif

		// Loader (not valid in bootmode)
		// modes[18] = { 0, 0 };

		init_bytecode = 1;
	}

#ifdef LOADER
	uint32_t set_mode = 18;
#else
	uint32_t set_mode = 3;
#endif
	struct   mode* current_mode = &modes[set_mode];

	uint32_t offset = 0;
	uint8_t  test_was_false = 0;

	uint32_t i;

	uint8_t* code = bytecode;
	uint8_t* end = code + len;
	while (code < end && code >= bytecode) {
		switch(*code) {
			case OP_NOP:
				if (debug)
					log("nop\n");
				code++;
				test_was_false = 0;
				break;
			case OP_REL: // Change relativity.
#ifdef LOADER
				// Loader doesn't support this. Just treat it like a two-byte NOP.
				code += 2;
#else
				if (debug)
					log("rel\n");
				code++;
				if (!test_was_false)
					current_mode = &modes[*code];
				else
					test_was_false = 0;
				set_mode = *code;
				code++;
#endif
				break;
			case OP_FIND: // Find pattern.
				if (debug)
					log("find\n");
				code += 2;
				if (!test_was_false) {
					offset = (uint32_t)memfind(current_mode->memory+offset, current_mode->size - offset, code, *(code-1));
					if ((uint8_t*)offset == NULL) {
						// Error. Abort.
						abort("Find opcode failed.\n");
					}
					offset = offset - (uint32_t)current_mode->memory;
				} else {
					test_was_false = 0;
				}
				code += *(code-1);
				break;
			case OP_BACK:
				if (debug)
					log("back\n");
				code++;
				if (!test_was_false) {
					if (offset < *code) {
						// Went out of bounds. Error.
						abort("Back underflowed.\n");
					}
					offset -= *code;
				} else {
					test_was_false = 0;
				}
				code++;
				break;
			case OP_FWD:
				if (debug)
					log("fwd\n");
				code++;
				if (!test_was_false) {
					offset += *code;
					if (offset >= current_mode->size) {
						// Went out of bounds. Error.
						abort("Fwd overflowed.\n");
					}
				} else {
					test_was_false = 0;
				}
				code++;
				break;
			case OP_SET: // Set data.
				if (debug)
					log("set\n");
				code += 2;
				if (!test_was_false)
					memcpy(current_mode->memory+offset, code, *(code-1));
				else
					test_was_false = 0;
				offset += *(code-1);
				code   += *(code-1);
				break;
			case OP_TEST: // Test data.
				if (debug)
					log("test\n");
				code += 2;
				if(memcmp(current_mode->memory+offset, code, *(code-1))) {
					test_was_false = 1;
					if (debug)
						log("false\n");
				} else if (debug) {
					log("true\n");
				}
				code   += *(code-1);
				break;
			case OP_JMP: // Jump to offset.
				if (debug)
					log("jmp\n");
				code++;
				if (!test_was_false) {
					code = bytecode + code[1] + ( code[0] * 0x100 );
				} else {
					code += 2;
					test_was_false = 0;
				}
				break;
			case OP_REWIND:
				if (debug)
					log("rewind\n");
				code++;
				if (!test_was_false)
					offset = 0;
				else
					test_was_false = 0;
				break;
			case OP_AND:
				if (debug)
					log("and\n");
				code += 2;
				if (!test_was_false) {
					for(i=0; i < *(code-1); i++) {
						*(current_mode->memory + offset) &= code[i];
					}
					offset += *(code-1);
				} else {
					test_was_false = 0;
				}
				code   += *(code-1);
				break;
			case OP_TITLE:
				if (debug)
					log("title\n");
				// FIXME - NYI
			default:
#ifndef LOADER
				// Panic; not proper opcode.
				fprintf(stderr, "Invalid opcode. State:\n"
								"  Relative:  %u\n"
								"    Actual:  %x:%u\n"
								"  Memory:    %x\n"
								"    Actual:  %x\n"
								"  Code Loc:  %x\n"
								"    Actual:  %x\n"
								"  Opcode:    %u\n",
								set_mode,
								current_mode->memory, current_mode->size,
								offset,
								current_mode->memory + offset,
								code - bytecode,
								code,
								*code);
#endif
				abort("Halting startup.\n");
				break;
		}
	}

	return 0;
}

#ifdef LOADER
int execb(char* filename, uint64_t tid, uint8_t* search_mem, uint32_t search_len) {
#else
int execb(char* filename) {
#endif
	uint32_t patch_len;
	struct system_patch* patch;
#ifdef LOADER
	log("  check ");
	log(filename);
	log("\n");

	uint8_t  patch_dat2[MAX_PATCHSIZE];
	uint8_t *patch_dat = patch_dat2;

	patch = (struct system_patch*)patch_dat;

	Handle file;
	u32 total;

    // Open file.
    if (!R_SUCCEEDED(fileOpen(&file, ARCHIVE_SDMC, filename, FS_OPEN_READ))) {
        // Failed to open.
        return 1;
    }

	u64 file_size;

	if (!R_SUCCEEDED(FSFILE_GetSize(file, &file_size))) {
        FSFILE_Close(file); // Read to memory.

		return 1;
	}

	if (file_size > MAX_PATCHSIZE) {
		log("  too large (please report)\n");

        FSFILE_Close(file); // Read to memory.

		return 1;
	}

    // Read file.
    if (!R_SUCCEEDED(FSFILE_Read(file, &total, 0, patch_dat, file_size))) {
        FSFILE_Close(file); // Read to memory.

        // Failed to read.
        return 1;
    }

    FSFILE_Close(file); // Done reading in.

	// Set memory.
	modes[18].memory = search_mem;
	modes[18].size   = search_len;

	// Check magic.
	if (memcmp(patch->magic, "AIDA", 4)) {
		log("  magic wrong\n");

		return 1; // Incorrect patch magic.
	}

	// Check TID supported.
	if (patch->titles == 0) {
		return 0; // Not an error - this patch isn't meant for us.
	}

	log("  checking tid\n");

	uint8_t* title_buf = patch_dat + sizeof(struct system_patch);

	int apply = 0;

	for(uint32_t i=0; i < patch->titles; i++, title_buf += 8) {
		if(!memcmp(title_buf, (uint8_t*)&tid, 8)) {
			// Applicable patch found.
			log("  applicable\n");
			apply = 1;
			break;
		}
	}

	if (!apply) {
		// Not meant for us. Not an error, though.
		return 0;
	}

	// Patch is relevant to us, so we'll apply it.

	log("  exec\n");

	uint8_t* patch_mem = (uint8_t*)patch + sizeof(struct system_patch) + (patch->depends * 8) + (patch->titles * 8);
	patch_len = patch->size;
#else
	uint8_t* patch_mem;
	// Read patch to scrap memory.

	FILE* f = fopen(filename, "r");
	size_t len = fsize(f);
	fread((uint8_t*)FCRAM_PATCH_LOC, 1, len, f);
	fclose(f);

	patch = (struct system_patch*)FCRAM_PATCH_LOC;

	// Make sure various bits are correct.
	if (memcmp(patch->magic, "AIDA", 4)) {
		// Incorrect magic.
		return 1;
	}

	if (patch->titles != 0) {
		// Not an error, per se, but it means this patch is meant for loader, not us.
		// Patches intended for use during boot will always be applied to zero titles.
		return 0;
	}

	fprintf(stderr, "Patch: %s\n", patch->name);

	patch_mem = (uint8_t*)patch + sizeof(struct system_patch) + (patch->depends * 8) + (patch->titles * 8);
	patch_len = patch->size;
#endif
	return exec_bytecode(patch_mem, patch_len, 1);
}
