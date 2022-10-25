FILESEXTRAPATHS:prepend:transformers-nuv := "${THISDIR}/${PN}:"

SRC_URI:append:transformers-nuv = " file://80-buv-runbmc-sol.rules"

do_install:append:transformers-nuv() {
        install -m 0755 -d ${D}${sysconfdir}/${BPN}
        rm -f ${D}${sysconfdir}/${BPN}/server.ttyVUART0.conf
        install -m 0644 ${WORKDIR}/${BPN}.conf ${D}${sysconfdir}/
        ln -sr ${D}${sysconfdir}/${BPN}.conf ${D}${sysconfdir}/${BPN}/server.ttyS1.conf

        install -d ${D}/lib/udev/rules.d
        rm -f ${D}/lib/udev/rules.d/80-obmc-console-uart.rules
        install -m 0644 ${WORKDIR}/80-buv-runbmc-sol.rules ${D}/lib/udev/rules.d
}
