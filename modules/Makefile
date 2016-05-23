.PHONY: all copyout
all: template signatures
	mkdir -p ../out/corbenik/bin
	cp template/out/patch.vco ../out/corbenik/bin/example.vco
	cp signatures/out/patch.vco ../out/corbenik/bin/signatures.vco

.PHONY: clean
clean: clean_template clean_signatures
	rm -rf ../out/corbenik/bin

.PHONY: template
template:
	make -C template

.PHONY: signatures
signatures:
	make -C signatures

.PHONY: clean_template
clean_template:
	make -C template clean

.PHONY: clean_signatures
clean_signatures:
	make -C signatures clean
