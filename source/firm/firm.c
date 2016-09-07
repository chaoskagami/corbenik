#include <stdint.h>
#include <stddef.h>

#include <ctr9/io.h>
#include <ctr9/aes.h>
#include <ctr9/sha.h>
#include <common.h>

#define FIRM_INTERNAL_CODE
#include <firm/internal.h>

firm_h*
load_firm(const char *path, size_t *size_out)
{
    int success = 0;

    FILE *firm_file = fopen(path, "r");
    if (!firm_file) {
        return NULL;
    }

    size_t size = fsize(firm_file);

    if (size_out)
        *size_out = size;

    uint8_t* mem = malloc(size);

    firm_h *firm = (firm_h*)mem;

    fread(mem, 1, size, firm_file);

    fclose(firm_file);

    if (!memcmp(firm->magic, "DECFIRM", 7)) {
        // Fully decrypted FIRM, courtesy D9. Fix the entrypoint and we're good.
        firm = (firm_h*)mem;

        struct firm_signature* sig = get_firm_info(firm);

        patch_entry(firm, sig->type);
        if (patch_section_keys(firm, sig->k9l)) {
            free(mem);
            return NULL;
        }

        free(sig);
    } else if (!memcmp(firm->magic, "FIRM", 4)) {
        // O3DS fully decrypted FIRM

        firm = (firm_h*)mem;
    } else {
        // Encrypted.
        char *cetk_path = strdupcat(path, ".cetk");

        uint8_t* firmkey = get_titlekey(cetk_path);

        free(cetk_path);

        if (firmkey) {
            firm = extract_firm_from_ncch((ncch_h*)mem, firmkey, size);

            if (firm) {
                struct firm_signature* sig = get_firm_info(firm);

                if (sig->console == console_n3ds) {
                    if(dec_k9l(firm)) {
                        free(firm);
                        free(mem);
                        return NULL;
                    }

                    patch_entry(firm, sig->type);

                    if (sig->type == type_native && patch_section_keys(firm, sig->k9l)) {
                        free(firm);
                        free(mem);
                        return NULL;
                    }
                }

                free(sig);
            }
        }
        free(mem);
    }

    return firm;
}

int
prepatch_firm(const char* firm_path, const char* prepatch_path, const char* module_path)
{
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

    if (patch_firm_all(tid, firm, module_path)) {
		free(firm);
        return 1;
	}

	FILE* f = fopen(prepatch_path, "w");
	fwrite(firm, 1, size, f);
	fclose(f);

	free(firm);

	return 0;
}

int
boot_firm(const char* firm_path, const char* prepatch_path, const char* module_path)
{
	size_t size = 0;
    firm_h* firm = load_firm(firm_path, &size);

    if (firm == NULL)
        return 1;

    struct firm_signature *sig = get_firm_info(firm);

    uint64_t tid = 0x0004013800000002LLu;

    if (sig->console == console_n3ds)
        tid += 0x20000000LLu;

    tid += sig->type * 0x100LLu;

    free(sig);

    if (patch_firm_all(tid, firm, module_path)) {
		free(firm);
        return 1;
	}

	FILE* f = fopen(prepatch_path, "w");
	fwrite(firm, 1, size, f);
	fclose(f);

    firmlaunch(firm); // <- should NOT return if all is well

    return 1;
}
