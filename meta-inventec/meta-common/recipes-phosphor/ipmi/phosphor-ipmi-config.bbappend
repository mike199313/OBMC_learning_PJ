FILESEXTRAPATHS:prepend := "${THISDIR}/${BPN}:"

SRC_URI += " \
            file://lan_config.json \
            file://sys_info.json \
            file://warm_reset.json \
	    file://system_config_current.json \
	    file://ms_mediaredirect.json \
	    file://cipher_list.json \
           "

FILES:${PN} += " \
                ${datadir}/ipmi-providers/lan_config.json \
                ${datadir}/ipmi-providers/sys_info.json \
                ${datadir}/ipmi-providers/warm_reset.json \
                ${datadir}/ipmi-providers/system_config_current.json \
                ${datadir}/ipmi-providers/ms_mediaredirect.json \
                ${datadir}/ipmi-providers/cipher_list.json \
               "

do_install:append() {
    install -m 0644 -D ${WORKDIR}/lan_config.json \
        ${D}/usr/share/ipmi-providers/lan_config.json
    install -m 0644 -D ${WORKDIR}/sys_info.json \
        ${D}/usr/share/ipmi-providers/sys_info.json
    install -m 0644 -D ${WORKDIR}/warm_reset.json \
        ${D}/usr/share/ipmi-providers/warm_reset.json
    install -m 0644 -D ${WORKDIR}/system_config_current.json \
        ${D}/usr/share/ipmi-providers/system_config_current.json
    install -m 0644 -D ${WORKDIR}/ms_mediaredirect.json \
        ${D}/usr/share/ipmi-providers/ms_mediaredirect.json
    install -m 0644 -D ${WORKDIR}/cipher_list.json \
        ${D}/usr/share/ipmi-providers/cipher_list.json
}
