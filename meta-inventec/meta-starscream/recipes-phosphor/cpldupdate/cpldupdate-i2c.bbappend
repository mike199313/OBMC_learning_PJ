FILESEXTRAPATHS:append:starscream := "${THISDIR}/${PN}:"

FILES:${PN}:append = " ${datadir}/cpldupdate-i2c/config.json"

SRC_URI += " file://cpldupdate-i2c.hpp \
             file://config.json \
           "

do_install:append() {
    install -d ${D}${bindir}
    install -m 755 cpldupdate-i2c ${D}${bindir}/cpldupdate-i2c
    install -d 0755 ${D}/usr/share/cpldupdate-i2c
    install -m 0644 ${WORKDIR}/config.json ${D}/usr/share/cpldupdate-i2c/config.json
}

