#!/bin/bash

# Okay, so obviously this file would be non-redistributable if I put the keys here.
# You'll need to get them yourself (in plaintext) and put them in the correct place.

# Optionally, you can call this file with an option
# and the output binary will contain the keys. DO NOT do this unless you know
# you will NOT share the binaries; the resultant output is NOT MY PROBLEM, and
# logs will be marked as "Tainted key loading used". If I have any reason to believe
# you have enabled this option, NO SUPPORT will be provided unless you go and use
# a build with that off.

if [ "$1" == "--tainted-no-support" ]; then
	WELP_USER_IS_A_WEIRDO_BUT_WHATEVER="$1"
fi

function key() {
    if [ -e "keys/$1.txt" ]; then
		echo "Generating key metadata..."
	    mkdir -p ../source/firm/keys
		./key_char $WELP_USER_IS_A_WEIRDO_BUT_WHATEVER $(cat keys/$1.txt | tr -d '\n') > ../source/firm/keys/$1.gen
	else
		echo "Key not found, generating stub..."
        echo -n "{}" > ../source/firm/keys/$1.gen
	fi

    echo "$2" >> ../source/firm/keys/$1.gen
}

key Y11_95 ","
key Y11_96 ""

key Y05 ""

key Y3D_0 ","
key Y3D_1 ","
key Y3D_2 ","
key Y3D_3 ","
key Y3D_4 ","
key Y3D_5 ""
