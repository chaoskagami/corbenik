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
.global copy
copy:
    ldr r3, value
    add r1, r0, r1

    inner:
        cmp r0, r1
        ldrbne r2, [r0], #1
        strbne r2, [r3, #1]!
        bne inner

boot:
    ldr r3, offset
    bx r3

value:  .int 0x23efffff
offset: .int 0x23f00000
