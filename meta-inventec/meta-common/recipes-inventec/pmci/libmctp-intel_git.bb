SUMMARY = "libmctp:intel"
DESCRIPTION = "Implementation of MCTP(DMTF DSP0236)"

SRC_URI = "git://github.com/Intel-BMC/libmctp.git;protocol=ssh;branch=master"
SRCREV = "21dc38e911a27af2e914f834b2e2b775f7dad520"

S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"

LICENSE = "Apache-2.0 | GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=0d30807bb7a4f16d36e96b78f9ed8fae"

inherit cmake

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://0001-fix-the-build-error-on-lpc-and-i3c-test-procedure.patch"

DEPENDS += "i2c-tools"
