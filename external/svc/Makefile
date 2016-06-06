PATH := $(PATH):$(DEVKITARM)/bin

all: 7b.bin stub.bin emunand.bin reboot_hook.bin reboot_code.bin

%.o: %.s
	arm-none-eabi-as -o $@ $<

%.elf: %.o
	arm-none-eabi-ld -T link.ld -o $@ $<

%.bin: %.elf
	arm-none-eabi-objcopy -O binary $< $@

.PHONY: clean
clean:
	rm -f *.bin *.elf *.o
