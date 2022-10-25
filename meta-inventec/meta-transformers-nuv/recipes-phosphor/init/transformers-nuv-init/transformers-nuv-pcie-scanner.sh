#!/bin/sh

I2C_BUS_RISER1=24
I2C_BUS_RISER2=25
I2C_BUS_RISER3=22

E810_FRU_ADDRESS=0x50

function setpresent() {
        `busctl call \
        xyz.openbmc_project.Inventory.Manager \
        /xyz/openbmc_project/inventory \
        xyz.openbmc_project.Inventory.Manager \
        Notify a{oa{sa{sv}}} 1 \
        /system/chassis/motherboard/pcie_slot$1 1 xyz.openbmc_project.Inventory.Item 1 \
        "Present" "b" $2`

}

for i in {1..3}
do
NUMBER="$i"
BUS=$(("I2C_BUS_RISER$NUMBER"))

ret=`i2cget -y -f $BUS $E810_FRU_ADDRESS 0`
echo $ret

if [ -z $ret ];
    then
        echo "$BUS not ready"
        (setpresent $NUMBER "false")
    else
        if [ ! -f "/sys/bus/i2c/devices/i2c-$BUS/$BUS-0050/eeprom" ]
        then
            `echo 24c512 $E810_FRU_ADDRESS > /sys/bus/i2c/devices/i2c-$BUS/new_device`
            echo Mount i2c-$BUS eeprom driver
        else
            echo "$BUS-0050 exists."
            (setpresent $NUMBER "true")
        fi
fi
done

sleep 20
