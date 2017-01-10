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

// TODO - Add to python assembler
#define OP_INJECT 0x90

#define OP_STR    0x91

#define OP_NEXT 0xFF

#ifdef LOADER
  #define log(a) logstr(a)
  #define panic(a)                                                                                                                                               \
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
#endif

int
exec_bytecode(uint8_t *bytecode, uint32_t len, uint16_t ver, int debug)
{
    uint32_t set_mode = 0;

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
            case OP_SEEK: // Jump to offset if greater than or equal
                code++;
                offset = (uint32_t)(code[0] + (code[1] << 8) + (code[2] << 16) + (code[3] << 24));
                if (debug) {
#ifdef LOADER
                    log("seek\n");
#else
                    fprintf(stderr, "seek %lx\n", offset);
#endif
                }
                code += 4;
                break;
            case OP_ABORT:
                code++;
                if (debug)
                    log("abort\n");

                panic("abort triggered, halting VM!\n");
                break;
            case OP_ABORTEQ:
                code++;
                if (debug)
                    log("aborteq\n");
                if (eq)
                    panic("eq flag not set, halting VM!\n");
                break;
            case OP_ABORTNE:
                code++;
                if (debug)
                    log("abortlt\n");
                if (!eq)
                    panic("eq flag not set, halting VM!\n");
                break;
            case OP_ABORTLT:
                code++;
                if (debug)
                    log("abortlt\n");
                if (lt)
                    panic("lt flag set, halting VM!\n");
                break;
            case OP_ABORTGT:
                code++;
                if (debug)
                    log("abortgt\n");
                if (gt)
                    panic("gt flag set, halting VM!\n");
                break;
            case OP_ABORTF:
                code++;
                if (debug)
                    log("abortf\n");
                if (found)
                    panic("f flag set, halting VM!\n");
                break;
            case OP_ABORTNF:
                code++;
                if (debug)
                    log("abortnf\n");
                if (!found)
                    panic("f flag is not set, halting VM!\n");
                break;
            case OP_NEXT:
                if (debug) {
                    log("next\n");
                }
                found = gt = lt = eq = 0;

                bytecode = code + 1;
#ifndef LOADER
                set_mode = 0;
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
                FILE* f = cropen(fname, "r");
                crread(current_mode->memory + offset, 1, crsize(f), f);
                offset += crsize(f);
                code += strlen(fname);
                crclose(f);
#endif
                break;
            case OP_STR:
                ++code;
#ifdef LOADER
                log((char*)code);
                log("\n");
#else
                fprintf(stderr, "%s\n", code);
#endif
                code += strlen((char*)code) + 1;
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
                panic("Halting startup.\n");
                break;
        }

        if (offset > current_mode->size) { // Went out of bounds. Error.
#ifndef LOADER
            fprintf(stderr, " -> %lx", offset);
#endif
            panic("seeked out of bounds\n");
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
execb(uint64_t tid, uint16_t ver, EXHEADER_prog_addrs* shared)
{
#else
int
execb(uint64_t tid, firm_h* firm_patch)
{
    uint16_t ver = 0; // FIXME - Provide native_firm version
#endif
    uint32_t patch_len;
#ifdef LOADER
    char cache_path[] = PATH_LOADER_CACHE "/0000000000000000";

    hexdump_titleid(tid, cache_path);

    static uint8_t *patch_mem;

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

    patch_mem = malloc(file_size);
    if (patch_mem == NULL) {
        log("  out of memory on loading patch\n");

        FSFILE_Close(file); // Read to memory.

        return 1;
    }

    // Read file.
    if (!R_SUCCEEDED(FSFILE_Read(file, &total, 0, patch_mem, file_size))) {
        FSFILE_Close(file); // Read to memory.

        // Failed to read.
        return 1;
    }

    FSFILE_Close(file); // Done reading in.

    // Set memory.
    modes[0].memory = (uint8_t*)shared->text_addr;
    modes[0].size   = shared->total_size << 12;

    // Set memory.
    modes[1].memory = (uint8_t*)shared->text_addr;
    modes[1].size   = shared->text_size << 12;

    // Set memory.
    modes[2].memory = (uint8_t*)shared->data_addr;
    modes[2].size   = shared->data_size << 12;

    // Set memory.
    modes[3].memory = (uint8_t*)shared->ro_addr;
    modes[3].size   = shared->ro_size << 12;

    log("  exec\n");

    patch_len = file_size;
#else
    // The WHOLE firm.
    modes[0].memory = (uint8_t *)firm_patch;
    modes[0].size   = firm_patch->section[0].size + firm_patch->section[1].size + sizeof(firm_h) +
                      firm_patch->section[2].size + firm_patch->section[3].size;

    // FIRM Sect 0
    modes[1].memory = (uint8_t *)firm_patch + firm_patch->section[0].offset;
    modes[1].size   = firm_patch->section[0].size;

    // FIRM Sect 1
    modes[2].memory = (uint8_t *)firm_patch + firm_patch->section[1].offset;
    modes[2].size   = firm_patch->section[1].size;

    // FIRM Sect 2
    modes[3].memory = (uint8_t *)firm_patch + firm_patch->section[2].offset;
    modes[3].size   = firm_patch->section[2].size;

    // FIRM Sect 3
    modes[4].memory = (uint8_t *)firm_patch + firm_patch->section[3].offset;
    modes[4].size   = firm_patch->section[3].size;

    char cache_path[] = PATH_LOADER_CACHE "/0000000000000000";

    uint64_t title = tid;

    uint32_t tlen = strlen(cache_path) - 1;
    int j = 16;
    while (j--) {
        cache_path[tlen--] = hexDigits[title & 0xF];
        title >>= 4;
    }

    // Read patch to scrap memory.

    FILE *f = cropen(cache_path, "r");
    if (!f) {
        // File wasn't found. The user didn't enable anything.
        return 0;
    }

    patch_len = crsize(f);

    uint8_t* patch_mem = memalign(16, patch_len);

    crread(patch_mem, 1, patch_len, f);
    crclose(f);
#endif

    int debug = 0;
#ifdef LOADER
    if (config.options[OPTION_OVERLY_VERBOSE]) {
#else
    if (get_opt_u32(OPTION_OVERLY_VERBOSE)) {
#endif
        debug = 1;
    }

    int r = exec_bytecode(patch_mem, patch_len, ver, debug);

    free(patch_mem);

    return r;
}

#ifndef LOADER

int cache_patch(const char *filename) {
    uint16_t ver = 0; // FIXME - Provide native_firm version
    uint32_t patch_len;

    struct system_patch *patch;
    uint8_t *patch_mem;
    // Read patch to scrap memory.

    FILE *f = cropen(filename, "r");
    if (!f) {
        // File wasn't found. The user didn't enable anything.
        return 0;
    }

    uint32_t len = crsize(f);

    uint8_t* patch_loc = malloc(len);

    crread(patch_loc, 1, len, f);
    crclose(f);

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

            size_t len_nam = 4 + strlen(patch->name);
            char *write = malloc(len_nam);
            write[0] = OP_NEXT;
            write[1] = OP_STR;
            memcpy(write+2, patch->name, len_nam - 4);
            write[len_nam - 2] = 0;
            write[len_nam - 1] = OP_NEXT;

            FILE *cache = cropen(cache_path, "w");
            crseek(cache, 0, SEEK_END);
            crwrite(write, 1, len_nam, cache);
            crwrite(patch_mem, 1, patch_len, cache);
            crclose(cache);
            // Add to cache.
        }
    }

    return 0;
}

#endif
