.section .text
.global _start
_start:
    // Original code that still needs to be executed.
    mov r4, r0
    mov r5, r1
    mov r7, r2
    mov r6, r3

main:
    // If we're already trying to access the SD, return.
    ldr r2, [r0, #4]
    ldr r1, sdmmc // In armips this instruction uses pc-releative loading
    cmp r2, r1
    beq nand_sd_ret

    str r1, [r0, #4]  // Set object to be SD
    ldr r2, [r0, #8]  // Get sector to read
	cmp r2, #0        // Gateway compat

    ldr r3, nand_offset // ^ see above
    add r2, r3  // Add the offset to the NAND in the SD.

    ldreq r3, ncsd_offset // ^ see above
    addeq r2, r3

    str r2, [r0, #8]  // Store sector to read

    nand_sd_ret:
        // Restore registers.
        mov r0, r4
        mov r1, r5
        mov r2, r7
        mov r3, r6

        // Return 4 bytes behind where we got called,
        // due to the offset of this function being stored there.
        mov r0, lr
        add r0, #4
        bx  r0

sdmmc:              .ascii "SDMC" // The offset of the sdmmc object.
nand_offset:        .ascii "NAND" // The starting offset of the emuNAND on the SD.
ncsd_offset:        .ascii "NCSD" // Location of the NCSD header relative to nand_offset

