include $(top_srcdir)/common.mk

noinst_PROGRAMS = corbenik
corbenik_CFLAGS=$(AM_CFLAGS) -T$(srcdir)/linker.ld -nostartfiles
corbenik_LDFLAGS=$(AM_LDFLAGS) -lctr9
EXTRA_DIST = linker.ld

install:

all-local: corbenik.bin

clean-local:
	rm -f corbenik.bin

corbenik.bin:
	$(OBJCOPY) $(OCFLAGS) -O binary corbenik corbenik.bin

corbenikdir = $(top_srcdir)/source

inc_dir = $(top_srcdir)/include

corbenik_SOURCES = main.c option.c std/fs.c std/draw.c std/memory.c std/abort.c menu.c chain.c firm/firm.c firm/decryptor.c firm/fcram.c input.c display.c start.s interrupt.c screeninit.c
