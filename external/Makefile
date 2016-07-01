out := ../$(dir_out)/$(fw_folder)

.PHONY: all
all: loader bits screeninit
	mkdir -p $(out)/module
	mkdir -p $(out)/bits
	cp loader/loader.cxi $(out)/module/loader.cxi
	cp bits/backdoor.bin $(out)/bits/backdoor.bin
	cp bits/emunand.bin $(out)/bits/emunand.bin
ifeq "$(CHAINLOADER)" "1"
	cp bits/chain.bin $(out)/bits/chain.bin
endif
	cp bits/reboot_hook.bin $(out)/bits/reboot_hook.bin
	cp bits/reboot_code.bin $(out)/bits/reboot_code.bin
	cp screeninit/build/screeninit.bin $(out)/bits/screeninit.bin

.PHONY: clean
clean: clean_loader clean_bits clean_screeninit
	rm -rf $(out)/bits
	rm -rf $(out)/module

.PHONY: loader
loader:
	make -C loader

.PHONY: bits
bits:
	make -C bits

.PHONY: screeninit
screeninit:
	make -C screeninit

.PHONY: clean_loader
clean_loader:
	make -C loader clean

.PHONY: clean_bits
clean_bits:
	make -C bits clean

.PHONY: clean_screeninit
screeninit_clean:
	make -C screeninit clean
