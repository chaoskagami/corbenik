![Corbenik](https://raw.github.com/wiki/chaoskagami/corbenik/corbenik-logo.png)

[![Build Status](https://j.chaos.moe/job/Corbenik/badge/icon)](https://j.chaos.moe/job/Corbenik/)

Corbenik is another CFW for the 3DS somewhat like cakes, but using bytecode-based patches. This allows for doing most of what it does, but taking the control level to its logical extreme.

It's mainly intended for developers; so if you don't know your way around the 3DS, chances are this isn't for you.

Not every feature has been implemented/tested yet, but at the moment it offers a rather comprehensive experience for people who know what they're doing.

Corbenik is licensed under the terms of the GPLv3. Please obey it. You should have received a copy either as part of the git repository, or as part of the tarball/zipfile you downloaded. If not, get a copy here: `http://www.gnu.org/licenses/gpl-3.0.txt`

Usage
-------------------------

Automated builds are located here, with an alternate path of `/skeith` instead of `/corbenik`: https://j.chaos.moe/job/Corbenik

You will likely want to compile corbenik from source code to get the latest changes (if any.) Therefore, you will want to follow the `Building` section.

Afterwards, you should skip to `Installing` if you are installing this for the first time, otherwise run `git log` and search for any `BREAKING` within the log to figure out what incompatibilities the new commit has (if any.)

Building
-------------------------

First; make sure you have submodules properly checked out. If you do not, the build will fail in odd and unintelligible ways. You should either run `git clone --recursive` to check out the code, or run `git submodule update --init --recursive` if you did not clone recursively.

You will need at minimum the following:

 * devkitARM (must be in your PATH)
 * Host gcc (as in a native system compiler)
 * Python 2.7 (for patches)
 * Autotools (as in, automake/autoconf - mandatory)
 * libtool (expect weird link errors if this is missing)

Briefly; the following commands are enough to build:

```
./autogen.sh
./configure --host=arm-none-eabi
make -j4
```

In other words, the fairly standard autotools dance.

Output will be produced in a directory named `out` after a successful build. This produces a build largely identical to normal releases from master. You can create a release.zip and sha512sums by running `./host/release.sh` from the root of the project.

There's additional options one can provide - see `./configure --help` for information on these.

Building corbenik on Windows never has and never will be officially supported. Your mileage may vary, but you'll likely have issues.

Installing
-------------------------

Copy the files to the root of your SD (and optionally, rename arm9loaderhax.bin and set it up with a bootloader.)

Without the FIRMs, it cannot boot up your system. You'll need to fetch the following at minimum, putting the firm at `/corbenik/lib/firmware/native` and cetk at `/corbenik/lib/firmware/native.cetk`. If you are using a POSIX shell and have the wget command available, you can run the shell scripts provided to automatically fetch for your console.

Otherwise, manually fetching firmware should be done from the following URLs:

Old 3DS (Native FIRM, 11.4):
 * firm: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000002/0000005e
 * cetk: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000002/cetk

New 3DS (Native FIRM, 11.4):
 * firm: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000002/0000002f
 * cetk: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000002/cetk

This is only a recommendation - you can supply near any valid firmware file for your console (it has only been tested extensively for >=9.2, however.)

You should also fetch the agb firm and twl firms to `/corbenik/lib/firmware/agb` and `/corbenik/lib/firmware/twl` respectively. You can fetch the cetk for each of them to `/corbenik/lib/firmware/agb.cetk` and `/corbenik/lib/firmware/twl.cetk`. If these are missing, it will not be possible to run AGB and TWL patched, though they will still run (and without patches, this limits you to properly signed content.)

Old 3DS TWL_FIRM (Firmware for DS/DSi games):
 * cetk: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000102/cetk
 * firm: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000102/00000016

New 3DS TWL_FIRM (Firmware for DS/DSi games):
 * cetk: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000102/cetk
 * firm: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000102/00000000

Old 3DS AGB_FIRM (Firmware for GBA games):
 * cetk: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000202/cetk
 * firm: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000202/0000000B

New 3DS AGB_FIRM (Firmware for GBA games):
 * cetk: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000202/cetk
 * firm: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000202/00000000

**IMPORTANT** - On New3DS units, there's additional encryption on arm9loader which requires the 9.6 key to decrypt. This key also happens to be trashed by arm9loaderhax, so you'll need to acquire it elsewhere. It usually is named as `Slot0x11Key96.bin`. I can't tell you where to find this. Corbenik will attempt to read this from the root as `slot0x11key96.bin`. In a future version, keydb reading may be implemented, but no guarantees.

The folder `/corbenik/share/locale/emu` is for language emulation files. You can retrieve a set of files from 3dsdb for games that only specify one region and one language by running the `generate_localeemu.sh` shell script. Games which support more than one language are not generated, because there's no 'correct' language.

The folder `/corbenik/bin` contains additional patches you may enable at your own discretion. These are not as well tested as official patches and don't generally affect core functionality. Documentation is usually found on the header of the source code for them (contrib/*.pco) in the git repo. Patches in `/corbenik/bin` will be listed at the bottom of the list of patches, after the patches within `/corbenik/sbin` which are mostly core functionality.

Setup
-------------------------

Unless otherwise noted, menu controls are always shown at the top, but for reference:
 * A         -> Enter/Select/Enable/Increase
 * X         -> Decrease
 * B         -> Back/Exit/Boot
 * Up        -> Up one
 * Down      -> Down one
 * Left      -> Up five
 * Right     -> Down five
 * Select    -> Help (on any selectable option)
 * L+R+Start -> Screenshot (Menu ONLY.)

For starters, you'll want to go into Configuration->Options and enable `System Module Inject` to get loader to run patches as well. Even if you don't plan to run any loader patches, this will at very least kill ASLR and anti-OoThax/anti-Ninjhax features in the official Nintendo loader. Corbenik's loader is a bit weightier than nintendo's (approximately 10 mediaunits as of last check) but contains extra functionality.

If you're using 11.2 NATIVE_FIRM like I suggested, you may want to tick `svcBackdoor Fixup` to fix the broken svcBackdoor if you plan on using anything which requires it. This includes HBMenu, some Retroarch cores, etc. Your system will be more secure (as in against malicious code, not as in Nintendo) if you leave it off.

If you need to use an EmuNAND, you'll want to enable `EmuNAND` in options. If you've been using multiple EmuNANDs via Cakes or Luma you can also select the index while you're there with A to increase and X to decrease. This supports both Gateway-style (first sector at back) and standard copy NANDs (RedNAND)

You can enable `Autoboot` if you'd like, including `Silent mode` if you're using something like BootAnim9. As of 0.0.8, EmuNAND will be automatically disabled on AGB reboot, so you need not worry about your savedata with this. If you want to get back in the menu, hold the `R` button while booting.

If you plan to use TWL/AGB patches or have an O3DS, you should enable `Reboot Hook` in Options.

You'll also want to go into `Patches` and enable the usual bits, which includes:
 * Signature Fix
 * FIRM Protection

You'll also want these patches, which are done by loader and therefore require it:
 * Block Cart Update / Cart RF (Loader)
 * Block eShop Updates (Loader)
 * Block NIM Updates (Loader) [1]
 * Region Free HOME (Loader)
 * RO Signature Fix (Loader

If you're using the reboot hook, you might want these:
 * AGB Signature Fix
 * AGB Bootscreen [2]
 * TWL Patches - Select either one, the correct one will be applied

If you're on 11.0 or higher and using the respective FIRM, you also want these:
 * Title Downgrade Fix (11.0+ NFIRM)

If you're deliberately still running older firmware on your NAND, you'll want these:
 * Fake Friends Version (Loader)

If you region changed your console and replaced SecureInfo_A, you want:
 * SecureInfo_A Signature Fix (Loader)

Optional, but recommended patches are:
 * Settings Version String (Loader)
 * Verbose ErrDisp (Loader)

And these YOU SHOULD NOT ENABLE unless you have specialized needs:
 * Developer UNITINFO [3]
 * ARM11 XN Disable
 * Force TestMenu [4]

[1] - This is known to cause some issues with regards to software updates via HOME as of recent firmwares. Luma has disabled this by default. If you get errors repeatedly when attempting updates, this may be the cause.

[2] - This will stop games with a corrupted Nintendo logo from running. Disable for ROM hacks if this occurs. There's nothing I can do to resolve this, since the GBA bios is on the SoC. Please take this up with the author of your rom(hack).

[3] - This will disable usage of most internet functionality on the 3DS. Verbose ErrDisp is preferred.

[4] - Boots into TestMenu rather than HOME. This requires a TestMenu implementation to be installed, either factory, re-encrypted developer, or some experimental applet with that titleID.

Customization
-------------------------

You can copy some 90° rotated BGR8 pixel data sized to the screen (essentially, a menuhax splash) and it will be used as backgrounds for menus. Put them at:
 * Top: `/corbenik/share/top.bin`
 * Bottom: `/corbenik/share/bottom.bin`

The font is also customizable (`/corbenik/share/termfont.bin`) - read the github wiki for details. The bundled font is the very nice Tewi font.

Optionals
-------------------------

Corbenik has a chainloader that can be used to load other payloads. Simply place them in `/corbenik/boot` and they will be automatically picked up and shown in the `Chainloader` menu for use.

The chainloader only supports .bin payloads. .dat payloads are unsupported, since they are intended for execution from an ARM11 environment. Supporting them would require far too much effort for far too little gain.

Reporting issues
-------------------------

If you think you've found a bug, please do the following first, to save me some time:
 * Check if a recently enabled patch is the cause of the issue. If so, you should include this in the report.
 * Enable `Logging` and `Verbose` in `Options` then `Save Configuration` and retrieve the files `/corbenik/var/log/boot.log` and `/corbenik/var/log/loader.log` if they exist. I will want them. Do not report bugs without them, unless they are not created with the above enabled.
 * Please at least try to reproduce the bug from a clean installation.
 * Try to reproduce the problem from another CFW like luma or cakes, to determine if the cause is truly corbenik.

If you are requesting a feature, please do the following first, before opening an issue:
 * Determine whether you can implement and PR the feature yourself.
 * Determine whether what you are asking is reasonable.
 * Determine the relative difficulty of what you are asking.
 * Clarify what exactly you would like to see implemented.

Contributions
-------------------------

If you have a feature or bugfix, PR or hit me on freenode/#Cakey. However, please note the following conditions:
 * Do NOT base any code on Nintendo's SDK. Additionally, if you are under NDA, do not even bother to PR. I cannot accept tainted code. This is for my own legal safety (and sanity)
 * Please attempt to obey coding standards. The .clang-format is a loose guide to this. I'll tell you if I need reformatting.
 * Please ensure your changes are functional and don't break consoles, O3DS or N3DS. Do not assume anything about the environment you are running under unless it is a safe assumption (and near none are)

Credits
-------------------------

In no specific order:

@yifanlu
 * For the absolutely insane and wonderful idea to use bytecode, as well as the open source loader replacement. https://github.com/yifanlu/3ds_injector

@mid-kid
 * General inspiration from Cakes, FIRM decryption code, reboot assembly code, some code for text display. https://github.com/mid-kid/CakesForeveryWan

@Wolfvak
 * Code segment dumping + loading, ideas, and a lot of miscellaneous code.

@AuroraWright
 * RE work, patches, EmuNAND, SVC replacement/injection, and Reboot/Firmlaunch C code. https://github.com/AuroraWright/Luma3ds

@Reisyukaku
 * For the 'Force TestMenu' patch, and RE work. Also coded ReiNand, which some parts of Luma are derived from and thus some parts of this.

@d0k3
 * start.s, recursive directory listing, RE work. https://github.com/d0k3/GodMode9

@TuxSH
 * RE work, code, and patches. Some code in loader is based on his extensions to it.

@Steveice10
 * RE work, patch offsets.

@dark-samus
 * RE work, screen init.

@b1l1s
 * RE work, screen init.

@Normmatt
 * RE work, screen init, and sdmmc.c/h

@delebile
 * RE work, screen init, 2.x firmprot, and also the A9LH version I personally use.

@smealum
 * Pioneering the open source hax frontier. (No actual code of his is in this repo itself, but ctrulib is basically essential for everything.)

@TiniVi
 * RE work, screen deinit.

@gemarcano
 * For general help and libctr9 (which is a submodule here.) Made life much easier.

Temptress Cerise (GBATemp) 
 * A LOT of testing which was really helpful.

Lilith Valentine(GBATemp)
 * A LOT of testing which was really helpful.

Everyone on #Cakey for being generally cool people.

CyberConnect2
 * Because the name originates from .hack, which you should go play.

(If I've forgotten anyone, please let me know!)
