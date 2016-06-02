.PHONY: all
all: clean $(patsubst %.pco, %.vco, $(wildcard *.pco))
	mkdir -p ../out/corbenik/bin
	cp *.vco ../out/corbenik/bin/

%.vco: %.pco
	../host/bytecode_asm.py $< $@

clean:
	rm -f *.vco
