SUMMARY = "gpio-event-handler"
DESCRIPTION = "gpio-event-handler"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"
SYSTEMD_AUTO_ENABLE = "enable"

inherit obmc-phosphor-systemd

DEPENDS += "systemd sdbusplus libgpiod phosphor-dbus-interfaces"
RDEPENDS:${PN} += "libsystemd sdbusplus libgpiod"

S = "${WORKDIR}"
SRC_URI = "file://Makefile \
	   file://gpio-event-handler.cpp \
	   file://gpio-event-handler@.service \
	  "
TARGET_CC_ARCH += "${LDFLAGS}"

do_install(){
	install -d ${D}${bindir}
	install -m 0755 gpio-event-handler ${D}${bindir}
}

SYSTEMD_SERVICE:${PN} += "gpio-event-handler@.service"
