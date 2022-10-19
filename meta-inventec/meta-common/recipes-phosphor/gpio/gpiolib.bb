DESCRIPTION = "Inventec GPIO library"
PR = "r1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

inherit systemd

HASHSTYLE = "gnu"

DEPENDS += "boost"
DEPENDS += "systemd"

RDEPENDS:${PN} += "libsystemd"
RDEPENDS:${PN} += "bash"

FILESEXTRAPATHS:append := "${THISDIR}:"
S = "${WORKDIR}/${BPN}"
SRC_URI = "file://${BPN}/gpiolib.hpp \
           file://${BPN}/gpioutil \
          "


do_compile() {
    ${CXX} -nostartfiles -Wl,-hash-style=${HASHSTYLE} -o gpiolib ${S}/gpiolib.hpp
}

do_install() {
    install -d ${D}${sbindir}
    install -m 0755 ${S}/gpiolib ${D}${sbindir}
    install -m 0755 ${S}/gpioutil ${D}${sbindir}
    install -d ${D}${includedir}
    install -m 0755 ${S}/gpiolib.hpp ${D}${includedir}/
}
