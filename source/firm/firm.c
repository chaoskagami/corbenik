#include "firm.h"

#include <stdint.h>
#include <stddef.h>

#include "../common.h"

firm_h *firm_loc = (firm_h *)FCRAM_FIRM_LOC;
static uint32_t firm_size = FCRAM_SPACING;

firm_h *twl_firm_loc = (firm_h *)FCRAM_TWL_FIRM_LOC;
static uint32_t twl_firm_size = FCRAM_SPACING * 2;

firm_h *agb_firm_loc = (firm_h *)FCRAM_AGB_FIRM_LOC;
static uint32_t agb_firm_size = FCRAM_SPACING;

static int update_96_keys = 0;
static int save_firm = 0;

static volatile uint32_t *const a11_entry = (volatile uint32_t *)0x1FFFFFF8;

void slot0x11key96_init()
{
    // 9.6 crypto may need us to get the key from somewhere else.
    // Unless the console already has the key initialized, that is.
    uint8_t key[AES_BLOCK_SIZE];
    if (read_file(key, PATH_SLOT0X11KEY96, AES_BLOCK_SIZE) ||
        read_file(key, PATH_ALT_SLOT0X11KEY96, AES_BLOCK_SIZE)) {
        // Read key successfully.
        aes_setkey(0x11, key, AES_KEYNORMAL, AES_INPUT_BE | AES_INPUT_NORMAL);

        // Tell boot_firm it needs to regenerate the keys.
        update_96_keys = 1;
    }
    // If we can't read the key, we assume it's not needed, and the firmware is the right version.
    // Otherwise, we make sure the error message for decrypting arm9bin mentions this.
}

int decrypt_firm_title(firm_h *dest, ncch_h *ncch, uint32_t *size, void *key)
{
    uint8_t firm_iv[16] = {0};
    uint8_t exefs_key[16] = {0};
    uint8_t exefs_iv[16] = {0};

    fprintf(BOTTOM_SCREEN, "Decrypting the NCCH\n");
    aes_setkey(0x16, key, AES_KEYNORMAL, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_use_keyslot(0x16);
    aes(ncch, ncch, *size / AES_BLOCK_SIZE, firm_iv, AES_CBC_DECRYPT_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);

    if (ncch->magic != NCCH_MAGIC)
        return 1;

    memcpy(exefs_key, ncch, 16);
    ncch_getctr(ncch, exefs_iv, NCCHTYPE_EXEFS);

    // Get the exefs offset and size from the NCCH
    exefs_h *exefs = (exefs_h *)((void *)ncch + ncch->exeFSOffset * MEDIA_UNITS);
    uint32_t exefs_size = ncch->exeFSSize * MEDIA_UNITS;

    fprintf(BOTTOM_SCREEN, "Decrypting the exefs\n");
    aes_setkey(0x2C, exefs_key, AES_KEYY, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_use_keyslot(0x2C);
    aes(exefs, exefs, exefs_size / AES_BLOCK_SIZE, exefs_iv, AES_CTR_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);

    // Get the decrypted FIRM
    // We assume the firm.bin is always the first file
    firm_h *firm = (firm_h *)&exefs[1];  // The offset right behind the exefs header; the first file.
    *size = exefs->fileHeaders[0].size;

    if (firm->magic != FIRM_MAGIC)
        return 1;

    memcpy(dest, firm, *size);

    return 0;
}

int decrypt_arm9bin(arm9bin_h *header, uint64_t firm_title) {
    uint8_t slot = 0x15;

    fprintf(BOTTOM_SCREEN, "Decrypting ARM9 FIRM binary\n");

//    if (firm_title == NATIVE_FIRM_TITLEID && version > 0x0F) {
    if (firm_title == NATIVE_FIRM_TITLEID) {
        uint8_t decrypted_keyx[AES_BLOCK_SIZE];

        slot0x11key96_init();
        slot = 0x16;

        aes_use_keyslot(0x11);
        aes(decrypted_keyx, header->slot0x16keyX, 1, NULL, AES_ECB_DECRYPT_MODE, 0);
        aes_setkey(slot, decrypted_keyx, AES_KEYX, AES_INPUT_BE | AES_INPUT_NORMAL);
    }

    aes_setkey(slot, header->keyy, AES_KEYY, AES_INPUT_BE | AES_INPUT_NORMAL);
    aes_setiv(header->ctr, AES_INPUT_BE | AES_INPUT_NORMAL);

    void *arm9bin = (uint8_t *)header + 0x800;
    int size = atoi(header->size);

    aes_use_keyslot(slot);
    aes(arm9bin, arm9bin, size / AES_BLOCK_SIZE, header->ctr, AES_CTR_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);

    if (firm_title == NATIVE_FIRM_TITLEID)
        return *(uint32_t *)arm9bin != ARM9BIN_MAGIC;
    else if (firm_title == AGB_FIRM_TITLEID || firm_title == TWL_FIRM_TITLEID)
        return *(uint32_t *)arm9bin != LGY_ARM9BIN_MAGIC;
    else return 0;
}

int decrypt_firm(firm_h *dest, char *path_firmkey, char *path_cetk, uint32_t *size, uint64_t firm_title) {
    uint8_t firm_key[AES_BLOCK_SIZE];

    // Firmware is likely encrypted. Decrypt.
    if (!read_file(firm_key, path_firmkey, AES_BLOCK_SIZE)) {
        fprintf(BOTTOM_SCREEN, "Failed to load FIRM key\n");
        return 1;
    } else {
        fprintf(BOTTOM_SCREEN, "Loaded FIRM key\n");
    }

    fprintf(BOTTOM_SCREEN, "Decrypting FIRM\n");
    if (decrypt_firm_title(dest, (void *)dest, size, firm_key) != 0) {
        fprintf(BOTTOM_SCREEN, "Failed to decrypt the firmware\n");
        return 1;
    }
	return 0;
}

int load_firm(firm_h *dest, char *path, char *path_firmkey, char *path_cetk, uint32_t *size, uint64_t firm_title) {
    int status = 0;
    int firmware_changed = 0;

    if (read_file(dest, path, *size)) {
        fprintf(BOTTOM_SCREEN, "Failed to read FIRM from SD\n");

        // Only whine about this if it's NATIVE_FIRM, which is important.
        if (firm_title == NATIVE_FIRM_TITLEID) {
            fprintf(BOTTOM_SCREEN, "Failed to load NATIVE_FIRM from:\n"
                                   "  " PATH_NATIVE_F "\n"
                                   "This is fatal. Aborting.\n");
        }

        status = 1;
        goto exit_error;
    }

	// Check and decrypt FIRM if it is encrypted.
    if (dest->magic != FIRM_MAGIC) {
        status = decrypt_firm(dest, path_firmkey, path_cetk, size, firm_title);
        if (status != 0) {
            if (firm_title == NATIVE_FIRM_TITLEID) {
                fprintf(BOTTOM_SCREEN, "Failed to decrypt firmware.\n"
                                       "This is fatal. Aborting.\n");
            }
            goto exit_error;
        }
        firmware_changed = 1; // Decryption performed.
    } else {
        fprintf(BOTTOM_SCREEN, "FIRM not encrypted\n");
    }

    // The N3DS firm has an additional encryption layer for ARM9
    // Only run if n3ds.
    {
        // Look for the arm9 section
        for (firm_section_h *section = dest->section;
                section < dest->section + 4; section++) {
            if (section->type == FIRM_TYPE_ARM9) {
                // Check whether the arm9bin is encrypted.
                int arm9bin_iscrypt = 0;
                uint32_t magic = *(uint32_t*)((uintptr_t)dest + section->offset + 0x800);
                if (firm_title == NATIVE_FIRM_TITLEID)
                    arm9bin_iscrypt = (magic != ARM9BIN_MAGIC);
                else if (firm_title == AGB_FIRM_TITLEID || firm_title == TWL_FIRM_TITLEID)
                    arm9bin_iscrypt = (magic != LGY_ARM9BIN_MAGIC);

                if (arm9bin_iscrypt) {
                    // Decrypt the arm9bin.
                    if (decrypt_arm9bin((arm9bin_h *)((uintptr_t)dest + section->offset), firm_title)) {
                        fprintf(BOTTOM_SCREEN, "Couldn't decrypt ARM9 FIRM binary.\n"
                                               "Check if you have the needed key at:\n"
                                               "  " PATH_SLOT0X11KEY96 "\n");
                        status = 1;
                        goto exit_error;
                    }
                    firmware_changed = 1; // Decryption of arm9bin performed.
                } else {
                    fprintf(BOTTOM_SCREEN, "ARM9 FIRM binary not encrypted\n");
//                    if (firm_type == NATIVE_FIRM && firm_current->version > 0x0F) {
                        slot0x11key96_init(); // This has to be loaded regardless, otherwise boot will fail.
//                    }
                }

                // We assume there's only one section to decrypt.
                break;
            }
        }
    }

    // Save firmware.bin if decryption was done.
    if (firmware_changed) {
        fprintf(BOTTOM_SCREEN, "Saving decrypted FIRM\n");
        write_file(dest, path, *size);
    }

    //if (firm_current->console == console_n3ds)
    {
        fprintf(BOTTOM_SCREEN, "Fixing arm9 entrypoint\n");

        // Patch the entrypoint to skip arm9loader
        if (firm_title == NATIVE_FIRM_TITLEID) {
            dest->a9Entry = 0x0801B01C;
        } else if (firm_title == AGB_FIRM_TITLEID ||
                   firm_title == TWL_FIRM_TITLEID) {
            dest->a9Entry = 0x0801301C;
        }
        // The entrypoints seem to be the same across different FIRM versions,
        //  so we don't change them.
    }

    return 0;

exit_error:

    return status;
}

void __attribute__((naked)) disable_lcds() {
    *a11_entry = 0;  // Don't wait for us

    *(volatile uint32_t *)0x10202A44 = 0;
    *(volatile uint32_t *)0x10202244 = 0;
    *(volatile uint32_t *)0x1020200C = 0;
    *(volatile uint32_t *)0x10202014 = 0;

    while (!*a11_entry);
    ((void (*)())*a11_entry)();
}

void boot_firm() {
    // Set up the keys needed to boot a few firmwares, due to them being unset, depending on which firmware you're booting from.
    // TODO: Don't use the hardcoded offset.
    if (update_96_keys) {
        void *keydata = (void *)((uintptr_t)firm_loc + firm_loc->section[2].offset + 0x89814);

        aes_use_keyslot(0x11);
        uint8_t keyx[AES_BLOCK_SIZE];
        for (int slot = 0x19; slot < 0x20; slot++) {
            aes(keyx, keydata, 1, NULL, AES_ECB_DECRYPT_MODE, 0);
            aes_setkey(slot, keyx, AES_KEYX, AES_INPUT_BE | AES_INPUT_NORMAL);
            *(uint8_t *)(keydata + 0xF) += 1;
        }

        fprintf(BOTTOM_SCREEN, "Updated keyX keyslots\n");
    }

    struct memory_header *memory = (void *)(memory_loc + 1);
    while ((uintptr_t)memory < (uintptr_t)memory_loc + *memory_loc) {
        memcpy((void *)memory->location, memory + 1, memory->size);
        memory = (void *)((uintptr_t)(memory + 1) + memory->size);
    }
    fprintf(BOTTOM_SCREEN, "Copied memory\n");

    for (firm_section_h *section = firm_loc->section;
            section < firm_loc->section + 4 && section->address != 0; section++) {
        memcpy((void *)section->address, (void *)firm_loc + section->offset, section->size);
    }
    fprintf(BOTTOM_SCREEN, "Copied FIRM\n");

    *a11_entry = (uint32_t)disable_lcds;
    while (*a11_entry);  // Make sure it jumped there correctly before changing it.
    *a11_entry = (uint32_t)firm_loc->a11Entry;

    fprintf(BOTTOM_SCREEN, "Prepared arm11 entry, jumping to FIRM\n");

    ((void (*)())firm_loc->a9Entry)();
}

int load_firms() {
    fprintf(TOP_SCREEN, "[Loading FIRM]");

    fprintf(BOTTOM_SCREEN, "Loading NATIVE_FIRM\n");
    if (load_firm(firm_loc, PATH_NATIVE_F, PATH_NATIVE_FIRMKEY, PATH_NATIVE_CETK, &firm_size, NATIVE_FIRM_TITLEID) != 0)
        return 1;

    fprintf(BOTTOM_SCREEN, "Loading TWL_FIRM\n");
    if(load_firm(twl_firm_loc, PATH_TWL_F, PATH_TWL_FIRMKEY, PATH_TWL_CETK, &twl_firm_size, TWL_FIRM_TITLEID))
        fprintf(BOTTOM_SCREEN, "TWL_FIRM failed to load.\n");

    fprintf(BOTTOM_SCREEN, "Loading AGB_FIRM\n");
    if(load_firm(agb_firm_loc, PATH_AGB_F, PATH_AGB_FIRMKEY, PATH_AGB_CETK, &agb_firm_size, AGB_FIRM_TITLEID))
        fprintf(BOTTOM_SCREEN, "AGB_FIRM failed to load.\n");

    return 0;
}

void boot_cfw() {
    fprintf(TOP_SCREEN, "[Patching]");
//    if (patch_firm_all() != 0)
//        return;

    // Only save the firm if that option is required (or it's needed for autoboot),
    //   and either the patches have been modified, or the file doesn't exist.
    if ((save_firm || config.options[OPTION_AUTOBOOT]) &&
            f_stat(PATH_NATIVE_P, NULL) != 0) {
        fprintf(BOTTOM_SCREEN, "Saving patched NATIVE_FIRM\n");
        if (write_file(firm_loc, PATH_NATIVE_P, firm_size) != firm_size) {
            fprintf(BOTTOM_SCREEN, "%pFailed to save patched FIRM.\nWriting SD failed.\nThis is fatal.\n", COLOR(RED, BLACK));
            return;
        }
    }

    if ((save_firm || config.options[OPTION_AUTOBOOT]) &&
            f_stat(PATH_MEMBIN, NULL) != 0) {
        fprintf(BOTTOM_SCREEN, "Saving patched memory\n");
        if (write_file(memory_loc, PATH_MEMBIN, *memory_loc) != *memory_loc) {
            fprintf(BOTTOM_SCREEN, "%pFailed to save the patched FIRM\nWriting SD failed.\n", COLOR(RED, BLACK));
            return;
        }
    }

    if (f_stat(PATH_TWL_P, NULL) != 0) {
        fprintf(BOTTOM_SCREEN, "Saving patched TWL_FIRM\n");
        if (write_file(twl_firm_loc, PATH_TWL_P, twl_firm_size) != twl_firm_size) {
            fprintf(BOTTOM_SCREEN, "%pFailed to save the patched FIRM\nWriting SD failed.\n", COLOR(RED, BLACK));
            return;
        }
    }

    if (f_stat(PATH_AGB_P, NULL) != 0) {
        fprintf(BOTTOM_SCREEN, "Saving patched AGB_FIRM\n");
        if (write_file(agb_firm_loc, PATH_AGB_P, agb_firm_size) != agb_firm_size) {
            fprintf(BOTTOM_SCREEN, "%pFailed to save the patched FIRM\nWriting SD failed.\n", COLOR(RED, BLACK));
            return;
        }
    }

//    boot_firm();
}
