FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

KCS_DEVICE_2 ?= "ipmi-kcs2"

SYSTEMD_SERVICE:${PN} += " ${PN}@${KCS_DEVICE_2}.service "

