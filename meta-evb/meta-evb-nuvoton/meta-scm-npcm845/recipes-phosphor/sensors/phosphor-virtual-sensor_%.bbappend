FILESEXTRAPATHS:prepend:scm-npcm845 := "${THISDIR}/${PN}:"

SRC_URI:append:scm-npcm845 = " file://config-virtual-sensor.json \
                     "

RDEPENDS:${PN}:append:scm-npcm845 = "bash"

do_install:append:scm-npcm845() {
    install -d ${D}${datadir}/${PN}
    install -m 0644 -D ${WORKDIR}/config-virtual-sensor.json \
        ${D}${datadir}/${PN}/virtual_sensor_config.json
}
