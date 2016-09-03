#include <stdint.h>
#include <stddef.h>

#include <ctr9/io.h>
#include <ctr9/aes.h>
#include <ctr9/sha.h>
#include <common.h>

#define SECTOR_SIZE 0x200

int decrypt_k9l(arm9bin_h *header, enum firm_type type) {
    uint8_t slot = 0x15;

    if (type == type_native) {
        uint8_t decrypted_keyx[AES_BLOCK_SIZE];

        slot = 0x16;

        use_aeskey(0x11);
        ecb_decrypt(header->slot0x16keyX, decrypted_keyx, 1, AES_CNT_ECB_DECRYPT_MODE);
        setup_aeskeyX(slot, decrypted_keyx);
    }

    setup_aeskeyY(slot, header->keyy);

    set_ctr(header->ctr);

    void *arm9bin = (uint8_t *)header + 0x800;
    int size = atoi(header->size); // Size is plaintext, don't ask me *shrug*

    use_aeskey(slot);

    ctr_decrypt(arm9bin, arm9bin, (size_t)size / AES_BLOCK_SIZE, AES_CNT_CTRNAND_MODE, header->ctr);

    if (type == type_native && *(uint32_t *)arm9bin == ARM9BIN_MAGIC)
        return 0;
    else if (*(uint32_t *)arm9bin == LGY_ARM9BIN_MAGIC)
        return 0;

    return 1; // Failed.
}

void fix_entry(firm_h* firm, enum firm_type type) {
    // Patch the entrypoint to skip arm9loader
    if (type == type_native)
        firm->a9Entry = 0x0801B01C;
    else
        firm->a9Entry = 0x0801301C;

    // The entrypoints seem to be the same across different FIRM versions,
    //  so we don't change them.
}

// 0x0B130000 = start of FIRM0 partition, 0x400000 = size of FIRM partition (4MB)
firm_h* dump_firm(firm_h* buffer, uint8_t index) {
    firm_h* firm;

    // NOTE - Cast, because GCC is making assumptions about 'index'.
    uint32_t firm_offset = (uint32_t)(0x0B130000 + (index % 2) * 0x400000),
             firm_b_size = 0x00100000; // 1MB, because

    firm = malloc(firm_b_size);

    uint8_t ctr[0x10],
            cid[0x10],
            sha_t[0x20];

    if (sdmmc_nand_readsectors(firm_offset / SECTOR_SIZE, firm_b_size / SECTOR_SIZE, (uint8_t*)firm))
        goto failure;

    sdmmc_get_cid(1, (uint32_t*)cid);
    sha256sum(sha_t, cid, 0x10);
    memcpy(ctr, sha_t, 0x10);
    add_ctr(ctr, firm_offset / AES_BLOCK_SIZE);

    use_aeskey(0x06);
    set_ctr(ctr);
    ctr_decrypt(firm, firm, firm_b_size / AES_BLOCK_SIZE, AES_CNT_CTRNAND_MODE, ctr);

    if (memcmp((char*) & firm->magic, "FIRM", 4))
        goto failure;

    return firm;

failure:
    free(firm);

    return NULL;
}

uint8_t* find_section_key(firm_h *firm_loc) {
    // The key will be dword-aligned (I think? Verify this. May need new NFIRM to check assumption. Go, Nintendo!)
#if 0
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
#endif
    return NULL;
}

int set_section_keys(firm_h* firm_loc) {
    // Set up the keys needed to boot a few firmwares, due to them being unset,
    // depending on which firmware you're booting from.
    uint8_t *keydata = find_section_key(firm_loc);
    if (!keydata)
        return 1;

    use_aeskey(0x11);
    uint8_t keyx[AES_BLOCK_SIZE];
    for (int slot = 0x19; slot < 0x20; slot++) {
        ecb_decrypt(keydata, keyx, 1, AES_CNT_ECB_DECRYPT_MODE);
        setup_aeskeyX(slot, keyx);
        *(uint8_t *)(keydata + 0xF) += 1;
    }

    return 0;
}
