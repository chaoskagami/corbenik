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

corbenik_SOURCES = patch/reboot.c patch/svc.c patch/module.c patch/emunand.c main.c std/fs.c std/draw.c std/memory.c std/abort.c std/allocator.c menu.c firm/util.c firm/keys.c firm/firmlaunch.c firm/version.c firm/firm.c firm/decryptor.c interpreter.c input.c patcher.c chainloader.c config-backend-file.c menu-backend.c start.s interrupt.c arm11.c
