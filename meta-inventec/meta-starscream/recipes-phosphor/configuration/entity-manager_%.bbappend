FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

DISTRO_FEATURES += "ipmi-fru"


SRC_URI:append =  " file://blacklist.json \
                    file://motherboard.json \
                    file://runbmc.json \
                    file://scmbridge.json \
                    file://ocp_addr_80.json \
                    file://ocp_addr_81.json \
                    file://event.json \
                    file://pid.json \
                    file://pid_rmc.json \
                  "

do_install:append() {
    install -d 0755 ${D}/usr/share/entity-manager/configurations
    rm  -rf ${D}/usr/share/entity-manager/configurations/*.json
    install -m 0644 ${WORKDIR}/blacklist.json ${D}/usr/share/entity-manager/blacklist.json
    install -m 0644 ${WORKDIR}/motherboard.json ${D}/usr/share/entity-manager/configurations
    install -m 0644 ${WORKDIR}/runbmc.json ${D}/usr/share/entity-manager/configurations
    install -m 0644 ${WORKDIR}/scmbridge.json ${D}/usr/share/entity-manager/configurations
    install -m 0644 ${WORKDIR}/ocp_addr_80.json ${D}/usr/share/entity-manager/configurations
    install -m 0644 ${WORKDIR}/ocp_addr_81.json ${D}/usr/share/entity-manager/configurations
    install -m 0644 ${WORKDIR}/event.json ${D}/usr/share/entity-manager/configurations
    install -m 0644 ${WORKDIR}/pid.json ${D}/usr/share/entity-manager/configurations
    install -m 0644 ${WORKDIR}/pid_rmc.json ${D}/usr/share/entity-manager/configurations
}

