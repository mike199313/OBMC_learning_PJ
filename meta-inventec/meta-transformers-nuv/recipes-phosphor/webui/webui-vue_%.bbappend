FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-Bug-1077-Transformers-nuv-OpenBMC-WebUI-overview-IPv.patch \
"
