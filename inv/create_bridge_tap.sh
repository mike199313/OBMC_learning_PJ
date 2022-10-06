#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage sudo $0 <ifname> [clean]"
    #exit 1
fi

defaultIf="enp0s3"
ifname=${1:-${defaultIf}}


if [ "$2" == 'clean' ]; then
    echo "Cleanup...tun/tap"

    #Make sure everything is down
    sudo ifconfig ${ifname} down
    sudo ifconfig tap0 down
    sudo ifconfig br0 down

    #Remove tap interface tap0 from bridge br0
    sudo brctl delif br0 tap0

    #Delete tap0
    sudo tunctl -d tap0

    #Remove ${ifname} from bridge
    sudo brctl delif br0 ${ifname}

    #Bring bridge down
    sudo ifconfig br0 down

    #Remove bridge
    sudo brctl delbr br0

    #Bring ${ifname} up
    ifconfig ${ifname} up

    #Check if an IP is assigned to ${ifname}, if not request one
    sudo dhclient -v ${ifname}
else
    #Create a bridge
    sudo brctl addbr br0

    #Clear IP of ${ifname}
    sudo ip addr flush dev ${ifname}
    sudo ifconfig ${ifname} down

    #Add eth0 to bridge
    sudo brctl addif br0 ${ifname}

    #Create tap interface
    sudo tunctl -t tap0

    #Add tap0 to bridge
    sudo brctl addif br0 tap0

    #Make sure everything is up
    sudo ifconfig ${ifname} up
    sudo ifconfig tap0 up
    sudo ifconfig br0 up

    #Check if properly bridged
    sudo brctl show

    #Assign ip to br0
    sudo dhclient -v br0
fi

echo "If you want to run qemu-system-arm, please refer to below"
echo  "sudo ./qemu-system-arm -m 256 -M ast2600-evb -nographic -drive file=<$image>,format=raw,if=mtd -net nic -net tap,ifname=tap0,script=no,downscript=no"
