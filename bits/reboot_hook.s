.set load_addr,    0x24F00000
.set load_maxsize, 0x200000  // Random value that's bigger than corbenik

.section .text
.global _start
_start:
    // Interesting registers and locations to keep in mind, set before this code is ran:
    // - sp + 0x3A8 - 0x70: FIRM path in exefs.
    // - r7 (which is sp + 0x3A8 - 0x198): Reserved space for file handle
    // - *(sp + 0x3A8 - 0x198) + 0x28: fread function.

    pxi_wait_recv:
        ldr r2, =0x44846
        ldr r0, =0x10008000
        readPxiLoop1:
            ldrh r1, [r0, #4]
            lsls r1, #0x17
            bmi readPxiLoop1
            ldr r0, [r0, #0xC]
        cmp r0, r2
        bne pxi_wait_recv

    load_file:
        // Open file
        add r0, r7, #8
        adr r1, boot_fname
        mov r2, #1
        ldr r6, fopen
        orr r6, #1
        blx r6

        cmp r0, #0  // Check if we were able to load the FIRM
        bne die  // Otherwise, hang.

    read_file:
        // Read file to the proper base.
        mov r0, r7
        adr r1, bytes_read
        ldr r2, =load_addr
        ldr r3, =load_maxsize
        ldr r6, [r7]
        ldr r6, [r6, #0x28]
        blx r6

    // Set kernel state
    mov r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    swi 0x7C

    jump_to_kernel:
        // Jump to reboot code in kernel mode
        ldr r0, koffset_base
        add r0, pc
        swi 0x7B

    die:
        b die

bytes_read:      .word 0
fopen:           .ascii "OPEN"
koffset_base:    .word kernel_code-jump_to_kernel-12
.pool

kernel_code:
    // Disable MPU
    ldr r0, =0x42078
    mcr p15, 0, r0, c1, c0, 0

    // Flush cache
    mov r2, #0
    mov r1, r2
    flush_cache:
        mov r0, #0
        mov r3, r2, lsl #30
        flush_cache_inner_loop:
            orr r12, r3, r0, lsl#5
            mcr p15, 0, r12, c7, c14, 2  // clean and flush dcache entry (index and segment)
            add r0, #1
            cmp r0, #0x20
            bcc flush_cache_inner_loop
        add r2, #1
        cmp r2, #4
        bcc flush_cache

    mcr p15, 0, r1, c7, c10, 4  // drain write buffer

    mcr p15, 0, r1, c7, c5, 0   // Flush icache

    ldr r0, =load_addr
    bx r0

.pool
boot_fname:      // This gets appended at insert-time
