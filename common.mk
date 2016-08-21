INCPATHS=-I$(top_srcdir)/include -I$(top_srcdir)/external/libctr9/include

C9FLAGS=-mcpu=arm946e-s -march=armv5te -mlittle-endian -mword-relocations

SIZE_OPTIMIZATION = -Wl,--gc-sections -ffunction-sections

REVISION := $(shell git rev-parse HEAD | head -c10)+$(shell git rev-list --count HEAD)

AM_CFLAGS= -std=gnu11 -Os -g -ffast-math \
	-Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual \
	-Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op \
	-Wmissing-include-dirs -Wredundant-decls \
	-Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default \
	-Wundef -Wno-unused -Werror -Wno-error=cast-align -Wno-error=strict-overflow -Wno-error=pedantic \
	$(THUMBFLAGS) $(SIZE_OPTIMIZATION) $(INCPATHS) $(C9FLAGS) \
	-fno-builtin -std=gnu11 -DREVISION=\"$(REVISION)\" \
	-DFW_NAME=\"corbenik\" $(PATHARGS) -DMALLOC_DEBUG=1


AM_LDFLAGS=-Wl,--use-blx,--pic-veneer,-q -nostdlib -nodefaultlibs -Wl,-z,defs -lgcc \
	-lc -L$(top_srcdir)/external/libctr9/src

OCFLAGS=--set-section-flags .bss=alloc,load,contents
