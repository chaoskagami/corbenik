PATH := $(PATH):$(DEVKITARM)/bin

SOURCES=$(wildcard *.s)
OBJECTS=$(patsubst %.s, %.bin, $(SOURCES))

all: $(OBJECTS)

%.o: %.s
	arm-none-eabi-as -I$(shell pwd)/../include -o $@ $<

%.elf: %.o
	arm-none-eabi-ld -T link.ld -o $@ $<

%.bin: %.elf
	arm-none-eabi-objcopy -O binary $< $@

.PHONY: clean
clean:
	rm -f *.bin *.elf *.o

.PHONY: install
install:
