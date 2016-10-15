all: bdfe font misc

misc:
	gcc -o error_decoder error_decoder.c -g -O0

bdfe_dir:
	make -C bdfe

font: bdfe_dir
	./bdfe/bdfe -n -A ../external/tewi-font/tewi-medium-11.bdf > termfont.bin

clean:
	$(MAKE) -C bdfe clean
	rm -f termfont.bin key_char error_decoder
