#ifdef FIRM_INTERNAL_CODE

#ifndef __FIRM_INTENAL
#define __FIRM_INTERNAL

#define SECTOR_SIZE 0x200

#define ROLL_WINDOW    AES_BLOCK_SIZE
#define MODULO_WINDOW  (ROLL_WINDOW / 2)

typedef void (*void_call)();

void     firmlaunch(firm_h* firm);
uint8_t* slice_roll_search(uint8_t *mem, uint32_t size, key_find_t* find);

int set_Y3D_cetk(uint32_t commonKeyIndex);
int set_N11_K9L(uint32_t index);

int      decrypt_k9l(arm9bin_h *header, enum firm_type type, uint32_t k9l);

int      patch_entry(firm_h *firm, enum firm_type type);

void*    find_section_key(firm_h *firm_loc);
int      patch_section_keys(firm_h* firm_loc, uint32_t k9l);

int      dec_k9l(firm_h* firm);

uint8_t* get_titlekey(char *cetk_filename);

//int      decrypt_ncch(ncch_h *ncch, uint8_t* titlekey, size_t size);
//exefs_h *get_exefs(ncch_h *ncch);
//int      decrypt_exefs(ncch_h *ncch);

firm_h  *extract_firm_from_ncch(ncch_h *ncch, uint8_t* titlekey, size_t size);

#endif
#else
  #error "Do not include internal headers directly."
#endif
