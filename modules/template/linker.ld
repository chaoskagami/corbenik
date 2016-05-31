ENTRY(main)
SECTIONS
{
    . = 0x24400000;

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
