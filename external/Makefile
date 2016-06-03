.PHONY: all
all: loader service
	mkdir -p ../out/corbenik/module
	mkdir -p ../out/corbenik/service
	cp loader/loader.cxi ../out/corbenik/module/loader.cxi
	cp service/7b.bin ../out/corbenik/service/7b.bin

.PHONY: clean
clean: clean_loader clean_service
	rm -rf ../out/corbenik/service
	rm -rf ../out/corbenik/module

.PHONY: loader
loader:
	make -C loader

.PHONY: clean_loader
clean_loader:
	make -C loader clean

.PHONY: service
service:
	make -C service

.PHONY: clean_service
clean_service:
	make -C service clean
