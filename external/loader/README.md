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

At the moment this copy has support for resizing segments before loading the
executable. This means that segments can be appended to. Notably, this allows
ADDING code, not simply changing it.

The text, data, and ro segments are handled separately to streamline segment
expansion and limit search space.

A lot of the 'disassembled' looking code has been rewritten for readability, and
many cruft-ish artifacts of it have been cleaned up. Some wrapper functions
have also been rewritten out of the code, and anything nigh-unreadable has
been documented when I figure out what it does.

It has also been updated to the latest git ctrulib (in which FS_Archive is a u64,
not a struct.) Thanks, @TuxSH. It required manual conflict merged (my code is different)
but you provided the information needed to fix it up.

## Build
You need a working 3DS build environment with a fairly recent copy of devkitARM, 
ctrulib, and makerom.

Currently, there is no support for FIRM building in toolchain; whatever method
you use to inject this is up to you, but the software must have support for
resizing the sysmodule segment, or you must make sure the module size matches
the original.
