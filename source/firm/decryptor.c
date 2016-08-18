/*
  This is all fairly minimal and based on @d0k3's decrypt9 code.
*/

#include <common.h>
#include <ctr9/aes.h>
#include <ctr9/sha.h>

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
