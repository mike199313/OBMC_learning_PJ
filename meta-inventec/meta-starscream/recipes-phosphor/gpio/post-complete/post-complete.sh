#!/bin/sh
CPU1_I2C_BUS=16
CPU2_I2C_BUS=17

CPU1_SBRMI_ADDRESS=0x3C
CPU2_SBRMI_ADDRESS=0x38


host_state=$1



if [ "$host_state" = "on" ]; then
    echo "Host On"

    echo  sbrmi $CPU1_SBRMI_ADDRESS > /sys/bus/i2c/devices/i2c-$CPU1_I2C_BUS/new_device
    echo  sbrmi $CPU2_SBRMI_ADDRESS > /sys/bus/i2c/devices/i2c-$CPU2_I2C_BUS/new_device

    systemctl restart xyz.openbmc_project.psusensor.service
    systemctl restart phosphor-pid-control.service

    # Rebind OCP network driver
    echo 1e670000.ftgmac > /sys/bus/platform/drivers/ftgmac100/unbind
    echo 1e670000.ftgmac > /sys/bus/platform/drivers/ftgmac100/bind

elif [ "$host_state" = "off" ]; then
    echo "Host Off"

    echo $CPU1_SBRMI_ADDRESS > /sys/bus/i2c/devices/i2c-$CPU1_I2C_BUS/delete_device
    echo $CPU2_SBRMI_ADDRESS > /sys/bus/i2c/devices/i2c-$CPU2_I2C_BUS/delete_device

    systemctl restart xyz.openbmc_project.psusensor.service
elif [ "$host_state" = "init" ]; then
    echo "Host already on"

    echo  sbrmi $CPU1_SBRMI_ADDRESS > /sys/bus/i2c/devices/i2c-$CPU1_I2C_BUS/new_device
    echo  sbrmi $CPU2_SBRMI_ADDRESS > /sys/bus/i2c/devices/i2c-$CPU2_I2C_BUS/new_device
else
    echo "unknow state $1"
    exit 1;
fi

exit 0;


