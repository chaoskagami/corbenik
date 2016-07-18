SUBDIRS = include host external source patch contrib
ACLOCAL_AMFLAGS = -I m4

install:
	@echo "Please don't call the install target directly, it doesn't work."
	@exit 1

all-local:
	mkdir -p out
	mkdir -p out/@prefix@
	mkdir -p out/@libdir@/{module,firmware}
	mkdir -p out/@datarootdir@/{keys,locale/emu}
	mkdir -p out/@bindir@
	mkdir -p out/@sbindir@
	mkdir -p out/@prefix@/boot
	mkdir -p out/@libexecdir@
	mkdir -p out/@sysconfdir@
	mkdir -p out/@localstatedir@
	cp source/corbenik.bin out/arm9loaderhax.bin
	cp source/corbenik.bin out/@prefix@/boot/
	cp external/loader/loader.cxi out/@libdir@/module/loader.cxi
	cp external/bits/*.bin out/@libexecdir@/
	cp external/screeninit/build/screeninit.bin out/@libexecdir@/
	cp host/termfont.bin out/@libexecdir@/
	cp patch/*.vco out/@sbindir@
	cp contrib/*.vco out/@bindir@
