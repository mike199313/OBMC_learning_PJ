SUMMARY = "MCTP Daemon"
DESCRIPTION = "Implementation of MCTP (DTMF DSP0236)"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

SRC_URI = "git://github.com/Intel-BMC/mctpd.git;protocol=ssh;branch=main"
SRCREV = "da4270c0ba293ddb93391839e031a4e6c900ceba"

S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"

#OECMAKE_SOURCEPATH = "${S}/${PN}"

inherit cmake systemd

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

DEPENDS += " \
    libmctp-intel \
    systemd \
    sdbusplus \
    phosphor-logging \
    boost \
    i2c-tools \
    cli11 \
    nlohmann-json \
    gtest \
    phosphor-dbus-interfaces \
    udev \
    "

SMBUS_BINDING = "smbus"
PCIE_BINDING = "pcie"

FILES:${PN} += "${systemd_system_unitdir}/xyz.openbmc_project.mctpd@.service"
FILES:${PN} += "/usr/share/mctp/mctp_config.json"

SYSTEMD_SERVICE:${PN} += "xyz.openbmc_project.mctpd@${SMBUS_BINDING}.service"
SYSTEMD_SERVICE:${PN} += "xyz.openbmc_project.mctpd@${PCIE_BINDING}.service"
