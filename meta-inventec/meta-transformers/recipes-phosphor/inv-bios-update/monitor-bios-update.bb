SUMMARY = "monitor-bios-update@.service"
DESCRIPTION = "monitor bios update prgress and report to host"
PR = "r1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit obmc-phosphor-systemd

DEPENDS += "systemd"
RDEPENDS:${PN} += "libsystemd"


FILESEXTRAPATHS:prepend := "${THISDIR}/monitor-bios-update:"
SRC_URI += "file://monitor-bios-update.sh"

S = "${WORKDIR}"

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 monitor-bios-update.sh ${D}${sbindir}
}

SYSTEMD_SERVICE:${PN} += "monitor-bios-update@.service"
