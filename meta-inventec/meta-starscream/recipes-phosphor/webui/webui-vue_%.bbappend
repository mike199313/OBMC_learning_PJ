FILESEXTRAPATHS:prepend:starscream := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-Bug-1446-Starscream-ast-OpenBMC-WebUI-overview-IPv4-.patch \
"
