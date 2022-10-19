#!/bin/sh

# Stop FMC watchdog
echo "Stop FMC WDT2"
devmem 0x1e620064 w 0x0

# Check emmc
EMMC_CHECK_SH="/usr/sbin/emmc-check.sh"
bash $EMMC_CHECK_SH

# run cpld verion
CPLD_CHECK="/usr/sbin/transformers-cpld-init.sh"
bash $CPLD_CHECK