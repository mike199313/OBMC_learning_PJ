inherit obmc-phosphor-systemd

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

RDEPENDS:${PN}:append = " openssl-bin bash"


PACKAGECONFIG:append = " reboot-update static-bmc net-bridge aspeed-p2a"

SRC_URI:append  = " file://verify_image.sh \
                  "

SYSTEMD_SERVICE:${PN} += " verify-image.service"

EXTRA_OECONF:append = " CPPFLAGS='-DENABLE_PCI_BRIDGE'"


do_install:append() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/verify_image.sh ${D}${bindir}
}
