#include <stdint.h>          // for uint8_t, uint32_t
#include <stddef.h>          // for uint8_t, uint32_t
#include <ctr9/sha.h>        // for sha_get, sha_init, sha_update, SHA256_MODE
#include "firm/headers.h"    // for ncch_h, MEDIA_UNITS
#include "firm/keys.h"    // for ncch_h, MEDIA_UNITS
#include "firm/firm.h"    // for ncch_h, MEDIA_UNITS
#define FIRM_INTERNAL_CODE
#include <firm/internal.h>  // lines 6-6
#include "firm/decryptor.h"  // for ::NCCHTYPE_EXEFS, ::NCCHTYPE_EXHEADER

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
        unsigned int x = 0;
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
sha256sum(void* sum, void* data, uint32_t size)
{
    sha_init(SHA256_MODE);
    sha_update(data, size);
    sha_get(sum);
}
