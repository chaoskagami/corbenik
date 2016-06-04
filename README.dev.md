Corbenik
==============================

## This is the README intended for people looking at the code. If you're a user, read `README.user.md` instead.

This is (yet another) CFW for the 3DS. Unlike other CFWs, this was mostly written from scratch and for fun. I'm a control freak, and this carries quite a bit of my mindset being a LFS/Gentoo user.

Some parts are inherited from other CFWs - e.g. the firmware loading code in `src/firm` is mostly based on Cakes, and the patch bytecode is based on Luma3DS' implementation in C (though it isn't really derived from it.)

Out of the bunch of CFWs in existence, Corbenik is most similar to cakes of the bunch, in that it uses external patches. External patches are headered, can have dependencies, and consist of a lightweight and specialized bytecode/assembly which is intended solely for effiecient patching.

See `doc/bytecode.md`, `host/bytecode_asm.py`  and `patch/*` for more on this. The assembler is a bit crappy at the moment, and I *do* plan to improve it. However, it outputs the correct code and gets the job done.

## Rationale

I was initially going to make cakes dynamic, but I quickly realized a fatal flaw in any "patch" format: what you can do from a patch is limited to what the parser handles. With Cakes, converting to a dynamic method isn't terribly difficult, but what about the patches that have a 'find, then seek backwards until' type of logic? Cakes would need have another construct to decribe that, and at that point, the .cake format has become a kludge.

In my opinion, the best way to fix this was to externalize patches as programs - arm binaries or bytecode. The former didn't go so well (look back in the history of this repo. Fun times) and I ended up going with the latter.

I also had a number of mad science experiments which would be very hard to perform in the context of ReiNAND based firmwares, and Cakes wouldn't make it easy either due to its limited patch format.

## Comparison

If you want to know how Corbenik sizes up to other CFWs as of NOW - see `doc/features.md`. I don't intend to sugarcoat - Corbenik is under development and is incomplete. There will be no stable release until a number of common features are implemented, such as emunand.

However! It does have a few legs up on other CFWs, namely:
 * Injection of arbitrary ARM11 services, including svcBackdoor.
 * Bytecode patches? Bytecode patches.
   * Not only corbenik, but the loader replacement uses them too.
 * Loader can resize titles in memory and append code to segments. This isn't well tested, and it isn't enabled.
 * Pretty much every simple patch that Luma3DS has, and most every patch for loader.
   * All of the common bits aside from EmuNand, Reboot, and the exception vector hook, basically.
 * Loader is STILL smaller than Nintendo's by two units.
 * Loader is a little slower to boot than Cakes, and loader takes negligibly longer to load stuff. I may be able to optimize more, but at this point it is fast enough to not impact me.

Feedback is welcome, but don't report anything obvious in nightlies. Corbenik is my day-to-day CFW now, so I'll run into the same bugs as you.

For compilation instructions, see `doc/compiling.md`.

Unless otherwise noted, everything original in this repo can be used under the terms of the GNU GPLv3 or later. This includes situations where there's no copyright header within a source file. I get lazy with those; assume everything can be used under the GPLv3. No source files within this repo bear questionable licenses, I make sure when it is introduced.

## Quote of the Day

Welcome to "The World."

