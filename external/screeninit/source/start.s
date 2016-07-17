.section .text.start
.align 4
.global _start
_start:
    // Disable interrupts
    CPSID aif

    b main

do_init_or_deinit: .int 0
brightness: .int 0
mode:       .int 0
