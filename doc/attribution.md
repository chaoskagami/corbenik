While most everything in this repo is original, some parts are derived from existing software, and I make the best effort to document what is derivative in the interest of giving proper credits:

src/firm      - Firmware decryptor originally from CakesFW (http://github.com/mid-kid/CakesForeveryWan)
src/fatfs     - FatFS - http://elm-chan.org/fsw/ff/00index_e.html . This version originates from Decrypt9's repo.
src/std       - Work was originally based on memfuncs.c and memory.c from Cakes and ReiNand, but contains near none of the original code now.
src/patcher.c - Contains some bits from Luma3DS, but they've been adjusted for the cakes structures (src/firm) rather than crazy-ass pointer math.

