.PHONY: all copyout
all: template
	mkdir -p ../out/corbenik/bin
	cp template/out/patch.vco ../out/corbenik/bin/example.vco

.PHONY: clean
clean: clean_template
	rm -rf ../out/corbenik/bin

.PHONY: template
template:
	make -C template

.PHONY: clean_template
clean_template:
	make -C template clean
