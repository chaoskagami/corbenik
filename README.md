![Corbenik](https://raw.github.com/wiki/chaoskagami/corbenik/corbenik-logo.png)

Corbenik is another CFW for the 3DS somewhat like cakes, but using bytecode-based patches. This allows for doing most of what it does, but taking the control level to its logical extreme.

It's mainly intended for developers; so if you don't know your way around the 3DS, chances are this isn't for you.

Not every feature has been implemented/tested yet, but at the moment it offers a rather comprehensive experience for people who know what they're doing.

Corbenik is licensed under the terms of the GPLv3. Please obey it. You should have received a copy either as part of the git repository, or as part of the tarball/zipfile you downloaded. If not, get a copy here: `http://www.gnu.org/licenses/gpl-3.0.txt`

Usage
-------------------------

If you are using a nightly build off of https://github.com/chaoskagami/corbenik - treat all paths starting in `/corbenik` as `/skeith` instead for these instructions.

Skip to `Installing` if you are installing this for the first time, otherwise follow `Updating` and then `Installing`.

Updating
-------------------------

When updating Corbenik, usually you should (at minimum) perform the following steps to SAFELY update:

 * Delete these files/directories in `/corbenik`:
   * `cache`
   * `config`
   * `boot.log`
   * `loader.log`
 * If you're upgrading from earlier versions:
   * 0.0.6 and older:
     * Delete everything
   * 0.0.7
     * Delete `patch`.
   * 0.0.8
     * Delete `patch`.
     * Delete `svc`. Backdoor was folded into `bits`.
 * Unzip all files in-place, overwriting if prompted.
 * Reconfigure settings on boot (follow `Setup` for defaults)


Installing
-------------------------

Copy the files to the root of your SD (and optionally, rename arm9loaderhax.bin and set it up with a bootloader.)

Without the FIRMs, it cannot boot up your system. You'll need to fetch the following at minimum, and put it at `/corbenik/firmware/native`:

Old 3DS (Native FIRM, 11.0):
 * firm: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000002/00000052
 * cetk: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000002/cetk

New 3DS (Native FIRM, 11.0):
 * firm: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000002/00000021
 * cetk: http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000002/cetk

Note that this is only a recommendation - you can supply near any valid firmware file for your console.

You'll need the firmkey for it as well unless you have decrypted your firmware, and it should be placed at `/corbenik/keys/native.key`. I can't tell you where to get it obviously, but there are a few ways to get valid firmware with/without a firmkey and not involving a download from a questionable place:

 * Decrypt the CETK using D9 and extract the firmkey using D9's scripts
 * Extract the encTitleKey using D9's scripts, and have D9 convert it
 * Decrypt the firmware with D9 directly
 * Boot another CFW, then reboot into Corbenik with a cetk for native at `/corbenik/keys/native.cetk`

You can also fetch the agb firm and twl firms to `/corbenik/firmware/agb` and `/corbenik/firmware/twl` respectively. If you don't have the firmkeys for these, you can fetch the cetk for each of them to `/corbenik/keys/agb.cetk` and `/corbenik/keys/twl.cetk`. Boot up the system, go to system settings, and it will extract the firm keys for them after rebooting.

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

On New3DS units, there's additional encryption on arm9loader which requires the 9.6 key to decrypt. It usually is named `Slot0x11Key96.bin`, and I also can't tell you where to find this, aside from "check Plailect's guide." Corbenik will attempt to read this from the root as well as `/corbenik/keys/11.key`. In a future version, keydb reading may be implemented, but no guarantees.

The folder `/corbenik/locale` is automatically generated language emulation files from 3dbrew for games that only specify one region and one language. Games which support more than one language are not generated, because there's no 'correct' language. You can remove this if the number of files unnerves you. It isn't required. You can also add new files if you have specific needs.

The folder `/contrib` contains additional patches you may add at your own discretion. These are not as well tested as official patches and don't generally affect core functionality. Documentation is usually found on the header of the source code for them (contrib/*.pco) in the git repo.

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

For starters, you'll want to go into options and enable `Loader Replacement` to get loader to run patches as well. Even if you don't plan to run any loader patches, this will at very least kill ASLR and anti-OoThax/anti-Ninjhax features in the official Nintendo loader.

If you're using 11.0 NATIVE_FIRM like I suggested, you may want to tick `svcBackdoor Fixup` to fix the broken svcBackdoor if you plan on using anything which requires it. This includes HBMenu, some Retroarch cores, etc. Your system will be more secure (as in against malicious code, not as in Nintendo) if you leave it off.

If you need to use an EmuNAND, you'll want to enable `EmuNAND` in options. If you've been using multiple EmuNANDs via Cakes or Luma you can also select the index while you're there with A to increase and X to decrease. This supports both Gateway-style (first sector at back) and standard copy NANDs (RedNAND)

You can enable `Autoboot` if you'd like, including `Silent mode` if you're using something like BootAnim9. As of 0.0.8, EmuNAND will be automatically disabled on AGB reboot, so you need not worry about your savedata with this. If you want to get back in the menu, hold the `R` button while booting.

If you plan to use TWL/AGB patches or have an O3DS, you should enable `Reboot Hook` in options.

You'll also want to go into `Patches` and enable the usual bits, which includes:
 * Signature Fix
 * FIRM Protect

You'll also want these patches, which are done by loader and therefore require it:
 * Block Cart Updates
 * Block eShop Updates
 * Block NIM updates
 * Region free HOME
 * RO signature fix

If you're using the reboot hook, you might want these:
 * AGB Signature fix
 * AGB Bootscreen
   * Will stop games with corrupted Nintendo logos from running. Disable for ROM hacks if this occurs.
 * TWL Patches

If you're on 11.0, you also want these:
 * Title Downgrade Fix (Only enable with 11.0 firmware.)

If you're deliberately still running older firmware on your NAND, you'll
want these:
 * Fake Friends Version

If you region changed your console and replaced SecureInfo_A, you want:
 * SecureInfo_A Signature Fix

Optional, but recommended patches are:
 * Settings Version String
 * ErrDisp devmode

And these YOU SHOULD NOT ENABLE unless you have specialized needs:
 * Developer UNITINFO (Pretends to be a developer console/Panda)
 * ARM11 XN Disable   (Grants +X maps by default to kernel)
 * Force TestMenu     (Boots into TestMenu rather than HOME - requires TestMenu to be installed)

Before booting, you should select 'Save Configuration' from the menu.

Customization
-------------------------

Extra patches may be supplied in the `/corbenik/contrib` folder. Copy any of these patches to `patch` if you wish to use them.

You can copy some 90° rotated BGR8 pixel data sized to the screen (essentially, a menuhax splash) and it will be used as backgrounds for menus. Put them at:
 * Top: `/corbenik/bits/top.bin`
 * Bottom: `/corbenik/bits/bottom.bin`

Reporting issues
-------------------------

If you think you've found a bug, please do the following first, to save me some time:

 * Check if a recently enabled patch is the cause of the issue. If so, you should include this in the report.
 * Enable `Logging` and `Verbose` in `Options` then `Save Configuration` and retrieve the files `/corbenik/boot.log` and `/corbenik/loader.log` if they exist. I will want them. Do not report bugs without them, unless they are not created with the above enabled.
 * Please at least try to reproduce the bug from a clean installation.

Contributions
-------------------------

If you have a feature or bugfix, PR or hit me on freenode/#Cakey. However, please note the following conditions:

 * Do NOT base any code on Nintendo's SDK. Additionally, if you are under NDA, do not even bother to PR. I cannot accept tainted code. This is for my own legal safety (and sanity)
 * Please attempt to obey coding standards. The .clang-format is a loose guide to this. I'll tell you if I need reformatting.
 * Please ensure your changes are functional and don't break consoles, O3DS or N3DS. Do not assume anything about the environment.

Credits
-------------------------

@yifanlu
 * For the absolutely insane and wonderful idea to use bytecode, as well as the open source loader replacement. https://github.com/yifanlu/3ds_injector

@mid-kid
 * General inspiration from Cakes, FIRM decryption code, reboot assembly code, some code for text display. https://github.com/mid-kid/CakesForeveryWan

@Wolfvak
 * Code segment dumping + loading

@AuroraWright
 * RE work, patches, EmuNAND, and Reboot/Firmlaunch C code. https://github.com/AuroraWright/Luma3ds

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
 * RE work, screen init, and also the A9LH version I personally use.

@smealum
 * Pioneering the open source hax frontier. (No actual code of his is in this repo itself, but ctrulib is basically essential for everything.)

@TiniVi
 * RE work, screen deinit.

Temptress Cerise (GBATemp) 
 * A LOT of testing which was really helpful.

Crystal the Glaceon (GBATemp)
 * A LOT of testing which was really helpful.

Everyone on #Cakey for being generally cool people.

CyberConnect2
 * Because the name originates from .hack, which you should go play.

(If I've forgotten anyone, please let me know!)
