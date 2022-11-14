SUMMARY = "dimm-plug@.service"
DESCRIPTION = "do dimm plug service when bios post comepletes"
PR = "r1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit obmc-phosphor-systemd

DEPENDS += "systemd"
RDEPENDS:${PN} += "libsystemd"


FILESEXTRAPATHS:prepend := "${THISDIR}/dimm-plug:"
SRC_URI += "file://dimm-plug.sh"

S = "${WORKDIR}"

do_install() {
        install -d ${D}${bindir}
        install -m 0755 dimm-plug.sh ${D}${bindir}
}

SYSTEMD_SERVICE:${PN} += "dimm-plug@.service"
