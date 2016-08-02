![Corbenik](https://raw.github.com/wiki/chaoskagami/corbenik/corbenik-logo.png)

Corbenik is another CFW for the 3DS somewhat like cakes, but using bytecode-based patches. This allows for doing most of what it does, but taking the control level to its logical extreme.

It's mainly intended for developers; so if you don't know your way around the 3DS, chances are this isn't for you.

Not every feature has been implemented/tested yet, but at the moment it offers a rather comprehensive experience for people who know what they're doing.

Corbenik is licensed under the terms of the GPLv3. Please obey it. You should have received a copy either as part of the git repository, or as part of the tarball/zipfile you downloaded. If not, get a copy here: `http://www.gnu.org/licenses/gpl-3.0.txt`

This branch happens to be a terrible experiment. Don't try it unless you know what it is; e.g. this is not corbenik, but a standalone chainloader using its menu code (yay flexibility)

Usage
-------------------------

If you're compiling this from source code, see at the bottom the `Building` section.

This is a heavily stripped down version of corbenik which is JUST a chainloader.

Installing
-------------------------

Copy the files to the root of your SD. This is a chainloader, so it's most useful as /arm9loaderhax.bin

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

Customization
-------------------------

You can copy some 90° rotated BGR8 pixel data sized to the screen (essentially, a menuhax splash) and it will be used as backgrounds for menus. Put them at:
 * Top: `/corbenik/share/top.bin`
 * Bottom: `/corbenik/share/bottom.bin`

The font is also customizable (`/corbenik/share/termfont.bin`) - read the github wiki for details.

Building
-------------------------

First; make sure you have submodules properly checked out. If you do not, the build will fail in odd and unintelligible ways.

You will need at minimum the following:

 * devkitARM
 * Host gcc (as in a native system compiler)
 * Autotools (as in, automake/autoconf - mandatory)
 * libtool (expect weird link errors if this is missing)

Briefly; the following commands are enough to build, assuming devkitarm is in your `PATH`:

```
./autogen.sh
./configure --host=arm-none-eabi
```

Output will be produced in a directory named `out` after a successful build. This produces a build largely identical to normal releases from master.

There's additional options one can provide - see `./configure --help` for information on these.

Building corbenik on Windows never has and never will be supported. Your mileage may vary.

Contributions
-------------------------

If you have a feature or bugfix, PR or hit me on freenode/#Cakey. However, please note the following conditions:

 * Do NOT base any code on Nintendo's SDK. Additionally, if you are under NDA, do not even bother to PR. I cannot accept tainted code. This is for my own legal safety (and sanity)
 * Please attempt to obey coding standards. The .clang-format is a loose guide to this. I'll tell you if I need reformatting.
 * Please ensure your changes are functional and don't break consoles, O3DS or N3DS. Do not assume anything about the environment you are running under.

Credits
-------------------------

@yifanlu
 * For the absolutely insane and wonderful idea to use bytecode, as well as the open source loader replacement. https://github.com/yifanlu/3ds_injector

@mid-kid
 * General inspiration from Cakes, FIRM decryption code, reboot assembly code, some code for text display. https://github.com/mid-kid/CakesForeveryWan

@Wolfvak
 * Code segment dumping + loading, ideas, and a lot of miscellaneous code.

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

@gemarcano
 * For general help and libctr9 (which is a submodule here.) Made life much easier.

Temptress Cerise (GBATemp) 
 * A LOT of testing which was really helpful.

Crystal the Glaceon (GBATemp)
 * A LOT of testing which was really helpful.

Everyone on #Cakey for being generally cool people.

CyberConnect2
 * Because the name originates from .hack, which you should go play.

(If I've forgotten anyone, please let me know!)
