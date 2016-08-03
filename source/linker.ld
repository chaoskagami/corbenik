OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
    . = 0x23F00000;

    .text.start : {
        *(.text.start)
    }

    . = ALIGN(4);

    .text : {
        *(.text)
    }

    . = ALIGN(4);

    .data : {
        *(.data)
    }

    . = ALIGN(4);

    .rodata : {
        *(.rodata)
    }

    . = ALIGN(4);

    .rel : {
        *(.rel)
    }

    . = ALIGN(4);

    .symtab : {
        *(.symtab)
    }

    . = ALIGN(4);

    .bss : {
        __bss_start = .;
        *(.bss COMMON)
    }
    __bss_end = .;

    . = ALIGN(4);

    __end__ = 0x20400000;
}
