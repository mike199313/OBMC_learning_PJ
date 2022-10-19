inherit obmc-phosphor-systemd

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

RDEPENDS:${PN}:append = " openssl-bin bash"

PACKAGECONFIG[ubitar-bmc] = ""

SRC_URI:append += " file://0001-Remove-extra-in-config-static-bmc-reboot.json.in-to-.patch \
		"
