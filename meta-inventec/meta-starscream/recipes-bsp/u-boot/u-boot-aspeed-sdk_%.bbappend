FILESEXTRAPATHS:append := "${THISDIR}/${BPN}:"


SRC_URI:append = " file://starscream-ast2600.cfg \
                   file://ast2600-starscream.dts \
                   file://0001-Modify-bootfile-name-and-env-offset.patch \
                   file://0002-Add-debug-mesg-to-show-SPI-clock-frequency.patch \
                   file://0003-Read-MAC0-address-from-EEPROM.patch \
                   file://0004-Support-max31790-device-driver.patch \
                   file://0005-Initial-starscream-machine.patch \
                   file://0006-Interface-readiness.patch \
                 "

do_copyfile () {
    if [ -e ${WORKDIR}/ast2600-starscream.dts ] ; then
        cp -v ${WORKDIR}/ast2600-starscream.dts ${S}/arch/arm/dts/
    else
        # if use devtool modify, then the append files were stored under oe-local-files
        cp -v ${S}/oe-local-files/ast2600-starscream.dts ${S}/arch/arm/dts/
    fi
}

addtask copyfile after do_patch before do_configure

include conf/machine/platform_configs.inc

EEPROM_MAC_I2C_BUS = "8"
EEPROM_MAC_I2C_ADDRESS = "0x51"
EEPROM_MAC_OFFSET = "0x400"
EEPROM_MAC_I2C_DEV_SPEED = "100000"
EEPROM_MAC_I2C_ADDR_LEN = "2"
EEPROM_ETH0_ADDR = "0x1e690000"

do_patch_headerfile () {
  cat >${S}/include/configs/IECplatformConfigs.h <<EOF
// This header file is automatically created, DO NOT EDIT IT.
#ifndef __IEC_PLATFORM_CONFIGS_H__
#define __IEC_PLATFORM_CONFIGS_H__

#define EEPROM_MAC_I2C_BUS (${EEPROM_MAC_I2C_BUS})
#define EEPROM_MAC_I2C_ADDRESS (${EEPROM_MAC_I2C_ADDRESS})
#define EEPROM_MAC_OFFSET (${EEPROM_MAC_OFFSET})
#define EEPROM_MAC_I2C_DEV_SPEED (${EEPROM_MAC_I2C_DEV_SPEED})
#define EEPROM_MAC_I2C_ADDR_LEN (${EEPROM_MAC_I2C_ADDR_LEN})
#define EEPROM_ETH0_ADDR (${EEPROM_ETH0_ADDR})

#endif /* __IEC_PLATFORM_CONFIGS_H__ */
EOF
}

addtask patch_headerfile after do_patch before do_configure

