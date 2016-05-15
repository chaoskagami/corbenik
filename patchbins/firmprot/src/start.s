.section .text.start
.align 4
.global _start
_start:
    bl main
    bx lr @ return from patch.
