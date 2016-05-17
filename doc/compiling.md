Cloning / Compilation
--------------------------

Okay, first thing's first. I can't support compiling on Windows. Use Linux. The repo abuses symbolic links links with good reason.

You'll need the following to compile:

 * git
 * gcc (as in, host gcc)
 * arm-none-eabi-gcc (as in, devkitarm OR baremetal...but I can't necessarily help with the latter)
 * a brain (hard requirement)

To clone: use `git clone --recursive`. Some parts of this CFW are submodules.

To build: run `make`. Output is in `out`. Done.

Not every revision is 100% guaranteed to build. I'll set up a CI eventually.
