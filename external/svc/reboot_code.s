.set firm_addr, 0x24000000  // Temporary location where we'll load the FIRM to
.set firm_maxsize, 0x200000  // Random value that's bigger than any of the currently known firm's sizes.

.section .text
.global _start
_start:

    // Set MPU settings
    mrc p15, 0, r0, c2, c0, 0  // dcacheable
    mrc p15, 0, r12, c2, c0, 1  // icacheable
    mrc p15, 0, r1, c3, c0, 0  // write bufferable
    mrc p15, 0, r2, c5, c0, 2  // daccess
    mrc p15, 0, r3, c5, c0, 3  // iaccess
    ldr r4, =0x18000035  // 0x18000000 128M
    bic r2, r2, #0xF0000  // unprotect region 4
    bic r3, r3, #0xF0000  // unprotect region 4
    orr r0, r0, #0x10  // dcacheable region 4
    orr r2, r2, #0x30000  // region 4 r/w
    orr r3, r3, #0x30000  // region 4 r/w
    orr r12, r12, #0x10  // icacheable region 4
    orr r1, r1, #0x10  // write bufferable region 4
    mcr p15, 0, r0, c2, c0, 0
    mcr p15, 0, r12, c2, c0, 1
    mcr p15, 0, r1, c3, c0, 0  // write bufferable
    mcr p15, 0, r2, c5, c0, 2  // daccess
    mcr p15, 0, r3, c5, c0, 3  // iaccess
    mcr p15, 0, r4, c6, c4, 0  // region 4 (hmmm)

    mrc p15, 0, r0, c2, c0, 0  // dcacheable
    mrc p15, 0, r1, c2, c0, 1  // icacheable
    mrc p15, 0, r2, c3, c0, 0  // write bufferable
    orr r0, r0, #0x20  // dcacheable region 5
    orr r1, r1, #0x20  // icacheable region 5
    orr r2, r2, #0x20  // write bufferable region 5
    mcr p15, 0, r0, c2, c0, 0  // dcacheable
    mcr p15, 0, r1, c2, c0, 1  // icacheable
    mcr p15, 0, r2, c3, c0, 0  // write bufferable

    // Copy the firmware
    mov r4, #firm_addr
    add r5, r4, #0x40  // Start of loop
    add r6, r5, #0x30 * 3  // End of loop (scan 4 entries)

    copy_firm_loop:
        ldr r0, [r5]
        cmp r0, #0
        addne r0, r4  // src
        ldrne r1, [r5, #4]  // dest
        ldrne r2, [r5, #8]  // size
        blne memcpy32

        cmp r5, r6
        addlo r5, #0x30
        blo copy_firm_loop

    // Flush cache
    mov r2, #0
    mov r1, r2
    flush_cache:
        mov r0, #0
        mov r3, r2, lsl #30
        flush_cache_inner_loop:
            orr r12, r3, r0, lsl#5
            mcr p15, 0, r1, c7, c10, 4  // drain write buffer
            mcr p15, 0, r12, c7, c14, 2  // clean and flush dcache entry (index and segment)
            add r0, #1
            cmp r0, #0x20
            bcc flush_cache_inner_loop
        add r2, #1
        cmp r2, #4
        bcc flush_cache

    // Enable MPU
    ldr r0, =0x42078  // alt vector select, enable itcm
    mcr p15, 0, r0, c1, c0, 0
    mcr p15, 0, r1, c7, c5, 0  // flush dcache
    mcr p15, 0, r1, c7, c6, 0  // flush icache
    mcr p15, 0, r1, c7, c10, 4  // drain write buffer
    mov r0, #firm_addr

    // Boot FIRM
    mov r1, #0x1FFFFFFC
    ldr r2, [r0, #8]  // arm11 entry
    str r2, [r1]
    ldr r0, [r0, #0xC]  // arm9 entry
    bx r0
.pool

memcpy32:  // memcpy32(void *src, void *dst, unsigned int size)
    add r2, r0
    memcpy32_loop:
        ldmia r0!, {r3}
        stmia r1!, {r3}
        cmp r0, r2
        blo memcpy32_loop
    bx lr
