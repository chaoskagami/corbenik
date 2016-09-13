SUBDIRS = include host external source patch contrib
ACLOCAL_AMFLAGS = -I m4

install:
	@echo "Please don't call the install target directly, it doesn't work."
	@exit 1

all-local:
	mkdir -p out
	mkdir -p out@prefix@
	mkdir -p out@libdir@/module
	mkdir -p out@libdir@/module/native
	mkdir -p out@libdir@/module/twl
	mkdir -p out@libdir@/module/agb
	mkdir -p out@libdir@/firmware
	mkdir -p out@datarootdir@/keys
	mkdir -p out@datarootdir@/locale/emu
	mkdir -p out@bindir@
	mkdir -p out@sbindir@
	mkdir -p out@prefix@/boot
	mkdir -p out@libexecdir@
	mkdir -p out@sysconfdir@
	mkdir -p out@localstatedir@
	cp source/corbenik.bin out/arm9loaderhax.bin
	cp external/loader/loader.cxi out@libdir@/module/native/loader.cxi
	cp external/bits/*.bin out@libexecdir@/
	cp host/termfont.bin out@datarootdir@/
	cp patch/*.vco out@sbindir@
	cp contrib/*.vco out@bindir@
	cat host/generate_localeemu.sh | sed "s|\@localedir\@|@localedir@|g" > out/generate_localeemu.sh
	chmod +x out/generate_localeemu.sh
	echo "#!/bin/sh" > out/o3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000002/00000056 -O .@libdir@/firmware/native"       >> out/o3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000002/cetk     -O .@datarootdir@/keys/native.cetk" >> out/o3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000102/00000016 -O .@libdir@/firmware/twl"       >> out/o3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000102/cetk     -O .@datarootdir@/keys/twl.cetk" >> out/o3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000202/0000000B -O .@libdir@/firmware/agb"       >> out/o3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000202/cetk     -O .@datarootdir@/keys/agb.cetk" >> out/o3ds_firm.sh
	echo "#!/bin/sh" > out/n3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000002/00000026 -O .@libdir@/firmware/native"       >> out/n3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000002/cetk     -O .@datarootdir@/keys/native.cetk" >> out/n3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000102/00000000 -O .@libdir@/firmware/twl"       >> out/n3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000102/cetk     -O .@datarootdir@/keys/twl.cetk" >> out/n3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000202/00000000 -O .@libdir@/firmware/agb"       >> out/n3ds_firm.sh
	echo "wget http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000202/cetk     -O .@datarootdir@/keys/agb.cetk" >> out/n3ds_firm.sh
	chmod 755 out/*.sh
	cp README.md LICENSE.txt out/

clean-local:
	rm -rf out
