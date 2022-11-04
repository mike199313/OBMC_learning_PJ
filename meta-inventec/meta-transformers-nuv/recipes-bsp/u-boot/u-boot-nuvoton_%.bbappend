FILESEXTRAPATHS:prepend:transformers-nuv := "${THISDIR}/u-boot-nuvoton:"

UBOOT_MAKE_TARGET:append:transformers-nuv = " DEVICE_TREE=${UBOOT_DEVICETREE}"

SRC_URI:append:transformers-nuv = " file://fixed_phy.cfg \
                                    file://0002-Add-enable-espi-four-channel-config.patch \
                                    file://0003-Bug-631-SW-Transformers-nuv-OpenBMC-Support-mc-selft.patch \
                                    file://0004-Add-Windbond-W25Q512JVFIM.patch \
                                    file://0005-Add-update-mac-address-info.patch \
                                    "



include conf/machine/platform_configs.inc

EEPROM_MAC_I2C_BUS = "8"
EEPROM_MAC_I2C_ADDRESS = "0x51"
EEPROM_MAC_OFFSET = "0x400"
EEPROM_MAC_I2C_DEV_SPEED = "100000"
EEPROM_MAC_I2C_ADDR_LEN = "2"
EEPROM_ETH0_ADDR = "0xf0825000"

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
