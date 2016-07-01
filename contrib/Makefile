.PHONY: all
all: build install

.PHONY: build
build: clean $(patsubst %.pco, %.vco, $(wildcard *.pco))

.PHONY: install
install: build
	mkdir -p $(out)/contrib
	cp *.vco README.md $(out)/contrib/

%.vco: %.pco
	../host/bytecode_asm.py $< $@

clean:
	rm -f *.vco
