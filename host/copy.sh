#!/bin/bash

# This is just a helper script I use to copy shit to my own unit.
# You don't and shouldn't need to use it.

mnt=/media
dev=/dev/mmcblk0p

mount -t vfat ${dev}1 $mnt || exit 0
rm $mnt/arm9loaderhax{,_si}.bin
rm -rf $mnt/corbenik
cp -r out/* $mnt/ || exit 0
if [ "$1" = "n" ]; then
    ( cd $mnt/ && ./n3ds_firm.sh )
else
    ( cd $mnt/ && ./o3ds_firm.sh )
fi
umount $mnt || exit 0
sync || exit 0
