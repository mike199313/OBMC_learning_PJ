FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

SRC_URI += "file://phosphor-multi-gpio-monitor.json"
#SRC_URI += "file://phosphor-multi-gpio-monitor.service"

do_install:append(){
    install -d ${D}/usr/share/phosphor-gpio-monitor
    install -m 0444 ${WORKDIR}/phosphor-multi-gpio-monitor.json ${D}/usr/share/phosphor-gpio-monitor/phosphor-multi-gpio-monitor.json
}
