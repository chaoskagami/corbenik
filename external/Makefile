.PHONY: all copyout
all: loader
	mkdir -p ../out/corbenik/lib/module
	cp loader/loader.cxi ../out/corbenik/lib/module/loader.cxi

.PHONY: clean
clean: clean_loader
	rm -rf ../out/corbenik/bin

.PHONY: loader
loader:
	make -C loader

.PHONY: clean_loader
clean_loader:
	make -C loader clean
