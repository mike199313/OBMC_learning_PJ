FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
            file://0001-Fix-type-mismatch-when-calling-sendTrap.patch \
"
