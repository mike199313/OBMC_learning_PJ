FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit obmc-phosphor-systemd

SRC_URI:append = " \
    file://0001-Implement-IPMB-time-sync-mechanism.patch \
"

SYSTEMD_SERVICE:${PN} += "ipmb-time-sync.service"
