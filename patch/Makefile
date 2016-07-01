out := ../$(dir_out)/$(fw_folder)

.PHONY: all
all: build install

.PHONY: build
build: clean $(patsubst %.pco, %.vco, $(wildcard *.pco))

.PHONY: install
install: build
	mkdir -p $(out)/patch
	cp *.vco $(out)/patch/

%.vco: %.pco
	../host/bytecode_asm.py $< $@

clean:
	rm -f *.vco
