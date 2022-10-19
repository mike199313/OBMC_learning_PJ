#!/bin/sh

if [ $# -ne 4 ]
then
    echo "usage: $0 <start|stop> <nbd device> <gadget number> <isCdrom>" >&2
    exit 1
fi

# action : start or stop
action=$1

# nbd device : /dev/nbdX (X=0,1,2...)
nbd_device=$2

#gadget number
gadgetnum=$3

# media type 1 is cdrom, 0 otherwise
isCdrom=$4

gadget_name=mass-storage
gadget_dir=/sys/kernel/config/usb_gadget/$gadget_name${gadgetnum}


set -ex

case "$action" in
start)
    mkdir -p $gadget_dir
    (
    cd $gadget_dir
    # http://www.linux-usb.org/usb.ids
    #    |-> 1d6b  Linux Foundation
    #          |-> 0104  Multifunction Composite Gadget
    echo "0x1d6b" > idVendor
    echo "0x0104" > idProduct
    mkdir -p strings/0x409
    echo "OpenBMC" > strings/0x409/manufacturer
    echo "MS Media Redirect" > strings/0x409/product
    mkdir -p configs/c.1/strings/0x409
    echo "config 1" > configs/c.1/strings/0x409/configuration
    mkdir -p functions/mass_storage.usb0
    ln -s functions/mass_storage.usb0 configs/c.1
    echo 1 > functions/mass_storage.usb0/lun.0/removable
    echo 1 > functions/mass_storage.usb0/lun.0/ro
    echo ${isCdrom} > functions/mass_storage.usb0/lun.0/cdrom
    echo ${nbd_device} > functions/mass_storage.usb0/lun.0/file
    echo "1e6a0000.usb-vhub:p"${gadgetnum} > UDC
    )
    ;;
stop)
    (
    cd $gadget_dir
    rm configs/c.1/mass_storage.usb0
    rmdir functions/mass_storage.usb0
    rmdir configs/c.1/strings/0x409
    rmdir configs/c.1
    rmdir strings/0x409
    )
    rmdir $gadget_dir
    ;;
*)
    echo "invalid action $action" >&2
    exit 1
esac

exit 0

