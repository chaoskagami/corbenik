Corbenik
==============================

This is (yet another) CFW for the 3DS. Unlike other CFWs, this was mostly written from scratch for fun, and because I'm a control freak. Some parts are inherited from other CFW - near everything in src/firm is based on CakesFW, and the signature patch/firmprot/svcbackdoor fix are all based on Luma3DS while using the more correct CakesFW section code.

Conceptually, and in operation, this is most similar to mid-kid/CakesForeveryWan out of the bunch. That is, it uses external patches from the filesystem and is intended for developers and control freaks. Unlike cakes, patches are dynamically linked code for whatever processor it is on.

Yes; you read that correctly. I initially was going to use a dynamic cake-like patch. I quickly realized a fatal flaw in any "patch" format: what you can do from a patch is limited to what the parser handles. The best way to fix this was to make patches standalone relocatable binaries instead.

The binary ABI is not yet stabilized. Do not expect a patch to simply function a version later. For this very reason, the CFW version field in the patch header is ignored at the moment until the ABI has been finalized. I *may* simply rewrite it as an elf loader; I'm unsure yet as to what I'll do.

If you want to know how Corbenik sizes up to other CFWs - see `doc/features.md`.

For compilation instructions, see `doc/compiling.md`.

Unless otherwise noted, everything in this repo can be used under the terms of the GNU GPL, Version 3 or later (if ever) at your discretion. This includes situations where there's no copyright header within a source file.

Technically, all patches must be open source under a compatible license as well due to these linking restrictions. I will not be making the linking exception. Allowing proprietary patches to exist will only harm everyone in the homebrew community in long-term. Read: NTR.
