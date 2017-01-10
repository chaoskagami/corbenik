.PHONY: all
all: build

.PHONY: build
build: clean $(patsubst %.pco, %.vco, $(wildcard *.pco))

.PHONY: install
install:

%.vco: %.pco
	../../host/bytecode_asm.py $< $@

clean:
	rm -f *.vco
