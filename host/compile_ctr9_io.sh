#!/bin/bash

unset CFLAGS
unset LDFLAGS
unset ASFLAGS
unset CROSS_CFLAGS
unset CROSS_LDFLAGS
unset CROSS_ASFLAGS

cd external/libctr9_io
git clean -fxd
autoreconf -fi
./configure --host arm-none-eabi --prefix=$(pwd)/out
make
make install
cd ../../
