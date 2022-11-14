FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://starscream-host-s0-set-failsafe.service \
    file://starscream-host-s5-set-failsafe.service \
    file://starscream-host-set-boot-failsafe@.service \
    file://starscream-check-host-state.service \
    file://starscream-set-boot-failsafe.sh \
    file://starscream-states-control.sh \
    file://starscream-check-host-state.sh \
    file://starscream-host-ready.target \
    "

RDEPENDS:${PN}:append = "bash"

CHASSIS_INSTANCE="0"

SYSTEMD_SERVICE:${PN}:append = " \
    starscream-host-s0-set-failsafe.service \
    starscream-host-s5-set-failsafe.service \
    starscream-host-set-boot-failsafe@${CHASSIS_INSTANCE}.service \
    starscream-check-host-state.service \
    starscream-host-ready.target \
    "

FILES:${PN}:append = " \
    ${systemd_system_unitdir}/starscream-host-set-boot-failsafe@.service \
    "

do_install:append() {
    install -d ${D}${bindir}


    install -m 0755 ${WORKDIR}/starscream-states-control.sh ${D}${bindir}/.
    install -m 0755 ${WORKDIR}/starscream-set-boot-failsafe.sh ${D}${bindir}/.
    install -m 0755 ${WORKDIR}/starscream-check-host-state.sh ${D}${bindir}/.

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/starscream-host-s0-set-failsafe.service ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/starscream-host-s5-set-failsafe.service ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/starscream-host-set-boot-failsafe@.service ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/starscream-check-host-state.service ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/starscream-host-ready.target ${D}${systemd_system_unitdir}
}
