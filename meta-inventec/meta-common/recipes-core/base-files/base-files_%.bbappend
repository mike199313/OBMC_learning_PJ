FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
    file://50-restricted_arp.conf \
"

do_install:append() {
    install -D -m 644 ${WORKDIR}/50-restricted_arp.conf ${D}/${libdir}/sysctl.d/50-restricted_arp.conf
}
