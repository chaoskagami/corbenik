#include <stddef.h>         // for NULL, size_t
#include <stdint.h>         // for uint64_t, uint8_t
#include <stdlib.h>         // for free
#include <string.h>         // for memcmp
#include <malloc.h>         // for memalign
#include "firm/headers.h"   // for firm_h, ncch_h
#include "firm/keys.h"   // for firm_h, ncch_h
#include "firm/firm.h"      // for firm_signature, get_firm_info, consoles::...
#define FIRM_INTERNAL_CODE
#include <firm/internal.h>  // for dec_k9l, extract_firm_from_ncch, firmlaunch
#include "patcher.h"        // for patch_firm_all
#include "std/fs.h"         // for cropen, crclose, crwrite, crread, crsize
#include "std/memory.h"     // for strdupcat

firm_h*
load_firm(const char *path, size_t *size_out)
{
    uint8_t* mem;
    struct firm_signature* sig;
    int success = 0;
    FILE* firm_file;
    size_t size;
    int save_dec = 0;

    char* decpath = strdupcat(path, ".dec");
    firm_file = cropen(decpath, "r");
    if (!firm_file) {
        firm_file = cropen(path, "r");
        if (!firm_file) {
            return NULL;
        }
    }

    size = crsize(firm_file);

    if (size_out)
        *size_out = size;

    // FIXME - Temp fix; allocating way more memory than needed to prevent memory corruption
    mem = memalign(16, 0x1000000);

    firm_h *firm = (firm_h*)mem;

    crread(mem, 1, size, firm_file);

    crclose(firm_file);

    if (memcmp(firm->magic, "FIRM", 4)) {
        char *key_path = strdupcat(path, ".key");

        // Attempt to open keyfile.
        uint8_t* firmkey = memalign(16, 16);
        if (read_file(firmkey, key_path, 16) != 16) {
            // Keyfile couldn't be opened, try the cetk.
            free(firmkey);

            // Encrypted. Open CETK.
            char *cetk_path = strdupcat(path, ".cetk");
            firmkey = get_titlekey(cetk_path);
            free(cetk_path);

            // Save firmkey.
            FILE* keyfile = cropen(key_path, "w");
            crwrite(firmkey, 1, 16, keyfile);
            crclose(keyfile);
        }

        free(key_path);

        if (firmkey) {
            firm = extract_firm_from_ncch((ncch_h*)mem, firmkey, size);

            free(firmkey);

            if (!firm) {
                free(mem);
                return NULL;
            }
        } else {
            free(mem);
            return NULL;
        }

        save_dec = 1;
    }

    sig = get_firm_info(firm);

    if (memcmp(firm->magic, "FIRM", 4)) {
        // Error. Abort.
        free(mem);
        return NULL;
    }

    // If this is a FIRM and not a k9l decrypted FIRM...
    if (!memcmp(firm->magic, "FIRM", 4) && memcmp(firm->magic + 4, "DEC", 3)) {
        if (sig->console == console_n3ds) {
            if (dec_k9l(firm)) {
                free(firm);
                free(mem);
                return NULL;
            }
        }
    }

    // Save decrypted FIRM.
    if (save_dec == 1) {
        firm_file = cropen(decpath, "w");
        crwrite(firm, 1, size, firm_file);
        crclose(firm_file);
    }

    free(decpath);

    // Arm9 decrypted firmware (n3ds)?
    if (!memcmp(firm->magic, "FIRMDEC", 7)) {
        if (sig->console == console_n3ds) {
            patch_entry(firm, sig->type);

            if (sig->type == type_native && patch_section_keys(firm, sig->k9l)) {
                free(firm);
                free(mem);
                return NULL;
            }
        }
    }

    return firm;
}

int
prepatch_firm(const char* firm_path, const char* prepatch_path, const char* module_path)
{
    FILE* test = cropen(prepatch_path, "r");
    if (test) {
        // Already exists.
        crclose(test);
        return 0;
    }

    size_t size = 0;
    firm_h* firm = load_firm(firm_path, &size);

    if (firm == NULL)
        return 1;

    struct firm_signature *sig = get_firm_info(firm);

    uint64_t tid = 0x0004013800000002LLu;

    if (sig->console == console_n3ds)
        tid |= 0x20000000LLu;

    tid |= sig->type * 0x100LLu;

    free(sig);

    if (patch_firm_all(tid, &firm, module_path)) {
        free(firm);
        return 1;
    }

    FILE* f = cropen(prepatch_path, "w");
    crwrite(firm, 1, size, f);
    crclose(f);

    free(firm);

    return 0;
}

int
boot_firm(const char* firm_path, const char* prepatch_path, const char* module_path)
{
    firm_h* firm;

    size_t size = 0;
    firm = load_firm(firm_path, &size);

    if (firm == NULL)
        return 1;

    struct firm_signature *sig = get_firm_info(firm);

    uint64_t tid = 0x0004013800000002LLu;

    if (sig->console == console_n3ds)
        tid += 0x20000000LLu;

    tid += sig->type * 0x100LLu;

    free(sig);

    if (patch_firm_all(tid, &firm, module_path)) {
        free(firm);
        return 1;
    }

    FILE* f = cropen(prepatch_path, "w");
    crwrite(firm, 1, size, f);
    crclose(f);

    firmlaunch(firm); // <- should NOT return if all is well

    return 1;
}
