.section .text.start
.align 4
.global _start
_start:
    // Change the stack pointer
    mov sp, #0x27000000

    // Give read/write access to all the memory regions
    ldr r5, =0x33333333
    mcr p15, 0, r5, c5, c0, 2 @ write data access
    mcr p15, 0, r5, c5, c0, 3 @ write instruction access

    // Sets MPU permissions and cache settings
    ldr r0, =0xFFFF001D	@ ffff0000 32k
    ldr r1, =0x01FF801D	@ 01ff8000 32k
    ldr r2, =0x08000027	@ 08000000 1M
    ldr r3, =0x10000021	@ 10000000 128k
    ldr r4, =0x10100025	@ 10100000 512k
    ldr r5, =0x20000035	@ 20000000 128M
    ldr r6, =0x1FF00027	@ 1FF00000 1M
    ldr r7, =0x1800002D	@ 18000000 8M
    mov r10, #0x25
    mov r11, #0x25
    mov r12, #0x25
    mcr p15, 0, r0, c6, c0, 0
    mcr p15, 0, r1, c6, c1, 0
    mcr p15, 0, r2, c6, c2, 0
    mcr p15, 0, r3, c6, c3, 0
    mcr p15, 0, r4, c6, c4, 0
    mcr p15, 0, r5, c6, c5, 0
    mcr p15, 0, r6, c6, c6, 0
    mcr p15, 0, r7, c6, c7, 0

	// Yes, you're reading correctly. We are NOT enabling instruction caching.
	// It causes issues with the linker, which is why it is left off.

    // Fixes mounting of SDMC
	ldr r0, =0x10000020
	mov r1, #0x340
	str r1, [r0]

    bl main

.die:
    b .die
