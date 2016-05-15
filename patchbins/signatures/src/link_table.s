.section .text.table
.align 4

.macro stub name
	.global \name
	\name :
		.byte 0
		.byte 0
		.byte 0
		.byte 0
.endm

.global MAGIC_START
MAGIC_START:
	.byte 0xc0
	.byte 0x9b
	.byte 0xe5
	.byte 0x1c

// Memory to patch as specified by the header. (uint8_t*)
stub memory_offset

// Size of memory offset. (uint32_t*)
stub memory_len

// Exported functions.

// memory.c
stub strlen
stub isprint
stub memcpy
stub memmove
stub memset
stub memcmp
stub strncpy
stub strncmp
stub atoi
stub memfind

// draw.c
stub putc
stub puts
stub fprintf
