.section .text.table
.align 4

.macro stub name
	.global \name
	\name :
		ldr pc, [pc, #-4] // Load the data after this to the PC. return will be before this call.
		bx lr // Fall through in case of error.
.endm

// (int) [0]
.global MAGIC_START
MAGIC_START:
	.byte 0xc0
	.byte 0x9b
	.byte 0xe5
	.byte 0x1c

// Exported functions.

// memory.c
// (int) [3]
stub strlen
// (int) [5]
stub isprint
// (int) [7]
stub memcpy
// (int) [9]
stub memmove
// (int) [11]
stub memset
// (int) [13]
stub memcmp
// (int) [15]
stub strncpy
// (int) [17]
stub strncmp
// (int) [19]
stub atoi
// (int) [21]
stub memfind

// draw.c
// (int) [23]
stub putc
// (int) [25]
stub puts
// (int) [27]
stub fprintf

// Wrappers to get shit.

// Gets NATIVE_FIRM memory offset as (firm_h*)
stub get_firm

// Get NATIVE_FIRM process9
stub get_firm_proc9_exefs

// Gets AGB_FIRM.
stub get_agb

// Get AGB_FIRM process9 exefs
stub get_agb_proc9_exefs

// Gets TWL_FIRM.
stub get_twl

// Get TWL_FIRM process9 exefs
stub get_twl_proc9_exefs
