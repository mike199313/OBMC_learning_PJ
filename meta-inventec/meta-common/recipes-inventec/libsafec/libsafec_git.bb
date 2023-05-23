SUMMARY = "safe c library"
DESCRIPTION = "safe c library extention"
PR = "r1"
PV = "1.0+git${SRCPV}"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://COPYING;md5=6d0eb7dfc57806a006fcbc4e389cf164"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

DEPENDS += "autoconf-archive-native"
RDEPENDS:${PN} += " \
        perl \
        "


SRC_URI = "git://github.com/rurban/safeclib;branch=master;protocol=https"
SRCREV = "0234bec46da4863f849f100c2f5336412ab2f69b"

PACKAGECONFIG ??= "libsafec"


