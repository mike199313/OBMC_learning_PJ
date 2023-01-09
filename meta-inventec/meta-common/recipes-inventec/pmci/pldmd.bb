SUMMARY = "PLDM Daemon"
DESCRIPTION = "Implementation of PLDM specifications"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

SRC_URI = "git://github.com/Intel-BMC/pldmd.git;protocol=ssh;branch=main"
SRCREV = "692461084759a72989f8d04d909956f7d33591ed"

S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"

inherit cmake systemd

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

DEPENDS += " \
    cli11 \
    libpldm-intel \
    mctp-wrapper \
    systemd \
    sdbusplus \
    phosphor-logging \
    gtest \
    boost \
    phosphor-dbus-interfaces \
    mctpwplus \
    nlohmann-json \
    "

#FILES:${PN} += "${systemd_system_unitdir}/xyz.openbmc_project.pldmd.service"
#SYSTEMD_SERVICE:${PN} += "xyz.openbmc_project.pldmd.service"
