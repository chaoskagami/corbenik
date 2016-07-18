#!/bin/bash

# This is just a helper script I use to copy shit to my own unit.
# You don't and shouldn't need to use it.

mnt=/media/sd
dev=/dev/sdb

mnt=/mnt/ext1

mount -t vfat ${dev}1 $mnt || exit 0
rm -rf $mnt/corbenik
cp -r out/* $mnt/ || exit 0
cp -r input/* $mnt/ || exit 0
cp out/arm9loaderhax.bin $mnt/corbenik/boot/Corbenik || exit 0
umount $mnt || exit 0
sync || exit 0
eject ${dev} || exit 0
