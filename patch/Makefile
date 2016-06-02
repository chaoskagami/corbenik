.PHONY: all
all: $(patsubst %.pco, %.vco, $(wildcard *.pco))

%.vco: %.pco
	../host/bytecode_asm.py $< $@

clean:
	rm -f *.vco
