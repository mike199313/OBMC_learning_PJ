FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://wait_interface.conf \
"

FILES:${PN} += "${sysconfdir}/systemd/wait_interface.conf"

do_install:append() {
    install -d ${D}${sysconfdir}/systemd
    install -m 0644 ${WORKDIR}/wait_interface.conf ${D}${sysconfdir}/systemd/wait_interface.conf
}
