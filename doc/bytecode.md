Bytecode format
===================

All instructions are dwords (four bytes) - this is for optimization reasons.

Registers
-------------------
r1-r4 - General
pc    - Current offset in bytecode
mem   - Memory offset set.

Instruction List
-------------------
1) call index
2) add reg, reg
3) sub reg, reg
4) mul reg, reg
5) div reg, reg
6) mov reg/ref/imm, reg/ref/imm
7) push reg
8) pop <reg>
9) .byte byte

