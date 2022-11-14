SUMMARY = "Service to handle post-complete"
DESCRIPTION = "Do things that bios post complete raise and fall"
PR = "r1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit obmc-phosphor-systemd

DEPENDS += "systemd"
RDEPENDS:${PN} += "libsystemd"


FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://post-complete.sh"

S = "${WORKDIR}"

do_install() {
        install -d ${D}${bindir}
        install -m 0755 post-complete.sh ${D}${bindir}
}

SYSTEMD_SERVICE:${PN} += "post-complete@.service"
