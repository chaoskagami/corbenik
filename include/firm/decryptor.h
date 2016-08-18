#ifndef __DECRYPTOR_H
#define __DECRYPTOR_H

/* Sha256Sums data quickly, outputting result to a buffer.
 *
 * \param sum Buffer to output sha256 to.
 * \param data Data to calculate sha256 over
 * \param size Size, in bytes, of data to sha256
 */
void sha256sum(void* sum, void* data, uint32_t size);

typedef enum {
    NCCHTYPE_EXHEADER = 1, ///< ExHeader CTR
    NCCHTYPE_EXEFS = 2,    ///< ExeFs CTR
    NCCHTYPE_ROMFS = 3,    ///< RomFs CTR
} ctr_ncchtypes;

/* Gets the initial counter for CTR crypto.
 *
 * \param ncch NCCH in memory to get CTR for
 * \param ctr Output buffer of 16 bytes to output ctr to.
 * \param type type of CTR value to get from NCCH, one of ctr_ncchtypes
 */
void ncch_getctr(const ncch_h *ncch, uint8_t *ctr, uint8_t type);

#endif
