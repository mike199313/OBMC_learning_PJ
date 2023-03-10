FILESEXTRAPATHS:prepend:starscream := "${THISDIR}/${BPN}:"

SRC_URI += " \
            file://dev_id.json \
            file://channel_config.json \
            file://channel_access.json \
            file://gpiomap.json \
            file://psu_config.json \
            file://fru_config.json \
           "

FILES:${PN} += " \
                ${datadir}/ipmi-providers/dev_id.json \
                ${datadir}/ipmi-providers/channel_config.json \
                ${datadir}/ipmi-providers/channel_access.json \
                ${datadir}/ipmi-providers/gpiomap.json \ 
                ${datadir}/ipmi-providers/psu_config.json \
                ${datadir}/ipmi-providers/fru_config.json \ 
                "

do_install:append() {
    install -m 0644 -D ${WORKDIR}/dev_id.json \
        ${D}/usr/share/ipmi-providers/dev_id.json
    install -m 0644 -D ${WORKDIR}/channel_config.json \
        ${D}/usr/share/ipmi-providers/channel_config.json
    install -m 0644 -D ${WORKDIR}/channel_access.json \
        ${D}/usr/share/ipmi-providers/channel_access.json
    install -m 0644 -D ${WORKDIR}/gpiomap.json \
        ${D}/usr/share/ipmi-providers/gpiomap.json
    install -m 0644 -D ${WORKDIR}/psu_config.json \
        ${D}/usr/share/ipmi-providers/psu_config.json
    install -m 0644 -D ${WORKDIR}/fru_config.json \
        ${D}/usr/share/ipmi-providers/fru_config.json
}
