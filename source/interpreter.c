#include <stdint.h>
#include <stddef.h>
#include "std/unused.h"

#ifndef LOADER
  #include <common.h>
#else
  #include <string.h>
#endif

#define OP_NOP 0x00
#define OP_REL 0x01
#define OP_FIND 0x02
#define OP_BACK 0x03
#define OP_FWD 0x04
#define OP_SET 0x05
#define OP_TEST 0x06
#define OP_JMP 0x07
#define OP_REWIND 0x08
#define OP_AND 0x09
#define OP_OR 0x0A
#define OP_XOR 0x0B
#define OP_NOT 0x0C
#define OP_VER 0x0D
#define OP_CLF 0x0E
#define OP_SEEK 0x0F
#define OP_N3DS 0x10

#define OP_ABORT   0x18
#define OP_ABORTEQ 0x28
#define OP_ABORTNE 0x38
#define OP_ABORTLT 0x48
#define OP_ABORTGT 0x58
#define OP_ABORTF  0x68
#define OP_ABORTNF 0x78

#define OP_JMPEQ 0x17
#define OP_JMPNE 0x27
#define OP_JMPLT 0x37
#define OP_JMPGT 0x47
#define OP_JMPLE 0x57
#define OP_JMPGE 0x67
#define OP_JMPF  0x77
#define OP_JMPNF 0x87

#define OP_INJECT 0x90

#define OP_NEXT 0xFF

#ifdef LOADER
  #define log(a) logstr(a)
  #define abort(a)                                                                                                                                               \
      {                                                                                                                                                          \
          logstr(a);                                                                                                                                             \
          svcBreak(USERBREAK_ASSERT);                                                                                                                            \
      }
#else
  #define log(a) fprintf(stderr, a)
#endif

struct mode
{
    uint8_t *memory;
    uint32_t size;
};
struct mode modes[21];
int init_bytecode = 0;

#ifndef LOADER
extern int is_n3ds;
static const char hexDigits[] = "0123456789ABCDEF";
#else
int is_n3ds = 1; // TODO - We don't really need to care, but it should still work from loader
#endif

#define STACK_SIZE 4096
#ifdef LOADER
  static uint8_t stack_glob[STACK_SIZE];
#else
  static uint8_t *stack_glob = NULL;
#endif

int
exec_bytecode(uint8_t *bytecode, uint32_t len, uint8_t* stack, uint32_t stack_size, uint16_t ver, int debug)
{
    if (!init_bytecode) {
#ifndef LOADER
        modes[0].memory = (uint8_t *)firm_loc;
        modes[0].size   = firm_loc->section[0].size + firm_loc->section[1].size + sizeof(firm_h) +
                          firm_loc->section[2].size + firm_loc->section[3].size; // NATIVE_FIRM

        modes[1].memory = (uint8_t *)agb_firm_loc;
        modes[1].size   = agb_firm_loc->section[0].size + agb_firm_loc->section[1].size + sizeof(firm_h) +
                          agb_firm_loc->section[2].size + agb_firm_loc->section[3].size; // AGB_FIRM

        modes[2].memory = (uint8_t *)twl_firm_loc;
        modes[2].size   = twl_firm_loc->section[0].size + twl_firm_loc->section[1].size + sizeof(firm_h) +
                          twl_firm_loc->section[2].size + twl_firm_loc->section[3].size; // TWL_FIRM

        // NATIVE_FIRM Process9 (This is also the default mode.)
        modes[3].memory = (uint8_t *)firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
        modes[3].size = firm_p9_exefs->fileHeaders[0].size;
        // AGB_FIRM Process9
        modes[4].memory = (uint8_t *)agb_firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
        modes[4].size = firm_p9_exefs->fileHeaders[0].size;
        // TWL_FIRM Process9
        modes[5].memory = (uint8_t *)twl_firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
        modes[5].size = firm_p9_exefs->fileHeaders[0].size;

        // NATIVE_FIRM Sect 0
        modes[6].memory = (uint8_t *)firm_loc + firm_loc->section[0].offset;
        modes[6].size = firm_loc->section[0].size;
        // NATIVE_FIRM Sect 1
        modes[7].memory = (uint8_t *)firm_loc + firm_loc->section[1].offset;
        modes[7].size = firm_loc->section[1].size;
        // NATIVE_FIRM Sect 2
        modes[8].memory = (uint8_t *)firm_loc + firm_loc->section[2].offset;
        modes[8].size = firm_loc->section[2].size;
        // NATIVE_FIRM Sect 3
        modes[9].memory = (uint8_t *)firm_loc + firm_loc->section[3].offset;
        modes[9].size = firm_loc->section[3].size;

        // AGB_FIRM Sect 0
        modes[10].memory = (uint8_t *)agb_firm_loc + agb_firm_loc->section[0].offset;
        modes[10].size = agb_firm_loc->section[0].size;
        // AGB_FIRM Sect 1
        modes[11].memory = (uint8_t *)agb_firm_loc + agb_firm_loc->section[1].offset;
        modes[11].size = agb_firm_loc->section[1].size;
        // AGB_FIRM Sect 2
        modes[12].memory = (uint8_t *)agb_firm_loc + agb_firm_loc->section[2].offset;
        modes[12].size = agb_firm_loc->section[2].size;
        // AGB_FIRM Sect 3
        modes[13].memory = (uint8_t *)agb_firm_loc + agb_firm_loc->section[3].offset;
        modes[13].size = agb_firm_loc->section[3].size;

        // TWL_FIRM Sect 0
        modes[14].memory = (uint8_t *)twl_firm_loc + twl_firm_loc->section[0].offset;
        modes[14].size = twl_firm_loc->section[0].size;
        // TWL_FIRM Sect 1
        modes[15].memory = (uint8_t *)twl_firm_loc + twl_firm_loc->section[1].offset;
        modes[15].size = twl_firm_loc->section[1].size;
        // TWL_FIRM Sect 2
        modes[16].memory = (uint8_t *)twl_firm_loc + twl_firm_loc->section[2].offset;
        modes[16].size = twl_firm_loc->section[2].size;
        // TWL_FIRM Sect 3
        modes[17].memory = (uint8_t *)twl_firm_loc + twl_firm_loc->section[3].offset;
        modes[17].size = twl_firm_loc->section[3].size;
#endif

        init_bytecode = 1;
    }

    memset(stack, 0, stack_size); // Clear stack.

    _UNUSED uint32_t top = stack_size - 1;

#ifdef LOADER
    uint32_t set_mode = 18;
#else
    uint32_t set_mode = 3;
#endif
    struct mode *current_mode = &modes[set_mode];

    uint32_t offset = 0, new_offset = 0;

    uint32_t i;

    int eq = 0, gt = 0, lt = 0, found = 0; // Flags.

    uint8_t *code = bytecode;
    uint8_t *end = code + len;
    while (code < end && code >= bytecode) {
        switch (*code) {
            case OP_NOP:
                if (debug) {
                    log("nop\n");
                }
                code++;
                break;
            case OP_REL: // Change relativity.
                if (debug) {
#ifdef LOADER
                    log("rel\n");
#else
                    fprintf(stderr, "rel %hhu\n", code[1]);
#endif
                }
                code++;
                current_mode = &modes[*code];
                set_mode = *code;
                code++;
                break;
            case OP_FIND: // Find pattern.
                if (debug) {
#ifdef LOADER
                    log("find\n");
#else
                    fprintf(stderr, "find %hhu ...\n", code[1]);
#endif
                }
                found = 0;
                new_offset = (size_t)memfind(current_mode->memory + offset, current_mode->size - offset, &code[2], code[1]);
                if ((uint8_t *)new_offset != NULL) {
                    // Pattern found, set found state flag
                    found = 1;
                    offset = new_offset - (size_t)current_mode->memory;
                }
                code += code[1] + 2;
                break;
            case OP_BACK:
                if (debug) {
#ifdef LOADER
                    log("back\n");
#else
                    fprintf(stderr, "back %hhu\n", code[1]);
#endif
                }
                offset -= code[1];
                code += 2;
                break;
            case OP_FWD:
                if (debug) {
#ifdef LOADER
                    log("fwd\n");
#else
                    fprintf(stderr, "fwd %u\n", code[1]);
#endif
                }
                offset += code[1];
                code += 2;
                break;
            case OP_SET: // Set data.
                if (debug) {
#ifdef LOADER
                    log("set\n");
#else
                    fprintf(stderr, "set %u, ...\n", code[1]);
#endif
                }
                memcpy(current_mode->memory + offset, &code[2], code[1]);
                offset += code[1];
                code += code[1] + 2;
                break;
            case OP_TEST: // Test data.
                if (debug) {
#ifdef LOADER
                    log("test\n");
#else
                    fprintf(stderr, "test %u, ...\n", code[1]);
#endif
                }
                eq = memcmp(current_mode->memory + offset, &code[2], code[1]);
                if (eq < 0)
                    lt = 1;
                if (eq > 0)
                    gt = 1;
                eq = !eq;
                code += code[1] + 2;
                break;
            case OP_JMP: // Jump to offset.
                code++;
                code = bytecode + (code[0] + (code[1] << 8));
                if (debug) {
#ifdef LOADER
                    log("jmp\n");
#else
                    fprintf(stderr, "jmp %u\n", code - bytecode);
#endif
                }
                break;
            case OP_JMPEQ: // Jump to offset if equal
                code++;
                if (eq)
                    code = bytecode + (code[0] + (code[1] << 8));
                else
                    code += 2;
                if (debug) {
#ifdef LOADER
                    log("jmpeq\n");
#else
                    fprintf(stderr, "jmpeq %u\n", code - bytecode);
#endif
                }
                break;
            case OP_JMPNE: // Jump to offset if not equal
                code++;
                if (!eq)
                    code = bytecode + (code[0] + (code[1] << 8));
                else
                    code += 2;
                if (debug) {
#ifdef LOADER
                    log("jmpne\n");
#else
                    fprintf(stderr, "jmpeq %u\n", code - bytecode);
#endif
                }
                break;
            case OP_JMPLT: // Jump to offset if less than
                code++;
                if (lt)
                    code = bytecode + (code[0] + (code[1] << 8));
                else
                    code += 2;
                if (debug) {
#ifdef LOADER
                    log("jmplt\n");
#else
                    fprintf(stderr, "jmplt %u\n", code - bytecode);
#endif
                }
                break;
            case OP_JMPGT: // Jump to offset if greater than
                code++;
                if (gt)
                    code = bytecode + (code[0] + (code[1] << 8));
                else
                    code += 2;
                if (debug) {
#ifdef LOADER
                    log("jmplt\n");
#else
                    fprintf(stderr, "jmplt %u\n", code - bytecode);
#endif
                }
                break;
            case OP_JMPLE: // Jump to offset if less than or equal
                code++;
                if (lt || eq)
                    code = bytecode + (code[0] + (code[1] << 8));
                else
                    code += 2;
                if (debug) {
#ifdef LOADER
                    log("jmplt\n");
#else
                    fprintf(stderr, "jmplt %u\n", code - bytecode);
#endif
                }
                break;
            case OP_JMPF: // Jump to offset if pattern found
                code++;
                if (found)
                    code = bytecode + (code[0] + (code[1] << 8));
                else
                    code += 2;
                if (debug) {
#ifdef LOADER
                    log("jmplt\n");
#else
                    fprintf(stderr, "jmplt %u\n", code - bytecode);
#endif
                }
                break;
            case OP_JMPNF: // Jump to offset if pattern NOT found
                code++;
                if (!found)
                    code = bytecode + (code[0] + (code[1] << 8));
                else
                    code += 2;
                if (debug) {
#ifdef LOADER
                    log("jmplt\n");
#else
                    fprintf(stderr, "jmplt %u\n", code - bytecode);
#endif
                }
                break;
            case OP_CLF: // Clear flags.
                if (debug) {
                    log("clf\n");
                }
                code++;
                found = gt = lt = eq = 0;
                break;
            case OP_REWIND:
                if (debug)
                    log("rewind\n");
                code++;
                offset = 0;
                break;
            case OP_AND:
                if (debug) {
                    log("and\n");
                }
                for (i = 0; i < code[1]; i++) {
                    current_mode->memory[offset] &= code[i+2];
                }
                offset += code[1];
                code += code[1] + 2;
                break;
            case OP_OR:
                if (debug) {
                    log("or\n");
                }
                for (i = 0; i < code[1]; i++) {
                    current_mode->memory[offset] |= code[i+2];
                }
                offset += code[1];
                code += code[1] + 2;
                break;
            case OP_XOR:
                if (debug) {
                    log("xor\n");
                }
                for (i = 0; i < code[1]; i++) {
                    current_mode->memory[offset] ^= code[i+2];
                }
                offset += code[1];
                code += code[1] + 2;
                break;
            case OP_NOT:
                if (debug) {
                    log("not\n");
                }
                for (i = 0; i < code[1]; i++) {
                    current_mode->memory[offset] = ~current_mode->memory[offset];
                }
                offset += code[1];
                code += 2;
                break;
            case OP_VER:
                if (debug) {
                    log("ver\n");
                }
                code++;
                eq = memcmp(&ver, code, 2);
                if (eq < 0)
                    lt = 1;
                if (eq > 0)
                    gt = 1;
                eq = !eq;
                code += 2;
                break;
            case OP_N3DS:
                if (debug) {
                    log("n3ds\n");
                }
                code++;
                eq = is_n3ds;
                break;
            case OP_SEEK: // Jump to offset if greater than or equal
                code++;
                offset = (uint32_t)(code[0] + (code[1] << 8) + (code[2] << 16) + (code[3] << 24));
                if (debug) {
#ifdef LOADER
                    log("seek\n");
#else
                    fprintf(stderr, "seek %lu\n", offset);
#endif
                }
                code += 4;
                break;
            case OP_ABORT:
                code++;
                if (debug)
                    log("abort\n");

                abort("abort triggered, halting VM!\n");
                break;
            case OP_ABORTEQ:
                code++;
                if (debug)
                    log("aborteq\n");
                if (eq)
                    abort("eq flag not set, halting VM!\n");
                break;
            case OP_ABORTNE:
                code++;
                if (debug)
                    log("abortlt\n");
                if (!eq)
                    abort("eq flag not set, halting VM!\n");
                break;
            case OP_ABORTLT:
                code++;
                if (debug)
                    log("abortlt\n");
                if (lt)
                    abort("lt flag set, halting VM!\n");
                break;
            case OP_ABORTGT:
                code++;
                if (debug)
                    log("abortgt\n");
                if (gt)
                    abort("gt flag set, halting VM!\n");
                break;
            case OP_ABORTF:
                code++;
                if (debug)
                    log("abortf\n");
                if (found)
                    abort("f flag set, halting VM!\n");
                break;
            case OP_ABORTNF:
                code++;
                if (debug)
                    log("abortnf\n");
                if (!found)
                    abort("f flag is not set, halting VM!\n");
                break;
            case OP_NEXT:
                if (debug) {
                    log("next\n");
                }
                found = gt = lt = eq = 0;

                memset(stack, 0, stack_size); // Clear stack.
                top = stack_size - 1;

                bytecode = code + 1;
#ifndef LOADER
                set_mode = 3;
                current_mode = &modes[set_mode];
#else
                set_mode = 18;
                current_mode = &modes[set_mode];
#endif
                offset = new_offset = 0;
                code = bytecode;
                break;
            case OP_INJECT: // Read in data (from filename)
                if (debug) {
#ifdef LOADER
                    log("set\n");
#else
                    fprintf(stderr, "set %s\n", &code[1]);
#endif
                }

                char* fname = (char*)&code[1];
#ifdef LOADER
                (void)fname;
                // NYI
#else
                FILE* f = fopen(fname, "r");
                fread(current_mode->memory + offset, 1, fsize(f), f);
                offset += fsize(f);
                code += strlen(fname);
                fclose(f);
#endif
                break;
            default:
#ifndef LOADER
                // Panic; not proper opcode.
                fprintf(stderr, "Invalid opcode. State:\n"
                                "  Relative:  %lu\n"
                                "    Actual:  %lx:%lu\n"
                                "  Memory:    %lx\n"
                                "    Actual:  %lx\n"
                                "  Code Loc:  %lx\n"
                                "    Actual:  %lx\n"
                                "  Opcode:    %hhu\n",
                        (uint32_t)set_mode,
                        (uint32_t)current_mode->memory,
                        (uint32_t)current_mode->size,
                        (uint32_t)offset,
                        (uint32_t)(current_mode->memory + offset),
                        (uint32_t)(code - bytecode),
                        (uint32_t)code,
                        *code);
#endif
                abort("Halting startup.\n");
                break;
        }

        if (offset > current_mode->size) { // Went out of bounds. Error.
#ifndef LOADER
            fprintf(stderr, " -> %lx", offset);
#endif
            abort("seeked out of bounds\n");
        }

#ifndef LOADER
        if (debug) {
            fprintf(stderr, "l:%d g:%d e:%d f:%d m:%lu o:0x%lx\nc:0x%lx m:0x%lx n:%lx\n",
                lt, gt, eq, found,
                set_mode,
                (uint32_t)offset, (uint32_t)(code - bytecode), (uint32_t)(current_mode->memory + offset), (uint32_t)code);
            wait();
        }
#endif
    }

    return 0;
}

#ifdef LOADER
int
execb(uint64_t tid, uint16_t ver, uint8_t *text_mem, uint32_t text_len, uint8_t *data_mem, uint32_t data_len, uint8_t *ro_mem, uint32_t ro_len)
{
#else
int
execb(const char *filename, int build_cache)
{
    uint16_t ver = 0; // FIXME - Provide native_firm version
#endif
    uint32_t patch_len;
#ifdef LOADER
    char cache_path[] = PATH_LOADER_CACHE "/0000000000000000";

    hexdump_titleid(tid, cache_path);

    static uint8_t patch_dat[MAX_PATCHSIZE];

    Handle file;
    u32 total;

    // Open file.
    if (!R_SUCCEEDED(fileOpen(&file, ARCHIVE_SDMC, cache_path, FS_OPEN_READ))) {
        // Failed to open.
        return 0; // No patches.
    }

    log("  patch: ");
    log(cache_path);
    log("\n");

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
    modes[18].memory = text_mem;
    modes[18].size = text_len;

    // Set memory.
    modes[19].memory = data_mem;
    modes[19].size = data_len;

    // Set memory.
    modes[20].memory = ro_mem;
    modes[20].size = ro_len;

    log("  exec\n");

    uint8_t *patch_mem = (uint8_t *)patch_dat;
    patch_len = file_size;
#else
    struct system_patch *patch;
    uint8_t *patch_mem;
    // Read patch to scrap memory.

    FILE *f = fopen(filename, "r");
    if (!f) {
        // File wasn't found. The user didn't enable anything.
        return 0;
    }

    uint32_t len = fsize(f);

    uint8_t* patch_loc = malloc(len);

    fread(patch_loc, 1, len, f);
    fclose(f);

    if (build_cache == 1) {
        patch = (struct system_patch*)patch_loc;

        // Make sure various bits are correct.
        if (memcmp(patch->magic, "AIDA", 4)) {
            // Incorrect magic.
            return 1;
        }

        fprintf(stderr, "Cache: %s\n", patch->name);

        patch_mem = (uint8_t *)patch + sizeof(struct system_patch) + (patch->depends * 8) + (patch->titles * 8);
        patch_len = patch->size;

        if (patch->titles != 0) {
            // Not an error, per se, but it means this patch is meant for loader, not us.
            // Patches intended for use during boot will always be applied to zero titles.
            // We should generate a cache for loader in a file intended for titleid.
            uint8_t *title_buf = (uint8_t *)patch + sizeof(struct system_patch);

            fprintf(stderr, "  Version: %u\n", patch->version);

            for (uint32_t i = 0; i < patch->titles; i++) {
                char cache_path[] = PATH_LOADER_CACHE "/0000000000000000";

                uint64_t title = 0;
                memcpy(&title, &title_buf[i * 8], 8);

                uint32_t tlen = strlen(cache_path) - 1;
                int j = 16;
                while (j--) {
                    cache_path[tlen--] = hexDigits[title & 0xF];
                    title >>= 4;
                }

                fprintf(stderr, "  cache: %s\n", &cache_path[strlen(cache_path) - 16]);

                char reset = 0xFF;

                FILE *cache = fopen(cache_path, "w");
                fseek(cache, 0, SEEK_END);
                fwrite(patch_mem, 1, patch_len, cache);
                fwrite(&reset, 1, 1, cache);
                fclose(cache);
                // Add to cache.
            }
        } else {
            // BOOT patch
            char cache_path[] = PATH_LOADER_CACHE "/BOOT";
            char reset = 0xFF;

            FILE *cache = fopen(cache_path, "w");
            fseek(cache, 0, SEEK_END);
            fwrite(patch_mem, 1, patch_len, cache);
            fwrite(&reset, 1, 1, cache);
            fclose(cache);
        }

        return 0;
    } else {
        patch_mem = patch_loc;
        patch_len = len;
    }
#endif

    int debug = 0;
#ifdef LOADER
    if (config.options[OPTION_OVERLY_VERBOSE]) {
#else
    if (get_opt_u32(OPTION_OVERLY_VERBOSE)) {
#endif
        debug = 1;
    }

#ifndef LOADER
    if (stack_glob == NULL) {
        stack_glob = malloc(STACK_SIZE);
    }
#endif

    return exec_bytecode(patch_mem, patch_len, stack_glob, STACK_SIZE, ver, debug);
}
