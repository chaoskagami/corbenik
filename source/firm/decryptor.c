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
aes_batch(void *dst, const void *src, uint32_t blockCount)
{
    *REG_AESBLKCNT = blockCount << 16;
    *REG_AESCNT |= AES_CNT_START;

    const uint32_t *src32 = (const uint32_t *)src;
    uint32_t *dst32 = (uint32_t *)dst;

    uint32_t wbc = blockCount;
    uint32_t rbc = blockCount;

    while (rbc) {
        if (wbc && ((*REG_AESCNT & 0x1F) <= 0xC)) // There's space for at least 4 ints
        {
			for(int i=0; i < 4; i++)
	            *REG_AESWRFIFO = *src32++;
            wbc--;
        }

        if (rbc && ((*REG_AESCNT & (0x1F << 0x5)) >= (0x4 << 0x5))) // At least 4 ints available for read
        {
			for(int i=0; i < 4; i++)
	            *dst32++ = *REG_AESRDFIFO;
            rbc--;
        }
    }
}

inline void
aes_setmode(uint32_t mode)
{
    *REG_AESCNT = mode | AES_CNT_INPUT_ORDER | AES_CNT_OUTPUT_ORDER | AES_CNT_INPUT_ENDIAN | AES_CNT_OUTPUT_ENDIAN | AES_CNT_FLUSH_READ | AES_CNT_FLUSH_WRITE;
}

void
aes(void *dst, void *src, uint32_t blockCount, void *iv, uint32_t mode)
{
    aes_setmode(mode);

    uint32_t blocks;
    while (blockCount != 0) {
        if ((mode & (7u << 27)) != AES_ECB_ENCRYPT_MODE && (mode & (7u << 27)) != AES_ECB_DECRYPT_MODE)
            set_ctr(iv);

        blocks = (blockCount >= 0xFFFF) ? 0xFFFF : blockCount;

        // Save the last block for the next decryption CBC batch's iv
        if ((mode & (7u << 27)) == AES_CBC_DECRYPT_MODE)
            memcpy(iv, src + (blocks - 1) * AES_BLOCK_SIZE, AES_BLOCK_SIZE);

        // Process the current batch
        aes_batch(dst, src, blocks);

        // Save the last block for the next encryption CBC batch's iv
        if ((mode & (7u << 27)) == AES_CBC_ENCRYPT_MODE)
            memcpy(iv, dst + (blocks - 1) * AES_BLOCK_SIZE, AES_BLOCK_SIZE);

        // Advance counter for CTR mode
        else if ((mode & (7u << 27)) == AES_CTR_MODE)
            add_ctr(iv, blocks);

        src += blocks * AES_BLOCK_SIZE;
        dst += blocks * AES_BLOCK_SIZE;
        blockCount -= blocks;
    }
}

void
sha256sum(void* sum, void* data, uint32_t size) {
	sha_init(SHA256_MODE);
    sha_update(data, size);
    sha_get(sum);
}
