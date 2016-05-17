.section .text.start
.align 4
.global _start
_start:
	b main
_die:
	b _die // I have no clue how one would end up here.
