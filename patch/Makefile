.PHONY: all
all: build install

.PHONY: build
build: clean $(patsubst %.pco, %.vco, $(wildcard *.pco))

.PHONY: install
install: build
	mkdir -p ../out/corbenik/patch
	cp *.vco ../out/corbenik/patch/

%.vco: %.pco
	../host/bytecode_asm.py $< $@

clean:
	rm -f *.vco
