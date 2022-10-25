FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://phosphor-multi-gpio-monitor.json"

do_install:append(){
    install -d 0755 ${D}/usr/share/phosphor-gpio-monitor
    install -m 0644 ${WORKDIR}/phosphor-multi-gpio-monitor.json ${D}/usr/share/phosphor-gpio-monitor/phosphor-multi-gpio-monitor.json
}
