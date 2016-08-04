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

corbenik_SOURCES = patch/reboot.c patch/svc.c patch/module.c patch/emunand.c main.c option.c std/fs.c std/draw.c std/memory.c std/abort.c menu.c chain.c firm/version.c firm/firm.c firm/decryptor.c std/allocator.c interp.c input.c patcher.c display.c start.s interrupt.c screeninit.c
