SUBDIRS = include host external boot patch loader bits

ACLOCAL_AMFLAGS = -I m4

boot: include host external patch

external: include host

loader: external

install:
	@echo "Please don't call the install target directly, it doesn't work."
	@exit 1

all-local: external loader boot bits patch
	mkdir -p out
	mkdir -p out@prefix@
	mkdir -p out@libdir@/module
	mkdir -p out@libdir@/module/native
	mkdir -p out@libdir@/module/twl
	mkdir -p out@libdir@/module/agb
	mkdir -p out@libdir@/firmware
	mkdir -p out@datarootdir@/locale/emu
	mkdir -p out@bindir@
	mkdir -p out@sbindir@
	mkdir -p out@prefix@/boot
	mkdir -p out@libexecdir@
	mkdir -p out@sysconfdir@
	mkdir -p out@localstatedir@
	cp boot/corbenik.bin out/arm9loaderhax.bin
	cp boot/corbenik out/corbenik.elf
	cp loader/loader.cxi out@libdir@/module/native/loader.cxi
	cp bits/*.bin out@libexecdir@/
	cp host/termfont.bin out@datarootdir@/
	cp patch/*.vco out@sbindir@
	cp external/contrib_patch/*.vco out@bindir@
	cat host/generate_localeemu.sh.in | sed "s|\@localedir\@|@localedir@|g" > out/generate_localeemu.sh
	chmod +x out/generate_localeemu.sh
	cat host/o3ds_firm.sh.in | sed "s|\@libdir\@|@libdir@|g" > out/o3ds_firm.sh
	chmod +x out/o3ds_firm.sh
	cat host/n3ds_firm.sh.in | sed "s|\@libdir\@|@libdir@|g" > out/n3ds_firm.sh
	chmod +x out/n3ds_firm.sh
	chmod 755 out/*.sh
	cp README.md LICENSE.txt out/

clean-local:
	rm -rf out
