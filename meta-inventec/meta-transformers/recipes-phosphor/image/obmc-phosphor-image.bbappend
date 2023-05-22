inherit extrausers

EXTRA_USERS_PARAMS:append = " \
useradd -e '' -ou 0 -d /var/wcs/home -G priv-admin,root,sudo,ipmi,web,redfish -p 'gzW59equAcJAg' sysadmin; \
useradd -e '' -ou 0 -d /var/wcs/home -G priv-admin,root,sudo,ipmi,web,redfish -p 'kFdHdjRkot8KQ' admin; \
"
OBMC_IMAGE_EXTRA_INSTALL:append = " openssh-sftp-server"
OBMC_IMAGE_EXTRA_INSTALL:append = " phosphor-ipmi-ipmb"
OBMC_IMAGE_EXTRA_INSTALL:append = " python3-smbus"
OBMC_IMAGE_EXTRA_INSTALL:append = " ipmitool"
OBMC_IMAGE_EXTRA_INSTALL:append = " gpiolib"
#BMC_IMAGE_EXTRA_INSTALL:append = " rest-dbus"
OBMC_IMAGE_EXTRA_INSTALL:append = " cpld"
OBMC_IMAGE_EXTRA_INSTALL:append = " mmc-utils"
OBMC_IMAGE_EXTRA_INSTALL:append = " transformers-init"
OBMC_IMAGE_EXTRA_INSTALL:append = " safec"
OBMC_IMAGE_EXTRA_INSTALL:append = " at-scale-debug"
OBMC_IMAGE_EXTRA_INSTALL:append = " inventec-util"
OBMC_IMAGE_EXTRA_INSTALL:append = " inv-ipmi-oem"
OBMC_IMAGE_EXTRA_INSTALL:append = " inventec-dcmi-power"
OBMC_IMAGE_EXTRA_INSTALL:append = " peci-pcie"
OBMC_IMAGE_EXTRA_INSTALL:append = " cpldupdate-i2c"
OBMC_IMAGE_EXTRA_INSTALL:append = " bios-update"
OBMC_IMAGE_EXTRA_INSTALL:append = " monitor-bios-update"
OBMC_IMAGE_EXTRA_INSTALL:append = " inv-transformers-ipmi-oem"
OBMC_IMAGE_EXTRA_INSTALL:append = " inv-psu-update"
OBMC_IMAGE_EXTRA_INSTALL:append = " smbios-mdr smbios-mdr-reload"
OBMC_IMAGE_EXTRA_INSTALL:append = " gpio-event-handler"
OBMC_IMAGE_EXTRA_INSTALL:append = " inventec-mac-config"
OBMC_IMAGE_EXTRA_INSTALL:append = " acpi-power-state-daemon"
OBMC_IMAGE_EXTRA_INSTALL:append = " system-watchdog"
OBMC_IMAGE_EXTRA_INSTALL:append = " configuration-manager"
OBMC_IMAGE_EXTRA_INSTALL:append = " configure-firmware-status"
OBMC_IMAGE_EXTRA_INSTALL:append = " mctpd"
OBMC_IMAGE_EXTRA_INSTALL:append = " pldmd"
OBMC_IMAGE_EXTRA_INSTALL:append = " nvme-mi"

