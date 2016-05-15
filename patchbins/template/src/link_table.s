.section .text.table
.align 4

.macro stub name
	.global \name
	\name :
		ldr pc, [pc, #-4] // Load the data after this to the PC. return will be before this call.
		.byte 0 // Read; pc relative in arm mode is pc+8, so pc-4 is the correct offset
		.byte 0
		.byte 0
		.byte 0
.endm

.macro ref name
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
ref memory_offset

// Size of memory offset. (uint32_t*)
ref memory_len

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
