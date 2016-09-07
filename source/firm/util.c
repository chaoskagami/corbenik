#include <stdint.h>
#include <stddef.h>

#include <ctr9/io.h>
#include <ctr9/aes.h>
#include <ctr9/sha.h>
#include <common.h>

#define FIRM_INTERNAL_CODE
#include <firm/internal.h>

int decrypt_k9l(arm9bin_h *header, enum firm_type type, uint32_t k9l) {
    uint8_t slot = 0x15;

    if (type == type_native && k9l) {
        uint8_t decrypted_keyx[AES_BLOCK_SIZE];

        slot = 0x16;

        set_N11_K9L(k9l - 1);

        ecb_decrypt(header->slot0x16keyX, decrypted_keyx, 1, AES_CNT_ECB_DECRYPT_MODE);
        setup_aeskeyX(slot, decrypted_keyx);
    }

    setup_aeskeyY(slot, header->keyy);

    set_ctr(header->ctr);

    void *arm9bin = (uint8_t *)header + 0x800;
    int size = atoi(header->size); // Size is plaintext, don't ask me *shrug*

    use_aeskey(slot);

    ctr_decrypt(arm9bin, arm9bin, (size_t)size / AES_BLOCK_SIZE, AES_CNT_CTRNAND_MODE, header->ctr);

    if (type == type_native)
        return *(uint32_t *)arm9bin != ARM9BIN_MAGIC;

    return *(uint32_t *)arm9bin != LGY_ARM9BIN_MAGIC;
}

int patch_entry(firm_h *firm, enum firm_type type) {
    if (type == type_native)
        firm->a9Entry = 0x0801B01C;
    else
        firm->a9Entry = 0x0801301C;

    return 0;
}

uint8_t*
get_titlekey(char *cetk_filename)
{
    FILE* f = fopen(cetk_filename, "r");
    size_t size = fsize(f);

    uint8_t* cetk = malloc(size);

    fread(cetk, 1, size, f);

    fclose(f);

	uint8_t iv[AES_BLOCK_SIZE] = { 0 };
	uint32_t sigtype = __builtin_bswap32(*(const uint32_t *)cetk);

	if (sigtype != SIG_TYPE_RSA2048_SHA256) {
        free(cetk);
		return NULL;
    }

	const ticket_h *ticket = (const ticket_h *)((const uint8_t*)cetk + sizeof(sigtype) + 0x13C);

    set_Y3D_cetk(1);

    uint8_t *key = malloc(AES_BLOCK_SIZE);

	memcpy(iv, ticket->titleID, sizeof(ticket->titleID));
	memcpy(key, ticket->titleKey, sizeof(ticket->titleKey));

	cbc_decrypt(key, key, 1, AES_CNT_TITLEKEY_DECRYPT_MODE, iv);

    free(cetk);

	return key;
}

int dec_k9l(firm_h* firm) {
    firm_section_h *arm9 = NULL;
    for (firm_section_h* section = firm->section; section < firm->section + 4; section++) {
        if (section->type == FIRM_TYPE_ARM9) { // ARM9
            arm9 = section;
            break;
        }
    }

    if (arm9 == NULL)
        return 1;

    struct firm_signature* sig = get_firm_info(firm);

    if (decrypt_k9l((arm9bin_h*)((uint8_t*)firm + arm9->offset), sig->type, sig->k9l))
        return 1;

    // Recalc section hash.
    sha256sum(arm9->hash, (uint8_t*)firm + arm9->offset, arm9->size);

    // Magic like D9.
    memcpy(firm->magic, "DECFIRM", 7);

    free(sig);

    return 0;
}

firm_h*
extract_firm_from_ncch(ncch_h *ncch, uint8_t *titlekey, size_t size)
{
	uint8_t firm_iv[16] = { 0 };
	uint8_t exefs_key[16] = { 0 };
	uint8_t exefs_iv[16] = { 0 };

	setup_aeskey(0x16, titlekey);
	use_aeskey(0x16);
	cbc_decrypt(ncch, ncch, size / AES_BLOCK_SIZE, AES_CNT_CBC_DECRYPT_MODE, firm_iv);

	if (ncch->magic != NCCH_MAGIC) {
		return NULL;
    }

	memcpy(exefs_key, ncch, AES_BLOCK_SIZE);
	ncch_getctr(ncch, exefs_iv, NCCHTYPE_EXEFS);

	// Get the exefs offset and size from the NCCH
	exefs_h *exefs = (exefs_h *)((uint8_t *)ncch + ncch->exeFSOffset * MEDIA_UNITS);
	uint32_t exefs_size = ncch->exeFSSize * MEDIA_UNITS;

	setup_aeskeyY(0x2C, exefs_key);
	use_aeskey(0x2C);
	ctr_decrypt(exefs, exefs, exefs_size / AES_BLOCK_SIZE, AES_CNT_CTRNAND_MODE, exefs_iv);

	// Get the decrypted FIRM
	// We assume the firm.bin is always the first file
	firm_h *firm = (firm_h *)&exefs[1]; // The offset right behind the exefs

	// header; the first file.
	size = exefs->fileHeaders[0].size;
	if (memcmp(firm->magic, "FIRM", 4)) {
		return NULL;
    }

    firm_h* dest = malloc(size);

	memcpy(dest, firm, size);

	return dest;
}

uint8_t* key_search(uint8_t* mem, uint32_t size, uint8_t* sha256, uint8_t byte) {
	uint8_t hash[0x20] = {0};
	// Search for key.
	for(uint32_t j = 0; j < size; j++) {
		// Is candidate?
		if (mem[j] == byte) {
			// Yes. Check hash.
			sha256sum(hash, &mem[j], 0x10);
			if(!memcmp(sha256, hash, 0x20)) {
				return &mem[j];
			}
		}
	}
	return NULL;
}

void* find_section_key(firm_h* firm_loc) {
	uint8_t sha256[] = {0xb9, 0x4d, 0xb1, 0xb1, 0xc3, 0xe0, 0x11, 0x08, 0x9c, 0x19, 0x46, 0x06, 0x4a, 0xbc, 0x40, 0x2a,
	                    0x7c, 0x66, 0xf4, 0x4a, 0x74, 0x6f, 0x71, 0x50, 0x32, 0xfd, 0xff, 0x03, 0x74, 0xd7, 0x45, 0x2c};

	uint8_t* key_loc = (uint8_t*)firm_loc + firm_loc->section[2].offset;
	uint32_t search_size = firm_loc->section[2].size;

	uint8_t* key_data = key_search(key_loc, search_size, sha256, 0xDD);

	if (!key_data)
		return NULL;

	return key_data;
}

int patch_section_keys(firm_h* firm_loc, uint32_t k9l) {
    // Set up the keys needed to boot a few firmwares, due to them being unset,
    // depending on which firmware you're booting from.

    uint8_t *keydata = find_section_key(firm_loc);
    if (!keydata)
        return 1;

    set_N11_K9L(k9l - 1);

    uint8_t keyx[AES_BLOCK_SIZE];
    for (int slot = 0x19; slot < 0x20; slot++) {
        ecb_decrypt(keydata, keyx, 1, AES_CNT_ECB_DECRYPT_MODE);
        setup_aeskeyX(slot, keyx);
        *(uint8_t *)(keydata + 0xF) += 1;
    }

    return 0;
}


exefs_h*
find_proc9(firm_h *firm)
{
	for (firm_section_h *section = firm->section; section < firm->section + 4; section++) {
		if (section->address == 0)
			break;
		if (section->type == FIRM_TYPE_ARM9) {
			uint8_t* arm9section = memfind((uint8_t *)firm + section->offset, section->size, "Process9", 8);
			if (!arm9section)
				return NULL;

			ncch_h *ncch = (ncch_h *)((uint8_t*)arm9section - sizeof(ncch_h));
			if (ncch->magic == NCCH_MAGIC) {
				// Found Process9
				ncch_ex_h *p9exheader = (ncch_ex_h *)(ncch + 1);
				return (exefs_h *)(p9exheader + 1);
			}
		}
	}
	return NULL;
}
