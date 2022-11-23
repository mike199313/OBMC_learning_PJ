#!/bin/sh

# Init GPIO setting

gpioset `gpiofind BMC_READY`=0
echo BMC ready !!
gpioset `gpiofind RST_BMC_SGPIO`=1
echo Release reset SGPIO !!
echo c0000000.spi > /sys/bus/platform/drivers/NPCM-FIU/unbind

# Init SGPIO output pin to high, avoid they reset i2c mux IC
gpioset `gpiofind RST_CPU_I2C_MUX_R_N`=1
gpioset `gpiofind RST_BMC_EDSFF_I2C_R_N`=1
gpioset `gpiofind RST_BMC_CPU_I2C_R_N`=1
gpioset `gpiofind RST_PE_I2C_MUX_R_N`=1

# Check emmc
EMMC_CHECK_SH="/usr/sbin/emmc-check.sh"
bash $EMMC_CHECK_SH

# run cpld verion
CPLD_CHECK="/usr/sbin/transformers-nuv-cpld-init.sh"
bash $CPLD_CHECK

if [ ! -d "/var/wcs/home/log" ]
then
    ln -s /var/log /var/wcs/home/
    echo "Create soft link /var/wcs/home/log"
fi
