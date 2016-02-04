#! /bin/sh

make;
make u-boot.hex;

mv u-boot.bin $1.bin
mv u-boot.hex $1.hex

cp $1.bin /mnt/