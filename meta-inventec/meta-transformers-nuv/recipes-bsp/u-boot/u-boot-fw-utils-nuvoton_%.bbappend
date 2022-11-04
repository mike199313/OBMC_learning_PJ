FILESEXTRAPATHS:prepend:transformers-nuv := "${THISDIR}/${PN}:"

SRC_URI:append:transformers-nuv = " file://fw_env.config"

do_install:append:transformers-nuv () {
	install -m 644 ${WORKDIR}/fw_env.config ${D}${sysconfdir}/fw_env.config
}
