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

There is also support for SDMC read/write (not found in original loader
implementation) which means that information can be logged to SD.

## About this fork

A lot of the 'disassembled' looking code has been rewritten for readability
(notable the lzss decompressor), and some cruft was cleaned out. Notably,
IFile increased the cxi size by two (!) pages, and therefore has been removed,
as it offers nothing over direct use of FSFILE and FSLDR.

This version of loader is capable of logging to the filesystem through added
write support. Tecnically, all the work for this was already in place, but IFile
didn't allow accessing this functionality. This makes debugging much easier,
since you can now actually figure out what went wrong.

Experimental support for resizing segments before loading the executable was
added. This means that segments can be appended to. Notably, this allows tacking
code onto the end.

The text, data, and ro segments are now handled separately to speed things up
marginally, since if you know a patch is applied to text there's no reason to
search the data segment, etc.

## Imported changes from other 3ds_injector forks

This was updated to the latest git ctrulib (in which FS_Archive typing has
changed to allow mountpoints.)

@TuxSH did this work before me, although I had to implement changes
manually due to my tree sharing almost nothing at this point with upstream or
Luma. It did serve as a useful reference, though. :)

## Build
You need a working 3DS build environment with a fairly recent copy of devkitARM, 
ctrulib, and makerom.

This will automatically be built by corbenik's top-level makefile, so you shouldn't
need to monkey around in here if you aren't making changes.

## Misc

This is not intended to be used with anything but corbenik due to specific use of
structures incompatible with other CFW, so don't attempt to inject this with other
CFWs. At best, it doesn't work. At worst, you can't boot without removing it.

For devs - message me if there's any changes you want help merging to your fork.
I'll be glad to help, either to explain what has been done here or to port changes.
I'm not into anti-competitive behavior, since we're all just trying to make the 3DS
better as a whole. ;P
