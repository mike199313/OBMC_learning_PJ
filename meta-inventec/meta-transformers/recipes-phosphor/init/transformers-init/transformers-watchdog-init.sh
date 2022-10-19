#!/bin/sh
LOGGIN_DBUS_SERVICE="xyz.openbmc_project.Logging.IPMI"
LOGGIN_DBUS_OBJECT="/xyz/openbmc_project/Logging/IPMI"
LOGGIN_DBUS_INTERFACE="xyz.openbmc_project.Logging.IPMI"

BMC_HEALTH_DBUS_OBJECT="/xyz/openbmc_project/sensors/oem_event_70h/oem_e0h/BMC_health"

WDT2_TIMEOUT_STAT_REG="0x1e785050"
WDT2_CLEAR_TIMEOUT_STAT_REG="0x1e785054"
WDT2_CLEAR_TIMEOUT_MAGIC="0x76"

FW_WDT2_CONTROL_REG="0x1e620064"

echo "Check watchdog timeout status"

reg=$((`devmem $WDT2_TIMEOUT_STAT_REG`))

echo $reg

# Timeout Status
# 23:20 - ARM reset
# 19:16 - Full reset
# 15:8  - SCO reset
# 0x00100(256)
if [ $reg -ge 256 ]; then
    echo "Timeout before"
    # Store SEL

    busctl call $LOGGIN_DBUS_SERVICE $LOGGIN_DBUS_OBJECT $LOGGIN_DBUS_INTERFACE \
    "IpmiSelAdd" ssaybq "BMC health" \
    $BMC_HEALTH_DBUS_OBJECT 3 {0xa3,0x00,0x00} yes 0x20
fi

#clear WDT2 timeout status
devmem $WDT2_CLEAR_TIMEOUT_STAT_REG w $WDT2_CLEAR_TIMEOUT_MAGIC

echo "Check FW WDT2 control"

reg=$((`devmem $FW_WDT2_CONTROL_REG`))

echo $reg


BMC_HEALTH_SPI_FLASH_BYTE1=0xa2

if [ $((reg & 0x10)) -eq $((0x10)) ]; then
    echo "boot from flash2"
    BMC_HEALTH_SPI_FLASH_BYTE2=2
else
    echo "boot from flash1"
    BMC_HEALTH_SPI_FLASH_BYTE2=1
fi
BMC_HEALTH_SPI_FLASH_BYTE3=0

busctl call $LOGGIN_DBUS_SERVICE $LOGGIN_DBUS_OBJECT $LOGGIN_DBUS_INTERFACE \
"IpmiSelAdd" ssaybq "BMC health" \
$BMC_HEALTH_DBUS_OBJECT 3 \
{$BMC_HEALTH_SPI_FLASH_BYTE1,$BMC_HEALTH_SPI_FLASH_BYTE2,$BMC_HEALTH_SPI_FLASH_BYTE3} \
yes 0x20




