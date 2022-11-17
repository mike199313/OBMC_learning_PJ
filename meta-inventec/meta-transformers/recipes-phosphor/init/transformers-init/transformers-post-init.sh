#!/bin/sh

gpioset `gpiofind BMC_READY`=0
echo BMC ready !!
gpioset `gpiofind RST_BMC_SGPIO`=1
echo Release reset SGPIO !!

if [ ! -d "/var/wcs/home/log" ]
then
    ln -s /var/log /var/wcs/home/
    echo "Create soft link /var/wcs/home/log"
fi
