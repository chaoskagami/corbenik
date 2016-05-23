#!/bin/bash

# This is just a helper script I use to copy shit to my own unit.
# You don't and shouldn't need to use it.

mnt=/media/sd
dev=/dev/sdc

mount ${dev}1 $mnt || exit 0
cp out/arm9loaderhax.bin $mnt/anim/boot/none.bin || exit 0
cp out/arm9loaderhax.bin $mnt/anim/boot/r.bin || exit 0
cp out/arm9loaderhax.bin $mnt/anim/boot/l.bin || exit 0
cp -r out/corbenik $mnt/ || exit 0
cp -r input/corbenik $mnt/ || exit 0
umount $mnt || exit 0
eject ${dev} || exit 0
