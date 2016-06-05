.PHONY: all
all: loader svc
	mkdir -p ../out/corbenik/module
	mkdir -p ../out/corbenik/svc
	cp loader/loader.cxi ../out/corbenik/module/loader.cxi
	cp svc/7b.bin ../out/corbenik/svc/7b.bin

.PHONY: clean
clean: clean_loader clean_svc
	rm -rf ../out/corbenik/svc
	rm -rf ../out/corbenik/module

.PHONY: loader
loader:
	make -C loader

.PHONY: clean_loader
clean_loader:
	make -C loader clean

.PHONY: svc
svc:
	make -C svc

.PHONY: clean_svc
clean_svc:
	make -C svc clean
