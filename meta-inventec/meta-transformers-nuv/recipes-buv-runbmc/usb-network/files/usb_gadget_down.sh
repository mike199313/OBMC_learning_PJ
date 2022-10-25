#!/bin/sh

TARGET=$1

cd /sys/kernel/config/usb_gadget/

if [ ! -d $TARGET ]; then
    exit 0
fi

cd ./$TARGET

# Disable gadget
echo "" > UDC

# Remove gadget config files
for dir in configs/*/strings/*; do
	[ -d $dir ] && rmdir $dir
done

for func in configs/*.*/*.*; do
	[ -e $func ] && rm $func
done

for conf in configs/*; do
	[ -d $conf ] && rmdir $conf
done

for func in functions/*.*; do
	[ -d $func ] && rmdir $func
done

for str in strings/*; do
	[ -d $str ] && rmdir $str
done

cd ..
rmdir $TARGET
exit 0