#include <stdint.h>
#include <stddef.h>

#include <ctr9/io.h>
#include <ctr9/aes.h>
#include <ctr9/sha.h>
#include <common.h>

firm_h *firm_loc = (firm_h *)FCRAM_FIRM_LOC;
uint32_t firm_size = FCRAM_SPACING;
firm_section_h firm_proc9;
exefs_h *firm_p9_exefs;

firm_h *twl_firm_loc = (firm_h *)FCRAM_TWL_FIRM_LOC;
uint32_t twl_firm_size = FCRAM_SPACING * 2;
firm_section_h twl_firm_proc9;
exefs_h *twl_firm_p9_exefs;

firm_h *agb_firm_loc = (firm_h *)FCRAM_AGB_FIRM_LOC;
uint32_t agb_firm_size = FCRAM_SPACING * 2;
firm_section_h agb_firm_proc9;
exefs_h *agb_firm_p9_exefs;

firm_h* firm0 = NULL;
firm_h* firm1 = NULL;

static int update_96_keys = 0;

static volatile uint32_t *const a11_entry = (volatile uint32_t *)0x1FFFFFF8;

// Fwd decl
int decrypt_arm9bin(arm9bin_h *header, uint64_t firm_title, uint8_t version);

#define SECTOR_SIZE 0x200

// 0x0B130000 = start of FIRM0 partition, 0x400000 = size of FIRM partition (4MB)
void dump_firm(firm_h** buffer, uint8_t index) {
    if (*buffer != NULL) return;

    // NOTE - Cast, because GCC is making assumptions about 'index'.
    uint32_t firm_offset = (uint32_t)(0x0B130000 + (index % 2) * 0x400000),
             firm_b_size = 0x00100000; // 1MB, because

    buffer[0] = static_allocate(firm_b_size);

    uint8_t ctr[0x10],
            cid[0x10],
            sha_t[0x20];

    firm_h* firm = buffer[0];

    if (sdmmc_nand_readsectors(firm_offset / SECTOR_SIZE, firm_b_size / SECTOR_SIZE, (uint8_t*)firm))
        abort("  Failed to read NAND!\n");

    fprintf(stderr, "  Read FIRM%u off NAND.\n", index);

    sdmmc_get_cid(1, (uint32_t*)cid);
    sha256sum(sha_t, cid, 0x10);
    memcpy(ctr, sha_t, 0x10);
    add_ctr(ctr, firm_offset / AES_BLOCK_SIZE);

    use_aeskey(0x06);
    set_ctr(ctr);
	ctr_decrypt(firm, firm, firm_b_size / AES_BLOCK_SIZE, AES_CTR_MODE|AES_CNT_INPUT_ENDIAN|AES_CNT_OUTPUT_ENDIAN|AES_CNT_INPUT_ORDER|AES_CNT_OUTPUT_ORDER, ctr);

    if (memcmp((char*) & firm->magic, "FIRM", 4))
        abort("  Decryption failed on FIRM.\n");

    fprintf(stderr, "  AES decrypted FIRM%u.\n", index);

    fprintf(stderr, "  Magic is intact on FIRM%u.\n", index);

    uint8_t detver = 0;

    if(index == 1)
        detver = 0x10;

    if(decrypt_arm9bin((arm9bin_h*)((uint8_t*)firm + firm->section[2].offset), NATIVE_FIRM_TITLEID, detver)) {
        abort("  Failed to decrypt FIRM%u arm9loader.\n", index);
    }

    fprintf(stderr, "  Decrypted FIRM%u arm9loader.\n", index);
}

void
slot0x11key96_init()
{
    // 9.6 crypto may need us to get the key from somewhere else.
    // Unless the console already has the key initialized, that is.
    uint8_t key[AES_BLOCK_SIZE];
    if (read_file(key, PATH_SLOT0X11KEY96, AES_BLOCK_SIZE) != 0 || read_file(key, PATH_ALT_SLOT0X11KEY96, AES_BLOCK_SIZE) != 0) {
        // Read key successfully.
        setup_aeskey(0x11, key);

        // Tell boot_firm it needs to regenerate the keys.
        update_96_keys = 1;
    }
}

uint8_t* key_search(uint8_t* mem, uint32_t size, uint8_t* sha256, uint8_t byte) {
    uint8_t hash[0x20] = {0};

    // Search ARM9 for key.
    for(uint32_t j = 0; j < size; j ++) {
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

void extract_slot0x05keyY() {
    if (firm0 == NULL)
        dump_firm(&firm0, 0);

    uint8_t sha256[] = {0x98, 0x24, 0x27, 0x14, 0x22, 0xB0, 0x6B, 0xF2, 0x10, 0x96, 0x9C, 0x36, 0x42, 0x53, 0x7C, 0x86,
                        0x62, 0x22, 0x5C, 0xFD, 0x6F, 0xAE, 0x9B, 0x0A, 0x85, 0xA5, 0xCE, 0x21, 0xAA, 0xB6, 0xC8, 0x4D};

    uint8_t* key_loc     = (uint8_t*)firm0 + firm0->section[2].offset;
    uint32_t search_size = firm0->section[2].size;

    uint8_t mem[16] __attribute__((aligned(4))) = {0};

    uint8_t* key_data = key_search(key_loc, search_size, sha256, 0x4D);

    if (!key_data)
        abort("  0x05 KeyY not found!\n");

    fprintf(stderr, "  0x05 KeyY at %lx in FIRM1\n", (uint32_t)key_data - (uint32_t)key_loc);

    memcpy(mem, key_data, 16);

    setup_aeskeyY(0x05, mem);
}

void extract_slot0x3DkeyY() {
    if (firm0 == NULL)
        dump_firm(&firm0, 0);

    uint8_t sha256[] = {0x21, 0x12, 0xf4, 0x50, 0x78, 0x6d, 0xce, 0x64, 0x39, 0xfd, 0xb8, 0x71, 0x14, 0x74, 0x41, 0xf4,
                        0x69, 0xb6, 0xc4, 0x70, 0xa4, 0xb1, 0x5f, 0x7d, 0xfd, 0xe8, 0xcc, 0xe4, 0xc4, 0x62, 0x82, 0x5b};

    uint8_t* key_loc     = (uint8_t*)firm0 + firm0->section[2].offset;
    uint32_t search_size = firm0->section[2].size;

    uint8_t mem[16] __attribute__((aligned(4))) = {0};

    uint8_t* key_data = key_search(key_loc, search_size, sha256, 0x0C);

    if (!key_data)
        abort("  0x3D KeyY #1 not found!\n");

    fprintf(stderr, "  0x3D KeyY #1 at %lx in FIRM0\n", (uint32_t)key_data - (uint32_t)key_loc);

    memcpy(mem, key_data, 16);

    setup_aeskeyY(0x3D, mem);
}

void* find_section_key() {
    // The key will be dword-aligned (I think? Verify this. May need new NFIRM to check assumption. Go, Nintendo!)

    // The hash of the key. Can't give the key itself out, obviously.
    uint8_t sha256[] = {0xb9, 0x4d, 0xb1, 0xb1, 0xc3, 0xe0, 0x11, 0x08, 0x9c, 0x19, 0x46, 0x06, 0x4a, 0xbc, 0x40, 0x2a,
                        0x7c, 0x66, 0xf4, 0x4a, 0x74, 0x6f, 0x71, 0x50, 0x32, 0xfd, 0xff, 0x03, 0x74, 0xd7, 0x45, 0x2c};
    uint8_t* key_loc = (uint8_t*)firm_loc + firm_loc->section[2].offset;
    uint32_t search_size = firm_loc->section[2].size;

    uint8_t* key_data = key_search(key_loc, search_size, sha256, 0xDD);

    if (!key_data)
        abort("  FIRM Section key not found!\n");

    fprintf(stderr, "  FIRM Section key at %lx in FIRM\n", (uint32_t)key_data - (uint32_t)key_loc);

    return key_data;
}

int
decrypt_cetk_key(void *key, const void *cetk)
{
	static int got_cetk = 0;
	uint8_t iv[AES_BLOCK_SIZE] = { 0 };
	uint32_t sigtype = __builtin_bswap32(*(uint32_t *)cetk);

	if (sigtype != SIG_TYPE_RSA2048_SHA256)
		return 1;

	ticket_h *ticket = (ticket_h *)(cetk + sizeof(sigtype) + 0x13C);
	if (ticket->ticketCommonKeyYIndex != 1)
		return 1;

	if (got_cetk == 0) {
		fprintf(stderr, "  Retrieving 0x3D KeyY...\n");
		extract_slot0x3DkeyY();
		got_cetk = 1;
	}

	use_aeskey(0x3D);

	memcpy(iv,  ticket->titleID,  sizeof(ticket->titleID));
	memcpy(key, ticket->titleKey, sizeof(ticket->titleKey));

	set_ctr(iv);
	cbc_decrypt(key, key, 1, AES_CBC_DECRYPT_MODE|AES_CNT_INPUT_ENDIAN|AES_CNT_OUTPUT_ENDIAN|AES_CNT_INPUT_ORDER|AES_CNT_OUTPUT_ORDER, iv);

	fprintf(stderr, "  Extracted titlekey from cetk.\n");

	return 0;
}

int
decrypt_firm_title(firm_h *dest, ncch_h *ncch, uint32_t *size, void *key)
{
    uint8_t firm_iv[16] = { 0 };
    uint8_t exefs_key[16] = { 0 };
    uint8_t exefs_iv[16] = { 0 };

    fprintf(stderr, "  Decrypting FIRM container (size is %u blocks)\n", *size / AES_BLOCK_SIZE);

    setup_aeskey(0x16, key);
    use_aeskey(0x16);

    cbc_decrypt(ncch, ncch, *size / AES_BLOCK_SIZE, AES_CBC_DECRYPT_MODE|AES_CNT_INPUT_ENDIAN|AES_CNT_OUTPUT_ENDIAN|AES_CNT_INPUT_ORDER|AES_CNT_OUTPUT_ORDER, firm_iv);

    if (ncch->magic != NCCH_MAGIC)
        return 1;

    memcpy(exefs_key, ncch, 16);

    ncch_getctr(ncch, exefs_iv, NCCHTYPE_EXEFS);

    // Get the exefs offset and size from the NCCH
    exefs_h *exefs = (exefs_h *)((uint8_t *)ncch + ncch->exeFSOffset * MEDIA_UNITS);
    uint32_t exefs_size = ncch->exeFSSize * MEDIA_UNITS;

    fprintf(stderr, "  Decrypting ExeFs for FIRM (size is %u blocks)\n", exefs_size / 16);

    setup_aeskeyY(0x2C, exefs_key);
    use_aeskey(0x2C);
    ctr_decrypt(exefs, exefs, exefs_size / AES_BLOCK_SIZE, AES_CTR_MODE|AES_CNT_INPUT_ENDIAN|AES_CNT_OUTPUT_ENDIAN|AES_CNT_INPUT_ORDER|AES_CNT_OUTPUT_ORDER, exefs_iv);

    // Get the decrypted FIRM
    // We assume the firm.bin is always the first file
    firm_h *firm = (firm_h *)&exefs[1]; // The offset right behind the exefs
                                        // header; the first file.
    *size = exefs->fileHeaders[0].size;

    if (firm->magic != FIRM_MAGIC)
        return 1;

    memcpy(dest, firm, *size);

    return 0;
}

int
decrypt_arm9bin(arm9bin_h *header, uint64_t firm_title, uint8_t version)
{
    uint8_t slot = 0x15;

    if (firm_title == NATIVE_FIRM_TITLEID && version > 0x0F) {
        uint8_t decrypted_keyx[AES_BLOCK_SIZE];

        slot0x11key96_init();
        slot = 0x16;

        use_aeskey(0x11);
        aes_decrypt(decrypted_keyx, header->slot0x16keyX, 1, AES_ECB_DECRYPT_MODE|AES_CNT_INPUT_ENDIAN|AES_CNT_OUTPUT_ENDIAN|AES_CNT_INPUT_ORDER|AES_CNT_OUTPUT_ORDER);
        setup_aeskeyX(slot, decrypted_keyx);
    }

    setup_aeskeyY(slot, header->keyy);
    set_ctr(header->ctr);

    void *arm9bin = (uint8_t *)header + 0x800;
    int size = atoi(header->size);

    use_aeskey(slot);
    ctr_decrypt(arm9bin, arm9bin, size / AES_BLOCK_SIZE, AES_CTR_MODE|AES_CNT_INPUT_ENDIAN|AES_CNT_OUTPUT_ENDIAN|AES_CNT_INPUT_ORDER|AES_CNT_OUTPUT_ORDER, header->ctr);

    if (firm_title == NATIVE_FIRM_TITLEID)
        return *(uint32_t *)arm9bin != ARM9BIN_MAGIC;

    else if (firm_title == AGB_FIRM_TITLEID || firm_title == TWL_FIRM_TITLEID)
        return *(uint32_t *)arm9bin != LGY_ARM9BIN_MAGIC;

    return 1;
}

int
decrypt_firm(firm_h *dest, char *path_firmkey, char *path_cetk, uint32_t *size)
{
    uint8_t firm_key[AES_BLOCK_SIZE];

    // Firmware is likely encrypted. Decrypt.
    if (!read_file(firm_key, path_firmkey, AES_BLOCK_SIZE)) {
        // Missing firmkey. Attempt to get from CETK (only works if system was booted)
        if (!read_file((void *)FCRAM_JUNK_LOC, path_cetk, FCRAM_SPACING) || decrypt_cetk_key(firm_key, (void *)FCRAM_JUNK_LOC)) {
            fprintf(stderr, "  No firmkey and failed to extract from cetk\n");
            return 1;
        } else {
            fprintf(stderr, "  Saving firmkey for future use.\n");
            write_file(firm_key, path_firmkey, AES_BLOCK_SIZE);
        }
    } else {
        fprintf(stderr, "  Read firmkey from filesystem.\n");
    }

    fprintf(stderr, "  Decrypting FIRM\n");
    if (decrypt_firm_title(dest, (void *)dest, size, firm_key) != 0) {
        fprintf(stderr, "  Failed to decrypt FIRM title.\n");
        return 1;
    }
    return 0;
}

extern int patch_services();

int
load_firm(firm_h *dest, char *path, char *path_firmkey, char *path_cetk, uint32_t *size, uint64_t firm_title)
{
    int status = 0;
    int firmware_changed = 0;

    if (read_file(dest, path, *size) == 0) {
        fprintf(stderr, "  FIRM file is missing.\n");
        return 1;
    } else {
        fprintf(stderr, "  Loaded FIRM off filesystem\n");
    }

    // Check and decrypt FIRM if it is encrypted.
    if (dest->magic != FIRM_MAGIC) {
        status = decrypt_firm(dest, path_firmkey, path_cetk, size);
        if (status != 0) {
            fprintf(stderr, "  Decryption seems to have failed\n");
            return 1;
        }
        firmware_changed = 1; // Decryption performed.
    } else {
        fprintf(stderr, "  FIRM is decrypted\n");
    }

    struct firm_signature *fsig = get_firm_info(dest);

    // The N3DS firm has an additional encryption layer for ARM9
    if (fsig->console == console_n3ds) {
        // Look for the arm9 section
        for (firm_section_h *section = dest->section; section < dest->section + 4; section++) {
            if (section->type == FIRM_TYPE_ARM9) {
                // Check whether the arm9bin is encrypted.
                int arm9bin_iscrypt = 0;
                uint32_t magic = *(uint32_t *)((uintptr_t)dest + section->offset + 0x800);
                if (firm_title == NATIVE_FIRM_TITLEID)
                    arm9bin_iscrypt = (magic != ARM9BIN_MAGIC);
                else if (firm_title == AGB_FIRM_TITLEID || firm_title == TWL_FIRM_TITLEID)
                    arm9bin_iscrypt = (magic != LGY_ARM9BIN_MAGIC);

                if (arm9bin_iscrypt) {
                    // Decrypt the arm9bin.
                    if (decrypt_arm9bin((arm9bin_h *)((uintptr_t)dest + section->offset), firm_title, fsig->version)) {
                        return 1;
                    }
                    firmware_changed = 1; // Decryption of arm9bin performed.
                } else {
                    fprintf(stderr, "  ARM9 segment is decrypted\n");
                    if (firm_title == NATIVE_FIRM_TITLEID && fsig->version > 0x0F) {
                        slot0x11key96_init(); // This has to be loaded
                                              // regardless, otherwise boot will
                                              // fail.
                    }
                }

                // We assume there's only one section to decrypt.
                break;
            }
        }
    }

    // Save firmware.bin if decryption was done.
    if (firmware_changed) {
        fprintf(stderr, "  Overwriting FIRM with decrypted FIRM\n");
        write_file(dest, path, *size);
    }

    if (fsig->console == console_n3ds) {
        fprintf(stderr, "  Patching arm9 entrypoint\n");

        // Patch the entrypoint to skip arm9loader
        if (firm_title == NATIVE_FIRM_TITLEID) {
            dest->a9Entry = 0x0801B01C;
        } else if (firm_title == AGB_FIRM_TITLEID || firm_title == TWL_FIRM_TITLEID) {
            dest->a9Entry = 0x0801301C;
        }
        // The entrypoints seem to be the same across different FIRM versions,
        //  so we don't change them.
    }

    return 0;
}

// FAIR WARNING; This function is arm11 code, not ARM9.
// They share enough in common that this isn't a problem,
// but still worth documenting.
void __attribute__((naked)) arm11_preboot_halt()
{
    *a11_entry = 0; // Don't wait for us

    // Disables the LCD.
    *(volatile uint32_t *)0x10202A44 = 0;
    *(volatile uint32_t *)0x10202244 = 0;
    *(volatile uint32_t *)0x1020200C = 0;
    *(volatile uint32_t *)0x10202014 = 0;

    while (!*a11_entry)
        ;
    ((void (*)()) * a11_entry)();
}

extern void wait();

void
boot_firm()
{
    struct firm_signature *fsig = get_firm_info(firm_loc);

    // Set up the keys needed to boot a few firmwares, due to them being unset,
    // depending on which firmware you're booting from.
    // TODO: Don't use the hardcoded offset.
    if (update_96_keys && fsig->console == console_n3ds && fsig->version > 0x0F) {
        uint8_t *keydata = find_section_key();
        if (!keydata) {
            abort("Couldn't find section key.\n");
        }

        wait();

        use_aeskey(0x11);
        uint8_t keyx[AES_BLOCK_SIZE];
        for (int slot = 0x19; slot < 0x20; slot++) {
            aes_decrypt(keyx, keydata, 1, AES_ECB_DECRYPT_MODE|AES_CNT_INPUT_ENDIAN|AES_CNT_OUTPUT_ENDIAN|AES_CNT_INPUT_ORDER|AES_CNT_OUTPUT_ORDER);
            setup_aeskeyX(slot, keyx);
            *(uint8_t *)(keydata + 0xF) += 1;
        }

        fprintf(stderr, "Updated keyX keyslots.\n");
    }

    for (firm_section_h *section = firm_loc->section; section < firm_loc->section + 4 && section->address != 0; section++) {
        memcpy((void *)section->address, (void *)((uint8_t*)firm_loc + section->offset), section->size);
    }
    fprintf(stderr, "Copied FIRM.\n");

    wait();

    clear_disp(stderr);
    set_cursor(stderr, 0, 0);

    fflush(stderr); // Flush logs if need be before unmount.

    fumount(); // Unmount SD. No longer needed.

    // No fprintf will work from here on out.

    *a11_entry = (uint32_t)arm11_preboot_halt;
    while (*a11_entry)
        ; // Make sure it jumped there correctly before changing it.
    *a11_entry = (uint32_t)firm_loc->a11Entry;

    ((void (*)())firm_loc->a9Entry)();
}

int
find_proc9(firm_h *firm, firm_section_h *process9, exefs_h **p9exefs)
{
    for (firm_section_h *section = firm->section; section < firm->section + 4; section++) {
        if (section->address == 0)
            break;

        if (section->type == FIRM_TYPE_ARM9) {
            uint8_t *arm9section = (uint8_t *)firm + section->offset;
            while (arm9section < arm9section + section->size) {
                if (!memcmp(arm9section, "Process9", 8)) { // Process9
                    ncch_h *ncch = (ncch_h *)((uint8_t*)arm9section - sizeof(ncch_h));
                    if (ncch->magic == NCCH_MAGIC) {
                        // Found Process9
                        ncch_ex_h *p9exheader = (ncch_ex_h *)(ncch + 1);
                        *p9exefs = (exefs_h *)(p9exheader + 1);
                        process9->address = p9exheader->sci.textCodeSet.address;
                        process9->size = (*p9exefs)->fileHeaders[0].size;
                        process9->offset = (uint32_t)((*p9exefs) + 1) - (uint32_t)firm;
                        fprintf(stderr, "  Found process9 offset\n");
                        return 0;
                    }
                }
                ++arm9section;
            }
        }
    }
    fprintf(stderr, "  Couldn't find Process9?\n");
    return 1;
}

int firm_loaded = 0;

int
load_firms()
{
    int state = 0;

    if (firm_loaded)
        return 0;

    fprintf(stderr, "FIRM load triggered.\n");

    fprintf(stderr, "Loading NATIVE_FIRM\n");
    if (load_firm(firm_loc, PATH_NATIVE_F, PATH_NATIVE_FIRMKEY, PATH_NATIVE_CETK, &firm_size, NATIVE_FIRM_TITLEID) != 0) {
        abort("\n  Failed to load NATIVE_FIRM.\n");
    }
    find_proc9(firm_loc, &firm_proc9, &firm_p9_exefs);
    fprintf(stderr, "  Ver: %x, %u\n", get_firm_info(firm_loc)->version, get_firm_info(firm_loc)->console );

    fprintf(stderr, "TWL_FIRM\n");
    if (load_firm(twl_firm_loc, PATH_TWL_F, PATH_TWL_FIRMKEY, PATH_TWL_CETK, &twl_firm_size, TWL_FIRM_TITLEID) != 0) {
        fprintf(stderr, "\n  TWL_FIRM failed to load.\n");
        state = 1;
    } else {
        find_proc9(twl_firm_loc, &twl_firm_proc9, &twl_firm_p9_exefs);
        fprintf(stderr, "  Ver: %x, %u\n", get_firm_info(twl_firm_loc)->version, get_firm_info(twl_firm_loc)->console );
    }

    fprintf(stderr, "AGB_FIRM\n");
    if (load_firm(agb_firm_loc, PATH_AGB_F, PATH_AGB_FIRMKEY, PATH_AGB_CETK, &agb_firm_size, AGB_FIRM_TITLEID) != 0) {
        fprintf(stderr, "\n  AGB_FIRM failed to load.\n");
        state = 1;
    } else {
        find_proc9(agb_firm_loc, &agb_firm_proc9, &agb_firm_p9_exefs);
        fprintf(stderr, "  Ver: %x, %u\n", get_firm_info(agb_firm_loc)->version, get_firm_info(agb_firm_loc)->console );
    }

    firm_loaded = 1; // Loaded.

    return state;
}

void
boot_cfw()
{
    fprintf(stderr, "Loading firmware...\n");

    load_firms();

    if (config.options[OPTION_RECONFIGURED]) {
        fprintf(stderr, "Generating patch cache...\n");
        generate_patch_cache();
    }

    fprintf(stderr, "Patching firmware...\n");
    if (patch_firm_all() != 0)
        return;

    if (config.options[OPTION_REBOOT] && config.options[OPTION_RECONFIGURED]) {
        fprintf(stderr, "Saving FIRM for reboot...\n");
        if (!write_file(firm_loc, PATH_NATIVE_P, firm_size))
            abort("Failed to save prepatched native\n");

        if (!write_file(twl_firm_loc, PATH_TWL_P, twl_firm_size))
            abort("Failed to save prepatched twl\n");

        if (!write_file(agb_firm_loc, PATH_AGB_P, agb_firm_size))
            abort("Failed to save prepatched agb\n");
    }

    boot_firm();
}
