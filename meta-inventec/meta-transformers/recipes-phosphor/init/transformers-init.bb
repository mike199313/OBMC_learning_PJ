SUMMARY = "transformers init service"
DESCRIPTION = "Essential init commands for transformers"
PR = "r1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit obmc-phosphor-systemd

DEPENDS += "systemd"
RDEPENDS:${PN} += "libsystemd"


FILESEXTRAPATHS:prepend := "${THISDIR}/transformers-init:"
SRC_URI = "\
            file://transformers-init.sh \
            file://transformers-watchdog-init.sh \
            file://transformers-post-init.sh \
            file://emmc-check.sh \
            file://transformers-cpld-init.sh \
          "

S = "${WORKDIR}"

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 transformers-init.sh ${D}${sbindir}
        install -m 0755 transformers-watchdog-init.sh ${D}${sbindir}
        install -m 0755 transformers-post-init.sh ${D}${sbindir}
        install -m 0755 emmc-check.sh ${D}${sbindir}
        install -m 0755 transformers-cpld-init.sh ${D}${sbindir}
}

SYSTEMD_SERVICE:${PN} += "transformers-init.service"
SYSTEMD_SERVICE:${PN} += "transformers-watchdog-init.service"
SYSTEMD_SERVICE:${PN} += "transformers-post-init.service"
