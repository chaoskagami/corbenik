Corbenik
==============================

This is (yet another) CFW for the 3DS. Unlike other CFWs, this was mostly written from scratch and for fun - because I'm a control freak, and this carries quite a bit of OSDev and RE with it.

Some parts are inherited from other CFW - for example, the firmware loading code in src/firm is based on Cakes, and sigpatch/firmprot/svcbackdoor were loosely based on Luma3DS.

Eventually in operation, this will be most similar to Cakes out of the bunch. That is, it will use external patches from the filesystem and is intended for developers and control freaks. Unlike cakes, patches will be dynamically loaded binaries for arm9/arm11 which will be relocated against internal functions, allowing relatively tiny patches. This is not stable, and does not work yet.

Rationale: I was initially going to make dynamic cakes, but I quickly realized a fatal flaw in any "patch" format: what you can do from a patch is limited to what the parser handles. The best way to fix is to simply externalize patches; e.g. even yifan lu suggested even that loader should use a bytecode VM.

If you want to know how Corbenik sizes up to other CFWs as of NOW - see `doc/features.md`. If you want to see how it will hopefully size up once I'm finished with all the code see `doc/intended-comparison.md`

For compilation instructions, see `doc/compiling.md`.

Unless otherwise noted, everything in this repo can be used under the terms of the GNU GPL, Version 3 or later (if ever) at your discretion. This includes situations where there's no copyright header within a source file. I get lazy with those; assume everything can be used under the GPL, or files were from GPLv2 or later (and thus are upgraded.)
QoTD: "And I will tell you again...Welcome to 'The World.'"
