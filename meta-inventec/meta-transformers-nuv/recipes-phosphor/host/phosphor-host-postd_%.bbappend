FILESEXTRAPATHS:prepend:transformers-nuv := "${THISDIR}/${PN}:"

SRC_URI:append:transformers-nuv = " file://0001-add-postcode-portled-service.patch"

inherit obmc-phosphor-systemd
PACKAGECONFIG:remove = "7seg"

DEPENDS += "libgpiod"
SYSTEMD_SERVICE:${PN} += "postcode-portled.service"
