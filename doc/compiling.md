Cloning / Compilation
--------------------------

Okay, first thing's first. I can't support compiling on Windows. Use Linux. The repo abuses symbolic links, relies on certain software being present which isn't with devkitPro, and has host tools which are untested on Windows and require a native gcc (as in, mingw only.) Therefore, do NOT attempt to use windows to compile this. No, I will not port the build scripts to Windows. If you want to fix it without invasive changes, be my guest and make a PR. If I deem the maintenance cost too high, I'll tell you.

It may or may not compile with an OSX based cross compiler. I dunno.

You'll need the following to compile:

 * git
 * python2
 * gcc (as in, host gcc)
 * arm-none-eabi-gcc (as in, devkitarm OR baremetal gcc (the latter isn't well tested, but may work. Compiling devkitPro using https://github.com/devkitPro/buildscripts is highly recommended.)
 * bash (as /bin/bash)

To clone: use `git clone --recursive`. Some parts of this CFW are submodules.

To build: run `make`. Output is in `out`. Done. Optionally, parallel make works so pass `-jN` to speed up things.

Not every revision is 100% guaranteed to build. I'll set up a CI eventually.
