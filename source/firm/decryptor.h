#ifndef __DECRYPTOR_H
#define __DECRYPTOR_H

void aes(void *dst, void *src, uint32_t blockCount, void *iv, uint32_t mode);
void sha256sum(void* sum, void* data, uint32_t size);

typedef enum {
    NCCHTYPE_EXHEADER = 1,
    NCCHTYPE_EXEFS = 2,
    NCCHTYPE_ROMFS = 3,
} ctr_ncchtypes;

void ncch_getctr(const ncch_h *ncch, uint8_t *ctr, uint8_t type);

/* Crypto adaptation from b1l1s -> ctr9 tl;dr:

  aes_setiv       -> set_ctr
  aes_setkey      -> setup_aeskey{,X,Y}
  aes_use_keyslot -> use_aeskey
  aes_advctr      -> add_ctr
  aes             -> wrapper function thing
*/

#endif
