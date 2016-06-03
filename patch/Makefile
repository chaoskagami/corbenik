.PHONY: all
all: clean $(patsubst %.pco, %.vco, $(wildcard *.pco))
	mkdir -p ../out/corbenik/patch
	cp *.vco ../out/corbenik/patch/

%.vco: %.pco
	../host/bytecode_asm.py $< $@

clean:
	rm -f *.vco
