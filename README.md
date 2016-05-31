Corbenik
==============================

## Oh god yet another rebrand of--

No. Definitely not.

This is (yet another) CFW for the 3DS. Unlike other CFWs, this was mostly written from scratch and for fun. I'm a control freak, and this carries quite a bit of my mindset, and some OSDev and self-done RE with it.

Some parts are inherited from other CFWs - e.g. the firmware loading code in src/firm is mostly based on Cakes, and sigpatch/firmprot/svcbackdoor were loosely based on Luma3DS's patches.

The *plan* at least is to be most similar to Cakes out of the bunch. That is, it will use external patches from the SD card's filesystem.

Unlike cakes, patches will be some form of program, be it bytecode, a scripting language or dynamically loaded binaries. This is not yet implemented due to technical problems, but IS on the roadmap to get done as soon as I figure out *why* it isn't working. This code is located in a branch offline, due to the fact that it doesn't work.

## Rationale

I was initially going to make dynamic cakes, but I quickly realized a fatal flaw in any "patch" format: what you can do from a patch is limited to what the parser handles. This exact problem is why sempatches are used sometimes instead of classic diffs on the LKML, and it isn't a problem that will be solved without code. In my opinion, the best way to fix it is to simply externalize patches.

## Comparison

If you want to know how Corbenik sizes up to other CFWs as of NOW - see `doc/features.md`. I don't intend to sugarcoat - Corbenik is under development and is incomplete. There will be no stable release until a number of common features are implemented, and patches have been externalized.

That said, feedback is welcome, but don't report anything obvious. Chances are I know, since Corbenik is my day-to-day CFW now.

For compilation instructions, see `doc/compiling.md`.

Unless otherwise noted, everything original in this repo can be used under the terms of the GNU GPLv2 or later. This includes situations where there's no copyright header within a source file. I get lazy with those; assume everything can be used under the GPL, or files were from software licensed GPLv2 or later (and thus are upgraded.) No source files within this repo bear questionable licenses.

## Quote of the Day

Welcome to "The World."
