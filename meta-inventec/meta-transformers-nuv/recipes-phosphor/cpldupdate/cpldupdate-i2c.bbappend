FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

FILES:${PN}:append = " ${datadir}/cpldupdate-i2c/config.json"

SRC_URI += " file://config.json \
           "

do_install:append() {
    install -d 0755 ${D}/usr/share/cpldupdate-i2c
    install -m 0644 ${WORKDIR}/config.json ${D}/usr/share/cpldupdate-i2c/config.json
}

