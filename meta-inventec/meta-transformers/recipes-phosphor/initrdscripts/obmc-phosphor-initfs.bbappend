FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"


do_install:append() {
    install -m 0755 ${WORKDIR}/obmc-init.sh ${D}/init
}
