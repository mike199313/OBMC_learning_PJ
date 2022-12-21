inherit buv-image
inherit extrausers
EXTRA_USERS_PARAMS:append = " \
useradd -e '' -ou 0 -d /var/wcs/home -G priv-admin,root,sudo,ipmi,web,redfish -p 'gzW59equAcJAg' sysadmin; \
useradd -e '' -ou 0 -d /var/wcs/home -G priv-admin,root,sudo,ipmi,web,redfish -p 'kFdHdjRkot8KQ' admin; \
"

do_prepare_bootloaders() {
    local olddir="$(pwd)"
    cd ${DEPLOY_DIR_IMAGE}
    bingo ${IGPS_DIR}/BootBlockAndHeader_${IGPS_MACHINE}.xml \
            -o ${DEPLOY_DIR_IMAGE}/${BOOTBLOCK}.${FULL_SUFFIX}

    bingo ${IGPS_DIR}/UbootHeader_${IGPS_MACHINE}.xml \
            -o ${DEPLOY_DIR_IMAGE}/${UBOOT_BINARY}.${FULL_SUFFIX}

    bingo ${IGPS_DIR}/mergedBootBlockAndUboot.xml \
            -o ${DEPLOY_DIR_IMAGE}/${UBOOT_BINARY}.${MERGED_SUFFIX}

    mv ${UBOOT_BINARY}.${MERGED_SUFFIX} uboot-tmp.bin
    filesize=$(stat -c %s "uboot-tmp.bin")
    checksum=`md5sum uboot-tmp.bin | awk '{ print $1 }'`
    echo "INVENTEC_UBOOT_SIZE_${filesize}_CHECKSUM_${checksum}" > uboot-checksum
    cat uboot-tmp.bin uboot-checksum >> ${UBOOT_BINARY}.${MERGED_SUFFIX}

    cd "$olddir"
}

OBMC_IMAGE_EXTRA_INSTALL:append = " openssh-sftp-server"
OBMC_IMAGE_EXTRA_INSTALL:append = " phosphor-ipmi-ipmb"
OBMC_IMAGE_EXTRA_INSTALL:append = " python3-smbus"
OBMC_IMAGE_EXTRA_INSTALL:append = " ipmitool"
OBMC_IMAGE_EXTRA_INSTALL:append = " gpiolib"
#BMC_IMAGE_EXTRA_INSTALL:append = " rest-dbus"
#OBMC_IMAGE_EXTRA_INSTALL:append = " cpld"
OBMC_IMAGE_EXTRA_INSTALL:append = " mmc-utils"
OBMC_IMAGE_EXTRA_INSTALL:append = " transformers-nuv-init"
OBMC_IMAGE_EXTRA_INSTALL:append = " libsafec"
OBMC_IMAGE_EXTRA_INSTALL:append = " intel-ipmi-oem"
OBMC_IMAGE_EXTRA_INSTALL:append = " inventec-util"
OBMC_IMAGE_EXTRA_INSTALL:append = " inv-ipmi-oem"
OBMC_IMAGE_EXTRA_INSTALL:append = " inventec-dcmi-power"
OBMC_IMAGE_EXTRA_INSTALL:append = " peci-pcie"
OBMC_IMAGE_EXTRA_INSTALL:append = " cpldupdate-i2c"
OBMC_IMAGE_EXTRA_INSTALL:append = " bios-update"
OBMC_IMAGE_EXTRA_INSTALL:append = " monitor-bios-update"
OBMC_IMAGE_EXTRA_INSTALL:append = " inv-transformers-nuv-ipmi-oem"
OBMC_IMAGE_EXTRA_INSTALL:append = " inv-psu-update"
OBMC_IMAGE_EXTRA_INSTALL:append = " smbios-mdr smbios-mdr-reload"
OBMC_IMAGE_EXTRA_INSTALL:append = " gpio-event-handler"
OBMC_IMAGE_EXTRA_INSTALL:append = " inventec-mac-config"
OBMC_IMAGE_EXTRA_INSTALL:append = " usb-network"
OBMC_IMAGE_EXTRA_INSTALL:append = " phosphor-ipmi-flash"
OBMC_IMAGE_EXTRA_INSTALL:append = " configuration-manager"
OBMC_IMAGE_EXTRA_INSTALL:append = " acpi-power-state-daemon"
OBMC_IMAGE_EXTRA_INSTALL:append = " configure-firmware-status"
