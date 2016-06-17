.PHONY: all
all: build install

.PHONY: build
build: clean $(patsubst %.pco, %.vco, $(wildcard *.pco))

.PHONY: install
install: build
	mkdir -p ../out/corbenik/contrib
	cp *.vco README.md ../out/corbenik/contrib/

%.vco: %.pco
	../host/bytecode_asm.py $< $@

clean:
	rm -f *.vco
