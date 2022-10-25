FILESEXTRAPATHS:prepend:transformers-nuv := "${THISDIR}/${PN}:"
SRC_URI:append:transformers-nuv = " file://busybox.cfg"
SRC_URI:append:transformers-nuv = "${@bb.utils.contains('DISTRO_FEATURES', 'buv-dev', ' file://buv-dev.cfg', '', d)}"
