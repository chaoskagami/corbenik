#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <openssl/sha.h>

static const char  digits[] = "0123456789abcdef";
static char ascii_to_digit[256] = {0};

#define HASH_LEN 32 // Sha256
void hexdump(uint8_t* text, uint8_t* bin, int bin_len)
{
    for(int i=0; i < bin_len; text += 2, bin++, i++) {
        text[0]   = digits[(bin[0] >> 4) & 0xf];
        text[1]   = digits[bin[0] & 0xf];
	}
}

void unhexdump(uint8_t* bin, uint8_t* str, int bin_len)
{
    for(int i=0; i < bin_len; bin++, str += 2, i++) {
        bin[0] = (ascii_to_digit[str[0]] << 4) | ascii_to_digit[str[1]];
	}
}

uint8_t* sha256(uint8_t* data, uint32_t len) {
    uint8_t *hash = malloc(SHA256_DIGEST_LENGTH);

    SHA256_CTX sha256;

    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, len);
    SHA256_Final(hash, &sha256);

    return hash;
}

void init() {
    ascii_to_digit['0'] = 0;
    ascii_to_digit['1'] = 1;
    ascii_to_digit['2'] = 2;
    ascii_to_digit['3'] = 3;
    ascii_to_digit['4'] = 4;
    ascii_to_digit['5'] = 5;
    ascii_to_digit['6'] = 6;
    ascii_to_digit['7'] = 7;
    ascii_to_digit['8'] = 8;
    ascii_to_digit['9'] = 9;

    ascii_to_digit['A'] = 0xa;
    ascii_to_digit['B'] = 0xb;
    ascii_to_digit['C'] = 0xc;
    ascii_to_digit['D'] = 0xd;
    ascii_to_digit['E'] = 0xe;
    ascii_to_digit['F'] = 0xf;

    ascii_to_digit['a'] = 0xa;
    ascii_to_digit['b'] = 0xb;
    ascii_to_digit['c'] = 0xc;
    ascii_to_digit['d'] = 0xd;
    ascii_to_digit['e'] = 0xe;
    ascii_to_digit['f'] = 0xf;
}

uint16_t get_roll(uint8_t* data) {
    uint16_t roll = 0;
    for (int i=0; i < 16; i++) {
        roll += data[i];
    }

    return roll;
}

int main(int c, char **v) {
    uint8_t data[16];
    char textsha[65] = {0};

    if (c < 2) {
        printf("Usage: %s <key>\n", v[0]);
        return 1;
    }

    init();

    unhexdump(data, v[1], 16);

    uint16_t roll = get_roll(data);

    uint8_t* sha = sha256(data, 16);

	printf("{.roll = 0x%03X, .sha = {", v[1], roll);

    for (int i=0; i < 32; i++) {
        printf("0x%02X", sha[i]);
        if (i != 31)
            printf(", ");
    }
    printf("} }");

    free(sha);

    return 0;
}
