.section .text.start
.align 4
.global _start
_start:
	stmdb sp!, {lr}
	bl main
	ldmia sp!, {lr}
    bx lr // return from patch.
