While most everything in this repo is original, some parts are derived from existing software, and I make the best effort to document what is derivative in the interest of giving proper credits:

src/firm        - Firmware decryptor originally from CakesFW (http://github.com/mid-kid/CakesForeveryWan)
src/fatfs       - FatFS - http://elm-chan.org/fsw/ff/00index_e.html . This version originates from Decrypt9's repo. The sdmmc code is from Normantt.
src/std         - Work was originally based on memfuncs.c and memory.c from Cakes and ReiNand,
                  but contains near none of the original code now. Not really fair to say it's derived from it, but it WAS before a near 100% rewrite.
external/loader - Originally based on Luma3DS' injector, which is in turn based on Yifan Lu's injector. Injector now has a very long history. ;P
vco/            - Most of the external patches are based on patcher code from Luma3DS. Obviously some changes were made.
