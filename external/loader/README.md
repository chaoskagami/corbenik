3DS Loader Replacement
======================

This is an open source implementation of 3DS `loader` system module--with 
additional features. The current aim of the project is to provide a nice 
entry point for patching 3DS modules.

## Roadmap
Right now, this can serve as an open-source replacement for the built in loader. 

There is additional support for patching any executable after it's loaded but 
before it starts. For example, you can patch `menu` to skip region checks and 
have region free game launching directly from the home menu.

There is also support for SDMC reading (not found in original loader implementation)
which  means that patches can be loaded from the SD card. Ultimately, there would be 
a patch system that supports easy loading of patches from the SD card.

## Changes I've made

A lot of the 'disassembled' looking code has been rewritten for readability, and
many cruft-ish artifacts of it have been cleaned up. Some wrapper functions
have also been rewritten out of the code, and anything nigh-unreadable and
non-rewritable has been documented when I managed to figure out what it does.

At the moment there is also experimental support for resizing segments before
loading the executable. This means that segments can be appended to. Notably,
this allows tacking on code to the end.

The text, data, and ro segments are handled separately to streamline the new
segment resizing and limit search space to speed things up a tad. Why search
text, data and ro when you know it is in text?

## Imported changes from other 3ds_injector forks

I updated it to the latest git ctrulib (in which FS_Archive is a u64,
not a struct.) @TuxSH did the work before me, it required manual conflict merges
but he provided the information needed to fix it up for the most part. ;P

## Build
You need a working 3DS build environment with a fairly recent copy of devkitARM, 
ctrulib, and makerom.

Currently, there is no support for FIRM building in toolchain; whatever method
you use to inject this is up to you, but the software must have support for
resizing the sysmodule segment, or you must make sure the module size matches
the original.
