#!/bin/sh

function initBlock(){
fdisk /dev/mmcblk0 <<EOF
n
p
1


w
EOF
}

if [ ! -b "/dev/mmcblk0p1" ]
then
    echo "/dev/mmcblk0p1 does not exists."
    initBlock
    mkfs.ext4 /dev/mmcblk0p1
else
    echo "/dev/mmcblk0p1 exists."
fi

if [ ! -d "/var/log/journal" ]
then
    mkdir -p /var/log/journal
    echo "Create /var/log/journal folder"
fi
