DESCRIPTION = "Inventec utilities"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESEXTRAPATHS:prepend := "${THISDIR}:"

inherit cmake obmc-phosphor-systemd externalsrc

DEPENDS += "systemd sdbusplus libgpiod phosphor-dbus-interfaces"
RDEPENDS:${PN} += "libsystemd sdbusplus libgpiod"

S = "${WORKDIR}/${BPN}"
EXTERNALSRC_SYMLINKS = ""
EXTERNALSRC = "${THISDIR}/${PN}"
EXTERNALSRC_BUILD = "${B}"

SYSTEMD_SERVICE:${BPN} += "mac_util_config@.service"

SYSTEMD_SERVICE:${BPN} += "eeprom-manager.service"

SYSTEMD_SERVICE:${BPN} += "post-bootup.service pre-shutdown.service"

EXTRA_OECMAKE=""
