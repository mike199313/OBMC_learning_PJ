FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"


PACKAGECONFIG[psu1] = "-Dpsu1-upgrade=enabled, -Dpsu1-upgrade=disabled"

PACKAGECONFIG:append =" \
            psu1 \
            "

