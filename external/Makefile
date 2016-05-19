.PHONY: all
all: loader service
	mkdir -p ../out/corbenik/lib/module
	mkdir -p ../out/corbenik/lib/service
	cp loader/loader.cxi ../out/corbenik/lib/module/loader.cxi
	cp service/7b.bin ../out/corbenik/lib/service/7b.bin

.PHONY: clean
clean: clean_loader clean_service
	rm -rf ../out/corbenik/bin

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
