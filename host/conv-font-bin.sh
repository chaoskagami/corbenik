#!/bin/bash

if [ "${CC}" == "" ]; then
	CC=gcc
fi

set -e

./host/bdfe/bdfe -n -L -v -A external/tewi-font/tewi-medium-11.bdf > host/font.h
grep "// Converted Font Size" host/font.h | sed -e 's|.* ||g' \
	-e 's|^|const int font_width = |g' \
	-e 's|x|;\nconst int font_height = |g' \
	-e 's|$|;|g' > host/font_prop.h
${CC} ${CFLAGS} -o host/font-emit host/font-emit.c
./host/font-emit
