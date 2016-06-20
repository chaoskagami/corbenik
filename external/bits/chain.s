.syntax unified
.section .text
.global copy
copy: // void copy(uint8_t* data, uint32_t size)
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
