Cloning / Compilation
--------------------------

Okay, first thing's first. I can't support compiling on Windows. Use Linux. The repo abuses symbolic links, relies on certain software being present which isn't with devkitPro, and has host tools which are untested on Windows and require a native gcc (as in, mingw only.)

If you want to fix windows builds, be my guest. I'll merge it so long as it A) doesn't break my machine's builds and B) uses mingw/cygwin as the native compiler. No msvcisms will be allowed on my watch.

You'll need the following to compile:

 * git
 * gcc (as in, host gcc)
 * arm-none-eabi-gcc (as in, devkitarm OR baremetal gcc (the latter isn't well tested, but may work)
 * a brain (hard requirement)

To clone: use `git clone --recursive`. Some parts of this CFW are submodules.

To build: run `make`. Output is in `out`. Done. Optionally, parallel make works so pass `-jN` to speed up things.

Not every revision is 100% guaranteed to build. I'll set up a CI eventually.
