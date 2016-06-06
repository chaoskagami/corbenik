.PHONY: all
all: loader svc screeninit
	mkdir -p ../out/corbenik/module
	mkdir -p ../out/corbenik/svc
	mkdir -p ../out/corbenik/bits
	cp loader/loader.cxi ../out/corbenik/module/loader.cxi
	cp svc/7b.bin ../out/corbenik/svc/7b.bin
	cp svc/emunand.bin ../out/corbenik/bits/emunand.bin
	cp svc/reboot_hook.bin ../out/corbenik/bits/reboot_hook.bin
	cp svc/reboot_code.bin ../out/corbenik/bits/reboot_code.bin
	cp screeninit/build/screeninit.bin ../out/corbenik/bits/screeninit.bin

.PHONY: clean
clean: clean_loader clean_svc clean_screeninit
	rm -rf ../out/corbenik/svc
	rm -rf ../out/corbenik/module

.PHONY: loader
loader:
	make -C loader

.PHONY: svc
svc:
	make -C svc

.PHONY: screeninit
screeninit:
	make -C screeninit

.PHONY: clean_loader
clean_loader:
	make -C loader clean

.PHONY: clean_svc
clean_svc:
	make -C svc clean

.PHONY: clean_screeninit
screeninit_clean:
	make -C screeninit clean
