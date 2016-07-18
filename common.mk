INCPATHS=-I$(top_srcdir)/include -I$(top_srcdir)/external/libctr9/include

C9FLAGS=-mcpu=arm946e-s -march=armv5te -mlittle-endian -mword-relocations

SIZE_OPTIMIZATION = -Wl,--gc-sections -ffunction-sections

AM_CFLAGS= -std=gnu11 -O2 -g -fomit-frame-pointer -ffast-math \
	-Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual \
	-Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op \
	-Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls \
	-Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default \
	-Wundef -Wno-unused $(THUMBFLAGS) $(SIZE_OPTIMIZATION) $(INCPATHS) $(C9FLAGS) \
	-Os $(ASFLAGS) \
	-fshort-wchar -fno-builtin -std=gnu11 -DVERSION=\"$(REVISION)\" -DREL=\"$(REL)\" \
	-DCHAINLOADER=$(CHAINLOADER) -DPATH_CFW=\"/$(fw_folder)\" -DFW_NAME=\"$(fw_name)\" \
	$(PATHARGS)


AM_LDFLAGS=-Wl,--use-blx,--pic-veneer,-q -nostdlib -Wl,-z,defs -lgcc \
	-L$(top_srcdir)/external/libctr9/src

OCFLAGS=--set-section-flags .bss=alloc,load,contents

# Only cygwin is maybe working on windows.
PATH := $(PATH):$(DEVKITARM)/bin

fw_name ?= Corbenik

# If unset, the primary folder is /corbenik.
fw_folder ?= corbenik

# Other valid options:
#  shadowhand
#  cruel
root ?= clusterfuck

# ifeq "$(root)" "clusterfuck"
#  PATHARGS := -DPATH_ROOT=\"\"
#  PATHARGS += -DPATH_DATA=\"/$(fw_folder)\"
# else ifeq "$(root)" "cruel"
#  PATHARGS := -DPATH_ROOT=\"/3ds/apps\"
#  PATHARGS += -DPATH_DATA=\"/3ds/appdata/$(fw_folder)\"
# else ifeq "$(root)" "shadowhand"
#  PATHARGS := -DPATH_ROOT=\"/homebrew/3ds\"
#  PATHARGS += -DPATH_DATA=\"/homebrew/3ds/$(fw_folder)\"
# endif

# dir_source := source
# dir_data   := data
# dir_build  := build
# dir_out    := out

REVISION := $(shell git rev-parse HEAD | head -c10)+$(shell git rev-list --count HEAD)
REL ?= master

# Default to enabling chainloader.
CHAINLOADER ?= 1

