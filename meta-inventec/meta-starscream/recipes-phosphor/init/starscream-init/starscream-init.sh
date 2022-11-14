#!/bin/sh


# Set BMC ready
gpioset `gpiofind BMC_READY`=0
echo BMC ready !!

# Check emmc
EMMC_CHECK_SH="/usr/sbin/emmc-check.sh"
bash $EMMC_CHECK_SH

# run cpld verion
CPLD_CHECK="/usr/sbin/starscream-cpld-init.sh"
bash $CPLD_CHECK
