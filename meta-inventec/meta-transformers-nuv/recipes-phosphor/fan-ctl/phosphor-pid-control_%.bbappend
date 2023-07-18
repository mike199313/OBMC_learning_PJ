FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

FILES:${PN}:append = " ${bindir}/fan-default-speed.sh"

SRC_URI:append = " file://fan-default-speed.sh \
                   file://phosphor-pid-control.service \
                   file://0001-ms-spec-of-sensor-reading-fail-condition-and-fail-sa.patch \
                 "

inherit obmc-phosphor-systemd
RDEPENDS:${PN} += "bash"

SYSTEMD_SERVICE:${PN} = "phosphor-pid-control.service"

do_install:append (){
    install -d ${D}/${bindir}
    install -m 0755 ${WORKDIR}/fan-default-speed.sh ${D}/${bindir}
}
