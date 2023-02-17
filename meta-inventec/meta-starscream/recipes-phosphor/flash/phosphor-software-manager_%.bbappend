FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

#add psu service
SYSTEMD_SERVICE:${PN}-updater += " \
   obmc-psu-update@.service \
"


PACKAGECONFIG[psu1] = "-Dpsu1-upgrade=enabled, -Dpsu1-upgrade=disabled"
PACKAGECONFIG[psu2] = "-Dpsu2-upgrade=enabled, -Dpsu2-upgrade=disabled"
PACKAGECONFIG[dps2400eb] = "-Ddps2400eb=enabled, -Ddps2400eb=disabled"

PACKAGECONFIG:append =" \
            psu1 \
            psu2 \
	    dps2400eb \
            "

