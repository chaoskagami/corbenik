3DS Loader Replacement
======================

This is an open source implementation of 3DS `loader` system module--with 
additional features. The current aim of the project is to provide a nice 
entry point for patching 3DS modules.

## Roadmap
Right now, this can serve as an open-source replacement for the built in loader,
and then some.

There is support for patching any executable after it's loaded but 
before it starts. For example, you can patch `menu` to skip region checks and 
have region free game launching directly from the home menu.

This currently requires recompilation which makes it less than ideal.

There is also support for SDMC reading (not found in original loader
implementation) which  means that patches can be loaded from the SD card.
Ultimately, there will be a patch system that supports easy loading of
patches from the SD card.

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
text, data and ro (big) when you know it is in text?

## Imported changes from other 3ds_injector forks

I updated it to the latest git ctrulib (in which FS_Archive is a u64,
not a struct.) @TuxSH did the work before me, although it required manual
conflict merges since my tree differs a LOT from both @yifanlu and his
version.

## Build
You need a working 3DS build environment with a fairly recent copy of devkitARM, 
ctrulib, and makerom.

This is not intended to be used with anything but corbenik, so please don't use
binaries of this with any other CFW. For devs - message me if there's any changes
you want help merging. I'll be glad to help. I'm not into anti-competitive
behavior. ;P
