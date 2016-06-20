.PHONY: all
all: loader bits screeninit
	mkdir -p ../out/corbenik/module
	mkdir -p ../out/corbenik/bits
	cp loader/loader.cxi ../out/corbenik/module/loader.cxi
	cp bits/backdoor.bin ../out/corbenik/bits/backdoor.bin
	cp bits/emunand.bin ../out/corbenik/bits/emunand.bin
	cp bits/chain.bin ../out/corbenik/bits/chain.bin
	cp bits/reboot_hook.bin ../out/corbenik/bits/reboot_hook.bin
	cp bits/reboot_code.bin ../out/corbenik/bits/reboot_code.bin
	cp screeninit/build/screeninit.bin ../out/corbenik/bits/screeninit.bin

.PHONY: clean
clean: clean_loader clean_bits clean_screeninit
	rm -rf ../out/corbenik/bits
	rm -rf ../out/corbenik/module

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
