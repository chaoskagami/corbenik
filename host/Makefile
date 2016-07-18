all: bdfe font

bdfe_dir:
	make -C bdfe

font: bdfe_dir
	./bdfe/bdfe -n -A ../external/tewi-font/tewi-medium-11.bdf > termfont.bin

clean:
	make -C bdfe clean
	rm -f termfont.bin
