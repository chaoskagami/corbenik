3DS Loader Replacement
======================

This is an open source implementation of 3DS `loader` system module -- with additional features. The current aim of the project is to provide a nice entry point for patching 3DS modules.

## Roadmap
Right now, this can serve as an open-source replacement for the built in loader, and then some.

There is support for patching any executable after it's loaded but before it starts. For example, you can patch `menu` to skip region checks and have region free game launching directly from the home menu.

There is also support for SDMC read/write (not found in original loader implementation) which means that information can be logged to SD, as well as code loading and dumping, etc.

## Changes in this fork

 * A lot of the 'disassembled' looking code has been rewriteen for readability.
   * The primary example is the lzss code, which is actually in the common include directory.
 * Some unneeded cruft was cleaned out.
   * IFile is gone - it offers nothing over FSFILE here, aside from growing the cxi two(!) mediaunits.
 * File read and write support.
 * Logging capabilities.
 * Code loading / storing as split segments (text / data / ro)
 * Proper support for a heap in the BASE region
 * Bytecode patchers, using the toplevel interpreter in src/interpreter.c

## Imported changes from other 3ds_injector forks

This was updated to the latest git ctrulib (in which FS_Archive typing has changed to allow mountpoints.)

@TuxSH did this work before me, although I had to implement changes manually due to my tree sharing almost nothing at this point with upstream or Luma. It did serve as a useful reference, though. :)

## Build
You need a working 3DS build environment with a fairly recent copy of devkitARM, ctrulib, and makerom.

This will automatically be built by corbenik's top-level makefile, so you shouldn't need to monkey around in here if you aren't making changes.

## Misc

This is not intended to be used with anything but corbenik due to specific use of structures incompatible with other CFW, so don't attempt to inject this with other CFWs. At best, it doesn't work. At worst, you can't boot without removing it.

For devs - message me if there's any changes you want help merging to your fork. I'll be glad to help, either to explain what has been done here or to port changes. I'm not into anti-competitive behavior, since we're all just trying to make the 3DS better as a whole. ;P
