.set firm_addr, 0x24000000  // Temporary location where we'll load the FIRM to
.set firm_maxsize, 0x200000  // Random value that's bigger than any of the currently known firm's sizes.

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

    // Convert 2 bytes of the path string
    // This will be the method of getting the lower 2 bytes of the title ID
    //   until someone bothers figuring out where the value is derived from.
    mov r0, #0  // Result
    add r1, sp, #0x3A8 - 0x70
    add r1, #0x22  // The significant bytes
    mov r2, #4  // Maximum loops (amount of bytes * 2)

    hex_string_to_int_loop:
        ldr r3, [r1], #2  // 2 because it's a utf-16 string.
        and r3, #0xFF

        // Check if it"s a number
        cmp r3, #'0'
        blo hex_string_to_int_end
        sub r3, #'0'
        cmp r3, #9
        bls hex_string_to_int_calc

        // Check if it"s a capital letter
        cmp r3, #'A' - '0'
        blo hex_string_to_int_end
        sub r3, #'A' - '0' - 0xA  // Make the correct value: 0xF >= al >= 0xA
        cmp r3, #0xF
        bls hex_string_to_int_calc

        // Incorrect value: x > "A"
        bhi hex_string_to_int_end

        hex_string_to_int_calc:
            orr r0, r3, r0, lsl #4
            subs r2, #1
            bne hex_string_to_int_loop
    hex_string_to_int_end:

    // Get the FIRM path
    cmp r0, #0x0002  // NATIVE_FIRM
    ldreq r1, firm_fname
    beq check_fname

    ldr r5, =0x0102  // TWL_FIRM
    cmp r0, r5
    ldreq r1, twl_firm_fname
    beq check_fname

    ldr r5, =0x0202  // AGB_FIRM
    cmp r0, r5
    ldreq r1, agb_firm_fname
    beq check_fname

    fallback:
        // Fallback: Load specified FIRM from exefs
        add r1, sp, #0x3A8-0x70  // Location of exefs string.
        b load_firm

    check_fname:
        // Check the given string offset
        cmp r1, #0
        beq fallback

    load_firm:
        // Open file
        add r0, r7, #8
        mov r2, #1
        ldr r6, fopen
        orr r6, #1
        blx r6

        cmp r0, #0  // Check if we were able to load the FIRM
        bne fallback  // Otherwise, try again with the FIRM from exefs.
        // This will loop indefinitely if the exefs FIRM fails to load, but whatever.

        // Read file
        mov r0, r7
        adr r1, bytes_read
        mov r2, #firm_addr
        mov r3, #firm_maxsize
        ldr r6, [sp, #0x3A8-0x198]
        ldr r6, [r6, #0x28]
        blx r6

    // Set kernel state
    mov r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    swi 0x7C

    // Jump to reboot code
    ldr r0, reboot_code
    swi 0x7B

    die:
        b die

.align 4
bytes_read:     .word 0
fopen:          .ascii "open"
reboot_code:    .ascii "rebc"
.pool
firm_fname:     .ascii "NATF"
twl_firm_fname: .ascii "TWLF"
agb_firm_fname: .ascii "AGBF"
