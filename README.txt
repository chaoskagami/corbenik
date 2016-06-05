About
-------------------------

Corbenik is another CFW for the 3DS somewhat like cakes, but using
bytecode-based patches. This allows for much greater flexibility.

It's mainly intended for developers; so if you don't know your way
around the 3DS, chances are this isn't for you.

Not every feature has been implemented/tested yet, but at the moment
it offers a rather comprehensive SysNAND experience for N3DS users.

Corbenik is licensed under the terms of the GPLv3. Please obey it.
You should have recieved a copy either as part of the git repository,
or as part of the tarball/zipfile you downloaded. If not, get a copy
here: `http://www.gnu.org/licenses/gpl-3.0.txt`

Installing
-------------------------

Copy the files to the root of your SD (and optionally, rename
arm9loaderhax.bin and set it up with a bootloader.)

Without the FIRMs, it cannot boot up your system. You'll need to
fetch the following at minimum, and put it at `/corbenik/firmware/native`:

Old 3DS (Native FIRM, 11.0):
  http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013800000002/00000052
New 3DS (Native FIRM, 11.0):
  http://nus.cdn.c.shop.nintendowifi.net/ccs/download/0004013820000002/00000021

You'll need the firmkey for it as well unless you have decrypted your firmware,
and it should be placed at `/corbenik/keys/native.key`. I can't tell you how to
get it obviously, but a good place to start may be an older version of Plailect's
guide when it still had a section on Cakes.

On New3DS units, there's additional crypto on arm9loader which requires the 9.6
key to decrypt. It usually is named ``Slot0x11Key96.bin`, and I also can't tell
you where to find this, aside from "check Plailect's guide." Corbenik will
attempt to read this from the root as well as `/corbenik/keys/11.key`.

The folder `corbenik/locale` is automatically generated language emulation
files from 3dbrew for games that only specify one region and one language.
Games which support more than one language are not generated, because there's
no 'correct' language. You can remove this if the number of files unnerves
you. It isn't required.

Setup
-------------------------

For starters, you'll want to go into options and enable `System Modules` to get
loader to run patches as well.

If you're using 11.0 NATIVE_FIRM like I suggested, you'll want to tick
`Service Replacement` to fix the broken svcBackdoor. Without this, Retroarch
won't work - and other applications that do JIT also won't work.

While you're there, you can enable `Autoboot` if you'd like, including
`silent mode` if you're using something like BootAnim9.

You'll also want to go into `Patches` and enable the usual bits, which includes:

 * Signature Fix
 * FIRM Protect

You'll also want these patches, which are done by loader and therefore require it:

 * Block Cart Updates
 * Block eShop Updates
 * Block NIM updates
 * Region free HOME
 * RO signature fix

If you're on 11.0, you also want these:

 * Title Downgrade Fix (Only enable with 11.0 firmware - others will fail)

If you're deliberately still running older firmware on your NAND, you'll
want these:

 * Fake Friends Version

If you region changed your console and replaced SecureInfo_A, you want:

 * SecureInfo_A Signature Fix

Optional, but recommended patches are:

 * MSET Version
 * ErrDisp devmode

And these YOU SHOULD NOT ENABLE unless you have specialized needs:

 * Developer UNITINFO
 * ARM11 XN Disable
 * Force TestMenu

Credits
-------------------------

This software contains code from multiple other open source projects, including:

 @mid-kid/CakesForeveryWan - FIRM Decryptor

 @AuroraWright/Luma3DS     - Used as reference for patcher bytecode (and in earlier
                             versions, the patch code itself) The version of loader
                             in use also originated from the code here.

 @d0k3/GodMode9            - FatFS version originates from here, start.s too.
                             Additionally, recursive directory removal and listing
                             patches is partially based on GodMode9.

 @yifanlu                  - Loader.

The complete "Thanks cool people" list:

 @yifanlu        For the absolutely insane and wonderful idea to use bytecode,
                 as well as the loader replacement.

 @mid-kid        General inspiration from Cakes.

 @Wolfvak        Go use BootAnim9.

 @AuroraWright   Luma made chunks of this possible.

 @Reisyukaku     You can thank him for the 'Force TestMenu' patch.

 @d0k3           Mostly code. Oh, and Decrypt9WIP/GodMode9 too.

 @smealum        HANS code, and pioneering the open source hax frontier.

 @TuxSH          Lots of RE work and important commits to Luma that I rely on.

 CyberConnect2   Because the name originates from .hack, which you should go play.
