#!/bin/bash

# This is just a helper script I use to copy shit to my own unit.
# You don't and shouldn't need to use it.

mnt=/media
dev=/dev/mmcblk0p

mount -t vfat ${dev}1 $mnt || exit 0
rm -rf $mnt/corbenik
cp -r out/* $mnt/ || exit 0
( cd $mnt/ && ./n3ds_firm.sh )
mv $mnt/arm9loaderhax{,_si}.bin
umount $mnt || exit 0
sync || exit 0
