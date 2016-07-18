rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

include common.mk

objects_cfw = $(patsubst %.s, %.o, \
			  $(patsubst %.c, %.o, \
			  $(call rwildcard, $(dir_source), *.s *.c)))

.PHONY: all
all: hosttools font ctr9io a9lh patch external

.PHONY: ctr9io
ctr9io:
	cd external/libctr9 && autoreconf -fi && CFLAGS= LDFLAGS= ASFLAGS= ./configure --host arm-none-eabi --prefix=$(shell pwd)/external/libctr9/out && make && make install

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
	make -C host/bdfe dir_out=$(dir_out) fw_folder=$(fw_folder) root=$(root)

.PHONY: font
font: hosttools
	make -C host/bdfe
	mkdir -p $(dir_out)/$(fw_folder)/bits
	./host/bdfe/bdfe -A -n external/tewi-font/tewi-medium-11.bdf >  $(dir_out)/$(fw_folder)/bits/termfont.bin

.PHONY: full
full: all contrib $(dir_out)/$(fw_folder)/locale
	cp README.md LICENSE.txt $(dir_out)/
	cd out && zip -r9 release.zip *
	cd out && sha512sum -b release.zip > release.zip.sha512 # Security.

.PHONY: contrib
contrib:
	make -C contrib dir_out=$(dir_out) fw_name=$(fw_name) fw_folder=$(fw_folder) root=$(root)

.PHONY: external
external:
	make -C external dir_out=$(dir_out) fw_name=$(fw_name) fw_folder=$(fw_folder) CHAINLOADER=$(CHAINLOADER) root=$(root)

.PHONY: patch
patch:
	make -C patch dir_out=$(dir_out) fw_name=$(fw_name) fw_folder=$(fw_folder) root=$(root)

.PHONY: a9lh
a9lh: $(dir_out)/main.bin
	mkdir -p $(dir_out)/$(fw_folder)/bits

.PHONY: reformat
reformat:
	clang-format -i $(dir_source)/*.{c,h} $(dir_source)/*/*.{c,h} external/loader/source/*.{c,h}

$(dir_out)/$(fw_folder)/locale: all
	echo "Generating langemu data from 3dsdb - may take a bit"
	cd out/$(fw_folder) && ../../host/generate_langemu_conf.sh

.PHONY: clean
clean:
	rm -f $(objects_cfw)
	cd external/libctr9 && git clean -fxd
	make -C external dir_out=$(dir_out) fw_folder=$(fw_folder) root=$(root) clean
	make -C patch dir_out=$(dir_out) fw_folder=$(fw_folder) root=$(root) clean
	make -C host/bdfe dir_out=$(dir_out) fw_folder=$(fw_folder) root=$(root) clean
	rm -rf $(dir_out) $(dir_build)

out/main.bin: out/main.elf
	$(OC) $(OCFLAGS) -S -O binary $< $@

out/main.elf: $(objects_cfw)
	$(CC) $(LDFLAGS) -T linker.ld -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(AS) $(ASFLAGS) -c -o $@ $<

include $(call rwildcard, $(dir_build), *.d)
