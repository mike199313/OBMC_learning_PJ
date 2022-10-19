SUMMARY = "bios update"
DESCRIPTION = "Command line tool to update bios firmware"
PR = "r1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

#inherit obmc-phosphor-systemd

TARGET_CC_ARCH += "${LDFLAGS}"

S = "${WORKDIR}"

DEPENDS += "systemd sdbusplus obmc-libi2c libgpiod phosphor-dbus-interfaces"
RDEPENDS:${PN} += "libsystemd sdbusplus obmc-libi2c libgpiod"

SRC_URI = "file://Makefile \
           file://main.cpp \
           file://bios-update.hpp \
           file://bios-update.cpp \
           file://configure-firmware-update.hpp \
           file://configure-firmware-update.cpp \
           file://COPYING.MIT \
          "
do_install() {
    install -d ${D}${sbindir}
    install -Dm755 ${S}/bios-update ${D}${sbindir}/bios-update
}
