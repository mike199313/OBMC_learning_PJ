SUMMARY = "MCTP Wrapper Library Plus"
DESCRIPTION = "Implementation of MCTP Wrapper Library Plus"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=615045c30a05cde5c0e924854d43c327"

SRC_URI = "git://github.com/Intel-BMC/mctpwplus.git;protocol=ssh;branch=main"
SRCREV = "2f525a71e7d35997cecc133481828f9c4efa506f"

S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"

inherit meson pkgconfig

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Fix-the-findBusByBindingType-error-and-core-dump.patch"

DEPENDS += " \
    boost \
    systemd \
    sdbusplus \
    phosphor-logging \
    cli11 \
    "
