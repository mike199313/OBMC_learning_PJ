SUMMARY = "Inventec PSU update service"
DESCRIPTION = "Post inventory data to dbus interface"
PR = "r1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit obmc-phosphor-systemd

DEPENDS += "systemd"
RDEPENDS:${PN} += "libsystemd"


FILESEXTRAPATHS:prepend := "${THISDIR}/inv-psu-update:"
SRC_URI += "file://inv-psu-update.sh"

S = "${WORKDIR}"

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 inv-psu-update.sh ${D}${sbindir}
}

SYSTEMD_SERVICE:${PN} += "inv-psu-update@.service"
