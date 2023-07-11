SUMMARY = "MCTP Wrapper Library"
DESCRIPTION = "Implementation of MCTP Wrapper Library"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=bcd9ada3a943f58551867d72893cc9ab"

SRC_URI = "git://github.com/Intel-BMC/mctp-wrapper.git;protocol=ssh;branch=main"
SRCREV = "a3457c8bcb2ec80ba76be6ca9a7fd96031419d52"

S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"

inherit cmake systemd

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-fix-build-error-for-mctp-wrapper.patch"

DEPENDS += " \
    libmctp-intel \
    systemd \
    sdbusplus \
    phosphor-logging \
    gtest \
    boost \
    phosphor-dbus-interfaces \
    "

EXTRA_OECMAKE += "-DYOCTO_DEPENDENCIES=ON"
