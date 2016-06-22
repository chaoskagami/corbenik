rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

# Only cygwin is maybe working on windows.
PATH := $(PATH):$(DEVKITARM)/bin

CROSS_CC := arm-none-eabi-gcc
CROSS_AS := arm-none-eabi-as
CROSS_LD := arm-none-eabi-ld
CROSS_OC := arm-none-eabi-objcopy

CC ?= gcc
AS ?= as
LD ?= ld
OC ?= objcopy

name := Corbenik

# If unset, the primary folder is /corbenik.
fw_folder ?= corbenik

dir_source := source
dir_data   := data
dir_build  := build
dir_out    := out

REVISION := $(shell git rev-parse HEAD | head -c10)+$(shell git rev-list --count HEAD)
REL ?= master

# Default to enabling chainloader.
CHAINLOADER ?= 1

CROSS_ASFLAGS := -mlittle-endian -mcpu=arm946e-s -march=armv5te
CROSS_CFLAGS  := -MMD -MP -Wall -Wextra -Werror -fomit-frame-pointer -Os $(ASFLAGS) -fshort-wchar -fno-builtin -std=gnu11 -DVERSION=\"$(REVISION)\" -DREL=\"$(REL)\" -DCHAINLOADER=$(CHAINLOADER)
CROSS_FLAGS   := dir_out=$(abspath $(dir_out)) --no-print-directory
CROSS_LDFLAGS := -nostdlib -Wl,-z,defs -lgcc -Wl,-Map,$(dir_build)/link.map

objects_cfw = $(patsubst $(dir_source)/%.s, $(dir_build)/%.o, \
			  $(patsubst $(dir_source)/%.c, $(dir_build)/%.o, \
			  $(call rwildcard, $(dir_source), *.s *.c)))

.PHONY: all
all: hosttools font a9lh patch external

.PHONY: release
release:
	rm -rf rel
	mkdir -p rel
	make clean
	make CHAINLOADER=0 REL=$(REL) full
	mv out/release.zip rel/release-nochain.zip
	cat out/release.zip.sha512 | sed 's/release.zip/release-nochain.zip/' > rel/release-nochain.zip.sha512
	make clean
	make CHAINLOADER=1 REL=$(REL) full
	mv out/release.zip out/release.zip.sha512 rel/

.PHONY: hosttools
hosttools:
	make -C host/bdfe dir_out=$(dir_out) fw_folder=$(fw_folder)

.PHONY: font
font: hosttools
	./host/conv-font-bin.sh
	mkdir -p $(dir_out)/$(fw_folder)/bits
	cp host/termfont.bin $(dir_out)/$(fw_folder)/bits/

.PHONY: full
full: all contrib $(dir_out)/$(fw_folder)/locale
	cp README.md LICENSE.txt $(dir_out)/
	cd out && zip -r9 release.zip *
	cd out && sha512sum -b release.zip > release.zip.sha512 # Security.

.PHONY: contrib
contrib:
	make -C contrib dir_out=$(dir_out) fw_folder=$(fw_folder)

.PHONY: external
external:
	make -C external dir_out=$(dir_out) fw_folder=$(fw_folder) CHAINLOADER=$(CHAINLOADER)

.PHONY: patch
patch:
	make -C patch dir_out=$(dir_out) fw_folder=$(fw_folder)

.PHONY: a9lh
a9lh: $(dir_out)/arm9loaderhax.bin
	# For the reboot patch.
	mkdir -p $(dir_out)/$(fw_folder)/bits
	cp $(dir_out)/arm9loaderhax.bin $(dir_out)/$(fw_folder)/bits/corbenik.bin

.PHONY: reformat
reformat:
	clang-format -i $(dir_source)/*.{c,h} $(dir_source)/*/*.{c,h} external/loader/source/*.{c,h}

$(dir_out)/$(fw_folder)/locale: all
	echo "Generating langemu data from 3dsdb - may take a bit"
	cd out/corbenik && ../../host/generate_langemu_conf.sh

.PHONY: clean
clean:
	rm -f host/{font-emit,font.h,font_prop.h,termfont.bin}
	make -C external dir_out=$(dir_out) fw_folder=$(fw_folder) clean
	make -C patch dir_out=$(dir_out) fw_folder=$(fw_folder) clean
	make -C host/bdfe dir_out=$(dir_out) fw_folder=$(fw_folder) clean
	rm -rf $(dir_out) $(dir_build)

.PHONY: $(dir_out)/arm9loaderhax.bin
$(dir_out)/arm9loaderhax.bin: $(dir_build)/main.bin
	@mkdir -p "$(dir_out)"
	@cp -av $< $@

$(dir_build)/main.bin: $(dir_build)/main.elf
	$(CROSS_OC) $(CROSS_OCFLAGS) -S -O binary $< $@

$(dir_build)/main.elf: $(objects_cfw)
	$(CROSS_CC) -T linker.ld $(OUTPUT_OPTION) $^ $(CROSS_LDFLAGS)

$(dir_build)/%.o: $(dir_source)/%.c
	@mkdir -p "$(@D)"
	$(CROSS_CC) $(CROSS_CFLAGS) -c $(OUTPUT_OPTION) $<

$(dir_build)/%.o: $(dir_source)/%.s
	@mkdir -p "$(@D)"
	$(CROSS_AS) $(CROSS_ASFLAGS) -c $(OUTPUT_OPTION) $<

$(dir_build)/fatfs/%.o: $(dir_source)/fatfs/%.c
	@mkdir -p "$(@D)"
	$(CROSS_CC) $(CROSS_CFLAGS) -c -Wno-unused-function $(OUTPUT_OPTION) $<

$(dir_build)/fatfs/%.o: $(dir_source)/fatfs/%.s
	@mkdir -p "$(@D)"
	$(CROSS_AS) $(CROSS_ASFLAGS) -c $(OUTPUT_OPTION) $<

$(dir_build)/std/%.o: $(dir_source)/std/%.c
	@mkdir -p "$(@D)"
	$(CROSS_CC) $(CROSS_CFLAGS) -c -Wno-unused-function $(OUTPUT_OPTION) $<

$(dir_build)/std/%.o: $(dir_source)/std/%.s
	@mkdir -p "$(@D)"
	$(CROSS_AS) $(CROSS_ASFLAGS) -c $(OUTPUT_OPTION) $<

$(dir_build)/firm/%.o: $(dir_source)/firm/%.c
	@mkdir -p "$(@D)"
	$(CROSS_CC) $(CROSS_CFLAGS) -c -Wno-unused-function $(OUTPUT_OPTION) $<

$(dir_build)/firm/%.o: $(dir_source)/firm/%.s
	@mkdir -p "$(@D)"
	$(CROSS_AS) $(CROSS_ASFLAGS) -c $(OUTPUT_OPTION) $<

$(dir_build)/patch/%.o: $(dir_source)/patch/%.c
	@mkdir -p "$(@D)"
	$(CROSS_CC) $(CROSS_CFLAGS) -c -Wno-unused-function $(OUTPUT_OPTION) $<

$(dir_build)/patch/%.o: $(dir_source)/patch/%.s
	@mkdir -p "$(@D)"
	$(CROSS_AS) $(CROSS_ASFLAGS) -c $(OUTPUT_OPTION) $<

include $(call rwildcard, $(dir_build), *.d)
