SUMMARY = "transformers-nuv init service"
DESCRIPTION = "Essential init commands for transformers-nuv"
PR = "r1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit obmc-phosphor-systemd

DEPENDS += "systemd"
RDEPENDS:${PN} += "libsystemd"


FILESEXTRAPATHS:prepend := "${THISDIR}/transformers-nuv-init:"
SRC_URI += "file://transformers-nuv-init.sh \
            file://emmc-check.sh \
            file://transformers-nuv-cpld-init.sh \
            file://transformers-nuv-pcie-scanner.sh \            
            "

S = "${WORKDIR}"

do_install() {
        install -d ${D}${sbindir}
        install -m 0755 transformers-nuv-init.sh ${D}${sbindir}
        install -m 0755 emmc-check.sh ${D}${sbindir}
        install -m 0755 transformers-nuv-cpld-init.sh ${D}${sbindir}
        install -m 0755 transformers-nuv-pcie-scanner.sh ${D}${sbindir}        
}

SYSTEMD_SERVICE:${PN} += "transformers-nuv-init.service"
SYSTEMD_SERVICE:${PN} += "transformers-nuv-pcie-scanner.service"