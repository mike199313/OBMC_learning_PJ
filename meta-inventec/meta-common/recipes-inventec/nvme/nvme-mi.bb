SUMMARY = "InventecNVMe-MI Daemon"
DESCRIPTION = "Implementation of NVMe-MI v1.0a specifications from Intel's implementation"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

SRC_URI = "git://github.com/Intel-BMC/nvme-mi.git;protocol=https;branch=master"
SRCREV = "b6f50e04516962a4e94fe9340251999f154197c4"

S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"

inherit meson pkgconfig systemd

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

DEPENDS += " \
    systemd \
    sdbusplus \
    phosphor-logging \
    boost \
    nlohmann-json \
    googletest \
    mctpwplus \
    mctpd \
    "

EXTRA_OEMSON = "-Dyocto_dep='enabled'"

FILES:${PN} += "${systemd_system_unitdir}/xyz.openbmc_project.nvme-mi.service"

SYSTEMD_SERVICE:${PN} += "xyz.openbmc_project.nvme-mi.service"

