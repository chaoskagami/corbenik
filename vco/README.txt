Corbenik patch binaries
----------------------------------

This directory contains the source for vco files - the patcher executables.

You're probably wondering what the heck corbenik does differently from cakes,
considering it seems similar in many ways.

Patches are actually code for whatever processor they're intended to run on,
be it ARM9 or ARM11. They're loaded to a static offset in memory, and executed
from there with relocations to corbenik's internal functions. This keeps patches
relatively small, and allows complete control over behavior.

Patches should have a declaration of this sort somewhere in them:
  { 0xc0, 0x9b, 0xe5, 0x1c }
Followed by a table large enough to fill with all usable functions.

The loader is subject to change at any moment's notice; the ABI is not yet
stable. It may become an ELF loader at some point. I don't know.

You may want to consult src/loader.c to see what functions are exported, or
simply base your code on the generic example in the 'template' folder instead.

There's some key differences here, obviously, from running just arm9loader code. Namely:

  1) Patches must not clobber the previous state. Meaning; start does nothing but
     chain to main.

  2) Patches must properly return, and also return a value. Return code 0 is
     success; keep this in mind. Corbenik will attempt to reset after a non-
     zero return code. If you don't know how to return; you're looking for
     'bx lr'.

  3) Patches must have a symbol table with the appropriate magic.
     No symbol table? No load. This might be relaxed in future versions
     to allow patches to be marked static, but IDK.

  4) Don't code a patch that does too fancy stuff. Patches are not intended to
     be 65k binaries. Seriously.

  5) _start must be at offset 0x24400000. This is where you are in memory.

You can implement shit yourself, but it's an utter waste of memory. Try to use
the linker exports unless you have a good reason not to.
