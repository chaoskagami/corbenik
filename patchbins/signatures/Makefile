rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

PATH := $(PATH):$(DEVKITARM)/bin

HOST_CC := gcc

CC := arm-none-eabi-gcc
AS := arm-none-eabi-as
LD := arm-none-eabi-ld
OC := arm-none-eabi-objcopy

dir_source := src
dir_build  := build
dir_out    := out

ASFLAGS := -mlittle-endian -mcpu=arm946e-s -march=armv5te
CFLAGS  := -MMD -MP -Wall -Wextra -Werror -Os $(ASFLAGS) -fno-builtin -std=c11 -DVERSION=\"$(REVISION)\"
FLAGS   := dir_out=$(abspath $(dir_out)) --no-print-directory
LDFLAGS := -nostdlib -Wl,-z,defs -lgcc

objects_cfw = $(patsubst $(dir_source)/%.s, $(dir_build)/%.o, \
			  $(patsubst $(dir_source)/%.c, $(dir_build)/%.o, \
			  $(call rwildcard, $(dir_source), *.s *.c)))

.PHONY: all
all: patchbin

.PHONY: patchbin
patchbin: tool $(dir_out)/patch.bin
	./compile_header

.PHONY: tool
tool:
	$(HOST_CC) -o compile_header tool/compile_header.c


.PHONY: clean
clean:
	rm -rf $(dir_out) $(dir_build) compile_header

.PHONY: $(dir_out)/patch.bin
$(dir_out)/patch.bin: $(dir_build)/main.bin
	@mkdir -p "$(dir_out)"
	@cp -av $< $@

$(dir_build)/main.bin: $(dir_build)/main.elf
	$(OC) -S -O binary $< $@

$(dir_build)/main.elf: $(objects_cfw)
	$(CC) -T linker.ld $(OUTPUT_OPTION) $^ $(LDFLAGS)

$(dir_build)/%.o: $(dir_source)/%.c
	@mkdir -p "$(@D)"
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(dir_build)/%.o: $(dir_source)/%.s
	@mkdir -p "$(@D)"
	$(COMPILE.s) $(OUTPUT_OPTION) $<

include $(call rwildcard, $(dir_build), *.d)
