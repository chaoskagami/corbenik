#include "firm.h"

#include <stdint.h>
#include <stddef.h>

#include <ctr9/io.h>
#include <ctr9/aes.h>

#include "../common.h"
#include "../misc/sha256.h"

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

static int update_96_keys = 0;

static volatile uint32_t *const a11_entry = (volatile uint32_t *)0x1FFFFFF8;

typedef enum {
    NCCHTYPE_EXHEADER = 1,
    NCCHTYPE_EXEFS = 2,
    NCCHTYPE_ROMFS = 3,
} ctr_ncchtypes;

void
ncch_getctr(const ncch_h *ncch, uint8_t *ctr, uint8_t type)
{
    uint32_t version = ncch->version;
    const uint8_t *partitionID = ncch->partitionID;
    int i;

    for (i = 0; i < 16; i++)
        ctr[i] = 0x00;

    if (version == 2 || version == 0) {
        for (i = 0; i < 8; i++)
            ctr[i] = partitionID[7 - i]; // Convert to big endian & normal input
        ctr[8] = type;
    } else if (version == 1) {
        int x = 0;
        if (type == NCCHTYPE_EXHEADER)
            x = MEDIA_UNITS;
        else if (type == NCCHTYPE_EXEFS)
            x = ncch->exeFSOffset * MEDIA_UNITS;
        else if (type == NCCHTYPE_ROMFS)
            x = ncch->exeFSOffset * MEDIA_UNITS;
        for (i = 0; i < 8; i++)
            ctr[i] = partitionID[i];
        for (i = 0; i < 4; i++)
            ctr[i + 12] = (x >> ((3 - i) * 8)) & 0xFF;
    }
}

void
aes(void *dst, const void *src, uint32_t blockCount, void *iv, uint32_t mode, uint32_t ivMode)
{
    aes_setmode(mode);
    uint32_t blocks;
    while (blockCount != 0) {
        if ((mode & AES_ALL_MODES) != AES_ECB_ENCRYPT_MODE && (mode & AES_ALL_MODES) != AES_ECB_DECRYPT_MODE)
            aes_setiv(iv, ivMode);
        blocks = (blockCount >= 0xFFFF) ? 0xFFFF : blockCount;

        // Save the last block for the next decryption CBC batch's iv
        if ((mode & AES_ALL_MODES) == AES_CBC_DECRYPT_MODE) {
            memcpy(iv, src + (blocks - 1) * AES_BLOCK_SIZE, AES_BLOCK_SIZE);
            aes_change_ctrmode(iv, AES_INPUT_BE | AES_INPUT_NORMAL, ivMode);
        }

        // Process the current batch
        aes_batch(dst, src, blocks);

        // Save the last block for the next encryption CBC batch's iv
        if ((mode & AES_ALL_MODES) == AES_CBC_ENCRYPT_MODE) {
            memcpy(iv, dst + (blocks - 1) * AES_BLOCK_SIZE, AES_BLOCK_SIZE);
            aes_change_ctrmode(iv, AES_INPUT_BE | AES_INPUT_NORMAL, ivMode);
        } else if ((mode & AES_ALL_MODES) == AES_CTR_MODE) { // Advance counter for CTR mode
            aes_advctr(iv, blocks, ivMode);
        }

        src += blocks * AES_BLOCK_SIZE;
        dst += blocks * AES_BLOCK_SIZE;
        blockCount -= blocks;
    }
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
    // If we can't read the key, we assume it's not needed, and the firmware is
    // the right version.
    // Otherwise, we make sure the error message for decrypting arm9bin mentions
    // this.
}

int
decrypt_cetk_key(void *key, const void *cetk)
{
    static int common_key_y_init = 0;
    uint8_t iv[AES_BLOCK_SIZE] = { 0 };
    uint32_t sigtype = __builtin_bswap32(*(uint32_t *)cetk);

    if (sigtype != SIG_TYPE_RSA2048_SHA256)
        return 1;

    ticket_h *ticket = (ticket_h *)((uint8_t*)cetk + sizeof(sigtype) + 0x13C);

    if (ticket->ticketCommonKeyYIndex != 1)
        return 1;

    if (!common_key_y_init) {
        uint8_t common_key_y[AES_BLOCK_SIZE] = { 0 };
        uint8_t *p9_base = (uint8_t *)0x08028000;
        uint8_t *i;
        for (i = p9_base + 0x70000 - AES_BLOCK_SIZE; i >= p9_base; i--) {
            if (i[0] == 0xD0 && i[4] == 0x9C && i[8] == 0x32 && i[12] == 0x23) {
                // At i, there's 7 keys with 4 bytes padding between them.
                // We only need the 2nd.
                memcpy(common_key_y, i + AES_BLOCK_SIZE + 4, sizeof(common_key_y));
                fprintf(stderr, "y");
                break;
            }
        }

        if (i < p9_base)
            return 1;

        setup_aeskeyY(0x3D, common_key_y);
        common_key_y_init = 1;
    }

    use_aeskey(0x3D);
    memcpy(iv, ticket->titleID, sizeof(ticket->titleID));

    memcpy(key, ticket->titleKey, sizeof(ticket->titleKey));
    aes(key, key, 1, iv, AES_CBC_DECRYPT_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);

    fprintf(stderr, "g");

    return 0;
}

int
decrypt_firm_title(firm_h *dest, ncch_h *ncch, uint32_t *size, void *key)
{
    uint8_t firm_iv[16] = { 0 };
    uint8_t exefs_key[16] = { 0 };
    uint8_t exefs_iv[16] = { 0 };

    fprintf(BOTTOM_SCREEN, "n");
    setup_aeskey(0x16, key);
    use_aeskey(0x16);
    aes(ncch, ncch, *size / AES_BLOCK_SIZE, firm_iv, AES_CBC_DECRYPT_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);

    if (ncch->magic != NCCH_MAGIC)
        return 1;

    memcpy(exefs_key, ncch, 16);
    ncch_getctr(ncch, exefs_iv, NCCHTYPE_EXEFS);

    // Get the exefs offset and size from the NCCH
    exefs_h *exefs = (exefs_h *)((uint8_t *)ncch + ncch->exeFSOffset * MEDIA_UNITS);
    uint32_t exefs_size = ncch->exeFSSize * MEDIA_UNITS;

    fprintf(BOTTOM_SCREEN, "e");
    setup_aeskeyY(0x2C, exefs_key);
    use_aeskey(0x2C);
    aes(exefs, exefs, exefs_size / AES_BLOCK_SIZE, exefs_iv, AES_CTR_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);

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

    fprintf(BOTTOM_SCREEN, "9");

    if (firm_title == NATIVE_FIRM_TITLEID && version > 0x0F) {
        uint8_t decrypted_keyx[AES_BLOCK_SIZE];

        slot0x11key96_init();
        slot = 0x16;

        use_aeskey(0x11);
        aes(decrypted_keyx, header->slot0x16keyX, 1, NULL, AES_ECB_DECRYPT_MODE, 0);
        setup_aeskeyX(slot, decrypted_keyx);
    }

    setup_aeskeyY(slot, header->keyy);
    set_ctr(header->ctr);

    void *arm9bin = (uint8_t *)header + 0x800;
    int size = atoi(header->size);

    use_aeskey(slot);
    aes(arm9bin, arm9bin, size / AES_BLOCK_SIZE, header->ctr, AES_CTR_MODE, AES_INPUT_BE | AES_INPUT_NORMAL);

    if (firm_title == NATIVE_FIRM_TITLEID)
        return *(uint32_t *)arm9bin != ARM9BIN_MAGIC;
    else if (firm_title == AGB_FIRM_TITLEID || firm_title == TWL_FIRM_TITLEID)
        return *(uint32_t *)arm9bin != LGY_ARM9BIN_MAGIC;
    else
        return 0;
}

int
decrypt_firm(firm_h *dest, char *path_firmkey, char *path_cetk, uint32_t *size)
{
    uint8_t firm_key[AES_BLOCK_SIZE];

    // Firmware is likely encrypted. Decrypt.
    if (!read_file(firm_key, path_firmkey, AES_BLOCK_SIZE)) {
        // Missing firmkey. Attempt to get from CETK (only works if system was booted)
        if (!read_file((void *)FCRAM_JUNK_LOC, path_cetk, FCRAM_SPACING) || decrypt_cetk_key(firm_key, (void *)FCRAM_JUNK_LOC)) {
            fprintf(BOTTOM_SCREEN, "!");
            return 1;
        } else {
            fprintf(BOTTOM_SCREEN, "t");
            write_file(firm_key, path_firmkey, AES_BLOCK_SIZE);
        }
    } else {
        fprintf(BOTTOM_SCREEN, "k");
    }

    fprintf(BOTTOM_SCREEN, "d");
    if (decrypt_firm_title(dest, (void *)dest, size, firm_key) != 0) {
        fprintf(BOTTOM_SCREEN, "!");
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
        fprintf(BOTTOM_SCREEN, "!");
        return 1;
    } else {
        fprintf(BOTTOM_SCREEN, "l");
    }

    // Check and decrypt FIRM if it is encrypted.
    if (dest->magic != FIRM_MAGIC) {
        status = decrypt_firm(dest, path_firmkey, path_cetk, size);
        if (status != 0) {
            fprintf(BOTTOM_SCREEN, "!");
            return 1;
        }
        firmware_changed = 1; // Decryption performed.
    } else {
        fprintf(BOTTOM_SCREEN, "_");
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
                    fprintf(BOTTOM_SCREEN, "_");
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
        fprintf(BOTTOM_SCREEN, "s");
        write_file(dest, path, *size);
    }

    if (fsig->console == console_n3ds) {
        fprintf(BOTTOM_SCREEN, "f");

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

void* find_section_key() {
    // The key will be dword-aligned (I think? Verify this. May need new NFIRM to check assumption. Go, Nintendo!)
    uint8_t* key_loc = (uint8_t*)firm_loc + firm_loc->section[2].offset - 16;
    uint32_t search_size = firm_loc->section[2].size - 16;

    key_loc = (uint8_t*)((uint32_t)key_loc & 0xFFFFFFF8); // Align to 4bytes.

    // The hash of the key. Can't give the key itself out, obviously.
    uint32_t sha256boot[32] = {0xb9, 0x4d, 0xb1, 0xb1, 0xc3, 0xe0, 0x11, 0x08,
                               0x9c, 0x19, 0x46, 0x06, 0x4a, 0xbc, 0x40, 0x2a,
                               0x7c, 0x66, 0xf4, 0x4a, 0x74, 0x6f, 0x71, 0x50,
                               0x32, 0xfd, 0xff, 0x03, 0x74, 0xd7, 0x45, 0x2c};
    uint8_t sha256check[32] = {0};

    for(; search_size > 0; search_size -= 4) {
        // Is candidate?
        if (key_loc[search_size] == 0xDD) {
            // Yes. Check hash.
            Sha256Data(&key_loc[search_size], 16, sha256check);
            if(memcmp(sha256boot, sha256check, 32)) {
                fprintf(stderr, "Key at %x\n", (uint32_t)(&key_loc[search_size]) - (uint32_t)key_loc);
                return &key_loc[search_size];
            }
        }
    }

    return NULL;
}

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
            abort("Couldn't find key!\n");
        }

        wait();

        use_aeskey(0x11);
        uint8_t keyx[AES_BLOCK_SIZE];
        for (int slot = 0x19; slot < 0x20; slot++) {
            aes(keyx, keydata, 1, NULL, AES_ECB_DECRYPT_MODE, 0);
            setup_aeskeyX(slot, keyx);
            *(uint8_t *)(keydata + 0xF) += 1;
        }

        fprintf(BOTTOM_SCREEN, "Updated keyX keyslots\n");
    }

    for (firm_section_h *section = firm_loc->section; section < firm_loc->section + 4 && section->address != 0; section++) {
        memcpy((void *)section->address, (void *)((uint8_t*)firm_loc + section->offset), section->size);
    }
    fprintf(BOTTOM_SCREEN, "Copied FIRM\n");

    wait();

    clear_disp(BOTTOM_SCREEN);
    set_cursor(BOTTOM_SCREEN, 0, 0);

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
                        process9->offset = ((uint32_t)((*p9exefs) + 1) - (uint32_t)firm);
                        fprintf(BOTTOM_SCREEN, "p");
                        return 0;
                    }
                }
                ++arm9section;
            }
        }
    }
    fprintf(BOTTOM_SCREEN, "\n  Couldn't find Process9?\n");
    return 1;
}

int firm_loaded = 0;

int
load_firms()
{
    int state = 0;

    if (firm_loaded)
        return 0;

    fprintf(BOTTOM_SCREEN, "FIRM load triggered.\n");

    fprintf(BOTTOM_SCREEN, "NATIVE_FIRM\n  [");
    if (load_firm(firm_loc, PATH_NATIVE_F, PATH_NATIVE_FIRMKEY, PATH_NATIVE_CETK, &firm_size, NATIVE_FIRM_TITLEID) != 0) {
        abort("]\n  Failed to load NATIVE_FIRM.\n");
    }
    find_proc9(firm_loc, &firm_proc9, &firm_p9_exefs);
    fprintf(stderr, "]\n");
    fprintf(stderr, "Ver: %x, %u\n", get_firm_info(firm_loc)->version, get_firm_info(firm_loc)->console );

    fprintf(BOTTOM_SCREEN, "TWL_FIRM\n  [");
    if (load_firm(twl_firm_loc, PATH_TWL_F, PATH_TWL_FIRMKEY, PATH_TWL_CETK, &twl_firm_size, TWL_FIRM_TITLEID) != 0) {
        fprintf(BOTTOM_SCREEN, "]\n  TWL_FIRM failed to load.\n");
        state = 1;
    } else {
        find_proc9(twl_firm_loc, &twl_firm_proc9, &twl_firm_p9_exefs);
        fprintf(stderr, "]\n");
        fprintf(stderr, "Ver: %x, %u\n", get_firm_info(twl_firm_loc)->version, get_firm_info(twl_firm_loc)->console );
    }

    fprintf(BOTTOM_SCREEN, "AGB_FIRM\n  [");
    if (load_firm(agb_firm_loc, PATH_AGB_F, PATH_AGB_FIRMKEY, PATH_AGB_CETK, &agb_firm_size, AGB_FIRM_TITLEID) != 0) {
        fprintf(BOTTOM_SCREEN, "]\n  AGB_FIRM failed to load.\n");
        state = 1;
    } else {
        find_proc9(agb_firm_loc, &agb_firm_proc9, &agb_firm_p9_exefs);
        fprintf(stderr, "]\n");
        fprintf(stderr, "Ver: %x, %u\n", get_firm_info(agb_firm_loc)->version, get_firm_info(agb_firm_loc)->console );
    }

    firm_loaded = 1; // Loaded.

    return state;
}

void
boot_cfw()
{
    fprintf(BOTTOM_SCREEN, "Loading firmware...\n");

    load_firms();

    if (config.options[OPTION_RECONFIGURED]) {
        fprintf(stderr, "Generating patch cache...\n");
        generate_patch_cache();
    }

    fprintf(BOTTOM_SCREEN, "Patching firmware...\n");
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
