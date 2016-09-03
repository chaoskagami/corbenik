#ifndef __FIRM_KEYS_H
#define __FIRM_KEYS_H

// See the D9 source code (by @d0k3) for info on these defines and structs:
//    https://github.com/d0k3/Decrypt9WIP/blob/master/source/decryptor/keys.h

#define KEY_ENCRYPT (1<<0)
#define KEY_DECRYPT (1<<1)

#define KEYS_UNKNOWN 0
#define KEYS_RETAIL 1
#define KEYS_DEVKIT 2

typedef struct {
    uint8_t slot;        // keyslot, 0x00...0x3F
    char type;           // type 'X' / 'Y' / 'N' for normalKey
    char id[10];         // key ID for special keys, all zero for standard keys
    uint8_t reserved[2]; // reserved space
    uint8_t isDevkitKey; // 0 for retail units / 1 for DevKit units
    uint8_t isEncrypted; // 0 if not / anything else if it is
    uint8_t key[16];
} __attribute__((packed)) aeskeydbent_t;

typedef struct {
	uint16_t    roll;
    uint8_t     sha[32];
	uint8_t     key[16];
} key_find_t;

#endif
