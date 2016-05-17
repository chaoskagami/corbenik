.PHONY: all copyout
all: loader
	mkdir -p ../out/corbenik/module
	cp loader/loader.cxi ../out/corbenik/module/loader.cxi

.PHONY: clean
clean: clean_loader
	rm -rf ../out/corbenik/module

.PHONY: loader
loader:
	make -C loader

.PHONY: clean_loader
clean_loader:
	make -C loader clean
