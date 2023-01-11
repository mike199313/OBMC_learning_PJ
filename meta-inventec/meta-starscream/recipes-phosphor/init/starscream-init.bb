SUMMARY = "starscream init service"
DESCRIPTION = "Essential init commands for starscream"
PR = "r1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit obmc-phosphor-systemd

DEPENDS += "systemd"
RDEPENDS:${PN} += "libsystemd"


FILESEXTRAPATHS:prepend := "${THISDIR}/starscream-init:"
SRC_URI += "file://starscream-init.sh \
            file://starscream-config-init.sh \
            file://starscream-gpio-init.sh \
            file://starscream-cpld-init.sh \
            file://create_json.sh \
            file://emmc-check.sh \
"

S = "${WORKDIR}"

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 starscream-init.sh ${D}${sbindir}
        install -m 0755 starscream-config-init.sh ${D}${sbindir}
        install -m 0755 starscream-gpio-init.sh ${D}${sbindir}
        install -m 0755 starscream-cpld-init.sh ${D}${sbindir}
        install -m 0755 create_json.sh ${D}${sbindir}
        install -m 0755 emmc-check.sh ${D}${sbindir}
}

SYSTEMD_SERVICE:${PN} += "starscream-init.service"
SYSTEMD_SERVICE:${PN} += "starscream-config-init.service"
SYSTEMD_SERVICE:${PN} += "starscream-gpio-init.service"
