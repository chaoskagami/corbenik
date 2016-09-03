#include <common.h>

#include <ctr9/aes.h>
#include <ctr9/sha.h>

key_find_t Y11_sec;
key_find_t X11_sec;

key_find_t Y11[] = {
    #include "keys/Y11_95.gen"
    #include "keys/Y11_96.gen"
};

key_find_t Y3D[] = {
    #include "keys/Y3D_0.gen"
    #include "keys/Y3D_1.gen"
    #include "keys/Y3D_2.gen"
    #include "keys/Y3D_3.gen"
    #include "keys/Y3D_4.gen"
    #include "keys/Y3D_5.gen"
};

key_find_t Y05 =
#include "keys/Y05.gen"
;

#define ROLL_WINDOW    AES_BLOCK_SIZE
#define MODULO_WINDOW  (ROLL_WINDOW / 2)
uint8_t* slice_roll_search(uint8_t *mem, uint32_t size, key_find_t* find) {
    uint16_t roll = 0;
    uint32_t i = 0;
    uint8_t hash[32];

    // Initial window.
    for(; i < ROLL_WINDOW; i++) {
        roll += mem[i];
    }

    // Loop through, moving the window.
    for(i = ROLL_WINDOW; i < size; i++) {
        if (find->roll == roll) {
            // Yes. Check hash.
            sha256sum(hash, &mem[i], 0x10);

            if(!memcmp(find->sha, hash, 0x20)) {
                return & mem[i];
            }
        }

        roll -= mem[i-16];
        roll += mem[i];
    }
    return NULL;
}

int get_Y11_sec() {
    // FIXME; this only handles the case of K9LH. Needs more sanity checks.

    uint8_t hash[32];

    // We know better here than GCC.
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdiscarded-qualifiers"
    memcpy(hash, (volatile void*)REG_SHAHASH, 32);
#pragma GCC diagnostic pop

    memcpy(X11_sec.key, hash, 16);
    memcpy(Y11_sec.key, hash + 16, 16);

    return 0;
}

int get_Y11_K9L(firm_h *firm, int index) {
    // A9LH corrupts this one. Can't do much here.
    int level = 0;

    uint8_t key[AES_BLOCK_SIZE];
    FILE* f;

    // 9.5 key (K9L1)
    f = fopen(PATH_SLOT0X11KEY95, "r");
    if (!f) {
        level |= 1;
        goto next;
    }

    fread(Y11[0].key, 1, AES_BLOCK_SIZE, f);
    fclose(f);

next:
    // 9.6 key (K9L2)
    f = fopen(PATH_SLOT0X11KEY96, "r");
    if (!f) {
        level |= 2;
        goto end;
    }

    fread(Y11[1].key, 1, AES_BLOCK_SIZE, f);
    fclose(f);

end:
    return level;
}

int get_Y3D(firm_h *firm, int index) {
    uint8_t* key_loc     = (uint8_t*)firm + firm->section[2].offset; // ARM9 segment
    uint32_t search_size = firm->section[2].size;

    uint8_t mem[16] __attribute__((aligned(16))) = {0};

    uint8_t* key_data = slice_roll_search(key_loc, search_size, & Y3D[0]);

    if (!key_data)
        return 1;

    memcpy(mem, key_data, 16);

    return 0;
}

int get_Y05(firm_h *firm) {
    uint8_t* key_loc     = (uint8_t*)firm + firm->section[2].offset; // ARM9 segment
    uint32_t search_size = firm->section[2].size;

    uint8_t mem[16] __attribute__((aligned(16))) = {0};

    uint8_t* key_data = slice_roll_search(key_loc, search_size, &Y05);

    if (!key_data)
        return 1;

    fprintf(stderr, "  0x05 KeyY at %lx in FIRM1\n", (uint32_t)key_data - (uint32_t)key_loc);

    memcpy(mem, key_data, 16);

    return 0;
}

int extract_keys() {
    int level = 0;

	if (get_Y11_sec()) {// MUST be done first. Otherwise, sha register gets clobbered.
        // At best, a warning.
        level |= 1;
    }

#if 0
    if (get_Y11_K9L()) { // For decrypting K9L.
        // Also a warning, but potentially fatal if an N3DS.
        level |= 2;
    }

    if (get_Y3D()) {
        // No cetk decryption.
        level |= 4;
    }

    if (get_Y05()) {
        // Pretty much a warning and nothing else atm.
        level |= 8;
    }
#endif

    return level;
}

int set_Y3D_common(int commonKeyIndex) {
	setup_aeskeyY(0x3D, (void*) Y3D[commonKeyIndex].key);

	use_aeskey(0x3D);

	return 0;
}

int set_Y05() {
    // N3DS nand key
    setup_aeskeyY(0x05, Y05.key);

	use_aeskey(0x05);

	return 0;
}

int set_Y11_K9L(int index) {
    setup_aeskey(0x11, Y11[index].key);

	use_aeskey(0x11);

	return 0;
}
