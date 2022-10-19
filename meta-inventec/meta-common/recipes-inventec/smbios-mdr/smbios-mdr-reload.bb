SUMMARY = "smbios mdr reload service"
DESCRIPTION = "reloading smbios table if the BIOS complete pin raising."
PR = "r1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit obmc-phosphor-systemd

DEPENDS += "systemd"
RDEPENDS:${PN} += "libsystemd"


FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://smbios-mdr-reload.sh"

S = "${WORKDIR}"

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 smbios-mdr-reload.sh ${D}${sbindir}
}

SYSTEMD_SERVICE:${PN} += "smbios-mdr-reload.service"
