.section .text.start
.align 4
.global _start
_start:
	b main

.macro stub name
	.global \name
	\name :
		ldr pc, [pc, #-4] // Load the data after this to the PC. return will be before this call.
		bx lr // Fall through in case of error.
.endm

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

// Wrappers to get shit.

// Get NATIVE_FIRM process9
stub get_firm_proc9_exefs

// Get AGB_FIRM process9 exefs
stub get_agb_proc9_exefs

// Get TWL_FIRM process9 exefs
stub get_twl_proc9_exefs
