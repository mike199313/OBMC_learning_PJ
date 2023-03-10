FILESEXTRAPATHS:prepend:transformers-nuv := "${THISDIR}/${BPN}:"

SRC_URI:append:transformers-nuv = " file://dev_id.json"
SRC_URI:append:transformers-nuv = " file://channel_access.json"
SRC_URI:append:transformers-nuv = " file://channel_config.json"
SRC_URI:append:transformers-nuv = " file://gpiomap.json"
SRC_URI:append:transformers-nuv = " file://psu_config.json"
SRC_URI:append:transformers-nuv = " file://fru_config.json"



FILES:${PN} += " \
                ${datadir}/ipmi-providers/gpiomap.json \
                ${datadir}/ipmi-providers/psu_config.json \
                ${datadir}/ipmi-providers/fru_config.json \
               "

do_install:append:transformers-nuv() {
    install -m 0644 -D ${WORKDIR}/dev_id.json \
        ${D}/usr/share/ipmi-providers/dev_id.json
    install -m 0644 -D ${WORKDIR}/channel_access.json \
        ${D}/usr/share/ipmi-providers/channel_config.json
    install -m 0644 -D ${WORKDIR}/channel_config.json \
        ${D}/usr/share/ipmi-providers/channel_config.json
    install -m 0644 -D ${WORKDIR}/gpiomap.json \
        ${D}/usr/share/ipmi-providers/gpiomap.json
    install -m 0644 -D ${WORKDIR}/psu_config.json \
        ${D}/usr/share/ipmi-providers/psu_config.json


}
