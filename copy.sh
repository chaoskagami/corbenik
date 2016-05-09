#!/bin/bash
mount /dev/sdb1 /media/sd
cp out/arm9loaderhax.bin /media/sd/anim/boot/a.bin
umount /media/sd
