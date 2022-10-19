SUMMARY = "Intel OEM IPMI commands"
DESCRIPTION = "Intel OEM IPMI commands"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=a6a4edad4aed50f39a66d098d74b265b"

SRC_URI = "git://github.com/openbmc/intel-ipmi-oem;branch=master;protocol=https"
SRCREV = "a165038f0472459ae2ec0ae50b7e0c09969882c7"

S = "${WORKDIR}/git"
PV = "0.1+git${SRCPV}"

DEPENDS = "boost phosphor-ipmi-host phosphor-logging systemd intel-dbus-interfaces libgpiod"
DEPENDS += " phosphor-snmp"

inherit cmake obmc-phosphor-ipmiprovider-symlink pkgconfig

EXTRA_OECMAKE="-DENABLE_TEST=0 -DYOCTO=1"

LIBRARY_NAMES = "libzinteloemcmds.so"

HOSTIPMI_PROVIDER_LIBRARY += "${LIBRARY_NAMES}"
NETIPMI_PROVIDER_LIBRARY += "${LIBRARY_NAMES}"

FILES:${PN}:append = " ${libdir}/ipmid-providers/lib*${SOLIBS}"
FILES:${PN}:append = " ${libdir}/host-ipmid/lib*${SOLIBS}"
FILES:${PN}:append = " ${libdir}/net-ipmid/lib*${SOLIBS}"
FILES:${PN}-dev:append = " ${libdir}/ipmid-providers/lib*${SOLIBSDEV}"

do_install:append(){
   install -d ${D}${includedir}/intel-ipmi-oem
   install -m 0644 -D ${S}/include/*.hpp ${D}${includedir}/intel-ipmi-oem
}

FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

SRC_URI:append += " file://0001-Removed-Get-Device-ID-command.patch \
                    file://0002-Remove-Intel-IPMI-OEM-commands.patch \
                    file://0003-SEL-Implement-Set-SEL-Time-command.patch  \
                    file://0004-SEL-Modified-IPMI-command-Add-Get-SEL-Entry.patch \
                    file://0005-SEL-Modified-Platform-Event-command-to-log-message-t.patch \
                    file://0006-dont-register-mfg-filters.patch \
                    file://0007-FRU-Command-modify.patch \
                    file://0008-disable-whitelist-filter.patch \
                    file://0009-SDR-Patches-for-SDR.patch \
                    file://0010-Bug-755-SW-common-intel-ipmi-oem-Add-more-sensor-typ.patch \
                    file://0011-Implement-get-set-system-boot-option.patch \
                    file://0012-The-main-implementation-of-WARM-RESET.patch \
                    file://0013-Sensor-Fix-set-sensor-threshold-command-fail-issue.patch \
                    file://0014-Bug-629-SW-Transformers-OpenBMC-Support-mc-selftest-.patch \
                    file://0015-Implement-Get-system-GUID-function.patch \
                    file://0016-Bridging-Implement-Get-Set-global-enables-and-modify.patch \
                    file://0017-Bug623-SW-Transformers-OpenBMC-IPMI-Implement-get-ch.patch \
                    file://0018-Implement-Get-Sensor-Reading-Factors-Command.patch \
                    file://0019-Add-Get-Sensor-Type-command-support.patch \
                    file://0020-Bug-876-SW-Common-Intel-ipmi-oem-Remove-sensors-has-.patch \
                    file://0021-Common-SDR-related-commands-implementation-and-modif.patch \
                    file://0022-MS-M-BMC-LOG-0008-bmcreset.patch \
                    file://0023-Implement-PEF-features.patch \
                    file://0024-Implement-SEL-timestamp-correction.patch \
                    file://0025-get-sensor-enable-after-setting.patch \
                  "
