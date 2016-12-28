all: bdfe ips2pco font misc

misc:
	gcc -o error_decoder error_decoder.c -g -O0

ips2pco:
	$(MAKE) -C ips2pco

bdfe_dir:
	$(MAKE) -C bdfe

font: bdfe_dir
	./bdfe/bdfe -n -A ../external/tewi-font/tewi-medium-11.bdf > termfont.bin

clean:
	$(MAKE) -C ips2pco clean
	$(MAKE) -C bdfe clean
	rm -f termfont.bin key_char error_decoder
