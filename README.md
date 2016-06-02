Corbenik
==============================

This is (yet another) CFW for the 3DS. Unlike other CFWs, this was mostly written from scratch and for fun. I'm a control freak, and this carries quite a bit of my mindset being a LFS/Gentoo user.

Some parts are inherited from other CFWs - e.g. the firmware loading code in src/firm is mostly based on Cakes, and the patch bytecode is based on Luma3DS' implementation in C (though, it isn't really derived from it)

Out of the bunch of CFWs in existence, Corbenik is most similar to cakes of the bunch, in that it uses external patches. External patches are headered, can have dependencies, and consist of a lightweight and specialized bytecode/assembly which is intended for patching. See `doc/bytecode.md`, `host/bytecode_asm.py`  and `patch/*` for more on this. The assembler is a bit crappy at the moment, and I *do* plan to improve it. However, it outputs the correct code which gets the job done.
## Rationale

I was initially going to make dynamic cakes, but I quickly realized a fatal flaw in any "patch" format: what you can do from a patch is limited to what the parser handles. This exact problem is why sempatches are used sometimes instead of classic diffs on the LKML, and it isn't a problem that will be solved without code. In my opinion, the best way to fix it is to simply externalize patches as programs. I also had a number of mad science experiments which would be very hard to perform in the context of ReiNAND based firmwares, and Cakes wouldn't make it easy either.

## Comparison

If you want to know how Corbenik sizes up to other CFWs as of NOW - see `doc/features.md`. I don't intend to sugarcoat - Corbenik is under development and is incomplete. There will be no stable release until a number of common features are implemented, such as emunand.

That said, feedback is welcome, but don't report anything obvious. Chances are I know, since Corbenik is my day-to-day CFW now, and I'll run into the same bugs as you.

For compilation instructions, see `doc/compiling.md`.

Unless otherwise noted, everything original in this repo can be used under the terms of the GNU GPLv3 or later. This includes situations where there's no copyright header within a source file. I get lazy with those; assume everything can be used under the GPL, or files were from software licensed GPLv2 or later (and thus are upgraded.) No source files within this repo bear questionable licenses.

## Quote of the Day

Welcome to "The World."
