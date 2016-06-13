.PHONY: all
all: build install

.PHONY: build
build: clean $(patsubst %.pco, %.vco, $(wildcard *.pco))
	mkdir -p ../out/corbenik/patch
	cp *.vco ../out/corbenik/patch/

.PHONY: install
install: build
	mkdir -p ../out/contrib
	cp *.vco ../out/contrib/

%.vco: %.pco
	../host/bytecode_asm.py $< $@

clean:
	rm -f *.vco
