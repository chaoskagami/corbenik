Corbenik
==============================

This is (yet another) CFW for the 3DS. Unlike other CFWs, this was mostly written from scratch for fun, and because I'm a control freak. Some parts are inherited from other CFW - near everything in src/firm is based on CakesFW, and the signature patch/firmprot/svcbackdoor fix are all based on Luma3DS while using the more correct CakesFW section code versus pointer math magic.

Eventually in operation, this will be most similar to mid-kid/CakesForeveryWan out of the bunch. That is, it will use external patches from the filesystem and is intended for developers and control freaks. Unlike cakes, patches will be dynamically loaded code for arm9/arm11 which will be relocated against internal functions, allowing relatively tiny patches. This is not stable yet, and does not work yet.

I was initially going to use a dynamic cake-like patch, but I quickly realized a fatal flaw in any "patch" format: what you can do from a patch is limited to what the parser handles. The best way to fix is to simply externalize patches; as yifan lu suggested even that loader should use a bytecode VM.

If you want to know how Corbenik sizes up to other CFWs as of NOW - see `doc/features.md`. If you want to see how it will hopefully size up once I'm finished with all the code see `doc/intended-comparison.md`

For compilation instructions, see `doc/compiling.md`.

Unless otherwise noted, everything in this repo can be used under the terms of the GNU GPL, Version 3 or later (if ever) at your discretion. This includes situations where there's no copyright header within a source file. I get lazy with those; assume everything can be used under the GPL.

Technically, all patches must be open source under a compatible license as well due to these linking restrictions. I will not be making the linking exception. Allowing proprietary patches to exist will only harm everyone in the homebrew community in long-term. Read: NTR.
