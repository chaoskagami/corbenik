/* IPS. Pretty standard stuff. */

#define IPS_MAGIC       "PATCH" // Original Spec
#define IPS_MAGIC_LENGTH 5

#define IPS_TAIL         "EOF"
#define IPS_TAIL_LENGTH  3

typedef struct ips_record_com_s {
	uint8_t offset[3];
	uint8_t size[2];
} ips_record_com_t;

typedef struct ips_record_rle_s {
	uint8_t  rle_size[2];
	uint8_t  byte;
} ips_record_rle_t;

typedef struct ips_record_s {
	ips_record_com_t* info;
	void* data; // Can be ips_record_rle_t or uint8_t.
} ips_record_t;

/* IPS32 is IPS with 32-bit offsets. */

#define IPS32_MAGIC        "IPS32" // 32-bit extension
#define IPS32_MAGIC_LENGTH 5

#define IPS32_TAIL         "EEOF"
#define IPS32_TAIL_LENGTH  4

typedef struct ips32_record_com_s {
	uint8_t offset[4];
	uint8_t size[2];
} ips32_record_com_t;

typedef struct ips32_record_rle_s {
	uint8_t  rle_size[2];
	uint8_t  byte;
} ips32_record_rle_t;

typedef struct ips32_record_s {
	ips32_record_com_t* info;
	void* data; // Can be ips_record_rle_t or uint8_t.
} ips32_record_t;

#define TYPE_INVALID 0
#define TYPE_IPS     1
#define TYPE_IPS32   2

#define BYTE4_TO_UINT32(bp) \
     (((uint32_t)(bp [0]) << 24) & 0xFF000000) | \
     (((uint32_t)(bp [1]) << 16) & 0x00FF0000) | \
     (((uint32_t)(bp [2]) << 8 ) & 0x0000FF00) | \
     ( (uint32_t)(bp [3])        & 0x000000FF)

#define BYTE3_TO_UINT32(bp) \
     (((uint32_t)(bp [0]) << 16) & 0x00FF0000) | \
     (((uint32_t)(bp [1]) << 8 ) & 0x0000FF00) | \
     ( (uint32_t)(bp [2])        & 0x000000FF)

#define BYTE2_TO_UINT16(bp) \
     (((uint16_t)(bp [0]) << 8 ) & 0xFF00) | \
     ( (uint16_t)(bp [1])        & 0x00FF)
