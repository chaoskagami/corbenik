include $(top_srcdir)/common.mk

noinst_PROGRAMS = bootstub
bootstub_CFLAGS=$(AM_CFLAGS) -T$(srcdir)/linker.ld -nostartfiles
bootstub_LDFLAGS=$(AM_LDFLAGS) -lctr9 -Wl,-Map,bootstub.map
EXTRA_DIST = linker.ld

install:

all-local: bootstub.bin

clean-local:
	rm -f bootstub.bin

bootstub.bin: bootstub
	$(OBJCOPY) $(OCFLAGS) -O binary bootstub bootstub.bin

bootstubdir = $(top_srcdir)/source

inc_dir = $(top_srcdir)/include

bootstub_SOURCES = \
	start.s fs.c memory.c chainloader.c
