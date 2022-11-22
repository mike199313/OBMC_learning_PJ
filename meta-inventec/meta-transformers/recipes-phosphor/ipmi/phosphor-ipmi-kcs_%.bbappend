FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://0001-Bug-1361-SW-Common-Interface-Readiness.patch \
                 "

KCS_DEVICE_2 ?= "ipmi-kcs2"

SYSTEMD_SERVICE:${PN} += " ${PN}@${KCS_DEVICE_2}.service "

