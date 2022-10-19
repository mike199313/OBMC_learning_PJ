FILESEXTRAPATHS:prepend:transformers := "${THISDIR}/${BPN}:"

SRC_URI:append:transformers = " file://dev_id.json"
SRC_URI:append:transformers = " file://channel_access.json"
SRC_URI:append:transformers = " file://channel_config.json"
SRC_URI:append:transformers = " file://gpiomap.json"


FILES:${PN} += " \
                ${datadir}/ipmi-providers/gpiomap.json \
               "

do_install:append:transformers() {
    install -m 0644 -D ${WORKDIR}/dev_id.json \
        ${D}/usr/share/ipmi-providers/dev_id.json
    install -m 0644 -D ${WORKDIR}/channel_access.json \
        ${D}/usr/share/ipmi-providers/channel_config.json
    install -m 0644 -D ${WORKDIR}/channel_config.json \
        ${D}/usr/share/ipmi-providers/channel_config.json
    install -m 0644 -D ${WORKDIR}/gpiomap.json \
        ${D}/usr/share/ipmi-providers/gpiomap.json
}
