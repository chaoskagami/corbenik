While most everything in this repo is original, some parts are derived from existing software, and I make the best effort to document what is derivative in the interest of giving proper credits:

src/firm        - Firmware decryptor originally from CakesFW (http://github.com/mid-kid/CakesForeveryWan). Rather hacked up.
src/fatfs       - FatFS - http://elm-chan.org/fsw/ff/00index_e.html . This version originates from Decrypt9's repo. The sdmmc code is from Normantt.
src/std         - Work was originally based on memfuncs.c and memory.c from Cakes and ReiNand,
                  but contains near none of the original code now. Not really fair to say it's derived from it, but it WAS before a near 100% rewrite.
external/loader - Originally based on Luma3DS' injector, which is in turn based on Yifan Lu's injector. Injector now has a very long history. ;P
patch/          - Most of the external patches were based on the C code in Luma3DS. Obviously, they share no actual code.
src/i2c.*       - I2C handling code from delebile's arm9loaderhax repo. I cleaned it up somewhat.
src/start.s     - Another inherited piece of code. I obtained this from Decrypt9's repo, but this is in use elsewhere.
