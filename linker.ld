/* This memory map is mainly to assist in doing stuff in code.
MEMORY
{
    INSTRUCTION_TCM  (rw)   : ORIGIN = 0x00000000, LENGTH = 0x08000000
    ARM_INTERNAL     (rw)  : ORIGIN = 0x08000000, LENGTH = 0x00100000
    NEW_INTERNAL     (rw)  : ORIGIN = 0x08100000, LENGTH = 0x00080000
    IO_MEMORY        (rw)  : ORIGIN = 0x10000000, LENGTH = 0x08000000
    VRAM             (rw)  : ORIGIN = 0x18000000, LENGTH = 0x00600000
    DSP_MEMORY       (rw)  : ORIGIN = 0x1FF00000, LENGTH = 0x00080000
    AXI_WRAM         (rw)  : ORIGIN = 0x1FF80000, LENGTH = 0x00080000
    FCRAM            (rwx) : ORIGIN = 0x20000000, LENGTH = 0x08000000
    NEW_FCRAM        (rwx) : ORIGIN = 0x28000000, LENGTH = 0x08000000
    DATA_TCM         (rw)  : ORIGIN = 0xFFF00000, LENGTH = 0x00004000
    BOOTROM          (rw)   : ORIGIN = 0xFFFF0000, LENGTH = 0x00010000
} */

ENTRY(_start)
SECTIONS
{
    . = 0x23F00000;

    START_SECTION = .;
      .text.start : {
          *(.text.start)
      }
    START_SECTION_END = .;

    TEXT_SECTION = .;
      .text : {
          *(.text)
      }
    TEXT_SECTION_END = .;

    DATA_SECTION = .;
      .data : {
          *(.data)
      }
    DATA_SECTION_END = .;

    BSS_SECTION = .;
      .bss : {
          *(.bss COMMON)
      }
    BSS_SECTION_END = .;

    RODATA_SECTION = .;
      .rodata : {
          *(.rodata)
      }
    RODATA_SECTION_END = .;

    . = ALIGN(4);
}
