FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

FILES:${PN}:append = " ${datadir}/swampd/config.json"
FILES:${PN}:append = " ${bindir}/fan-default-speed.sh"

SRC_URI:append = " file://config.json \
                   file://fan-default-speed.sh \
                   file://phosphor-pid-control.service \
                   file://0001-add-etc-adaptive-pid-control-algorithm.patch \
                   file://0002-ms-spec-of-sensor-reading-fail-condition-and-fail-sa.patch \
                 "

inherit obmc-phosphor-systemd
RDEPENDS:${PN} += "bash"

SYSTEMD_SERVICE:${PN} = "phosphor-pid-control.service"

do_install:append (){
    install -m 0755 -D ${WORKDIR}/config.json \
                   ${D}/usr/share/swampd/config.json
    install -d ${D}/${bindir}
    install -m 0755 ${WORKDIR}/fan-default-speed.sh ${D}/${bindir}
}
