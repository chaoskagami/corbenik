// This is a tiny chainloader. It expects r0 to be the
// code to copy, and r1 to be the code's size.

// As long as GCC follows standard calling conventions, you
// can call it from C once in memory like:

//   void copy(uint8_t* data, uint32_t size)

// This means NO need to use fatfs with the chainloader since the
// caller itself handles the disk read.

// The code below is also all PC-relative, meaning you can actually
// run the chainloader from anywhere (as long as it is aligned to
// instruction boundaries/the chainloader isn't overwritten/the
// code isn't copied wrong over itself)

.syntax unified
.section .text
copy:
    ldr r3, value
    add r1, r0, r1

    inner:
        cmp r0, r1
        ldrbne r2, [r0], #1
        strbne r2, [r3, #1]!
        bne inner

boot:
    // Flush caches

    // ICache
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 0

    // DCache
    mov r2, #0
    mov r1, r2
    flush_dcache:
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
        bcc flush_dcache

    // Reload argc and argv.
    ldr r0, argc
    ldr r1, argv

    // Actually boot payload.
    ldr r3, offset
    bx r3

.align 4

value:  .int 0x23efffff
offset: .int 0x23f00000
argc:   .ascii "ARGC"
argv:   .ascii "ARGV"
