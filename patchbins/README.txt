You're probably wondering what the heck corbenik does differently from cakes,
considering it seems similar.

Patches are actually code for whatever processor they're intended to run on.
They're loaded to a static offset in memory, and executed.

Patches should have a declaration of this sort somewhere in them:
  { 0xc0, 0x9b, 0xe5, 0x1c }
Followed by a table large enough to fill with all usable functions.

See template/ for how this works.

When the loader finds that magic value, it fills the table with
the offsets of important variables and functions for use by the patcher, much
like how an ELF loader sets up references. In other words; patches are binaries
that are relocated relative to a 'standard library'.

There's some key differences, obviously, from running just arm9loader code.
Namely:
  1) Patches must not clobber the previous state.
  2) Patches must properly return.
  3) Patches must have a symbol table with the appropriate magic.
  4) Patches shouldn't do anything aside from change data.
  5) _start must be at offset 0x24400000.

Basically, don't get fancy in _start. Just do a bl main; bx lr. You can implement
shit yourself, but it's an utter waste of memory. Try not to.
