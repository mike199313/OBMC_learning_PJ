SUMMARY = "libpldm_intel"
DESCRIPTION = "Provides encode/decode APIs for PLDM specifications"

#include pmci.inc

SRC_URI = "git://git@github.com/Intel-BMC/libpldm.git;protocol=ssh;branch=main"
SRCREV = "cf792b06a27f308a888c4bbf5cb5f8b90fa18d22"

S = "${WORKDIR}/git"

PV = "1.0+git${SRCPV}"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"

inherit cmake

SRC_URI += "file://0001-Add-new-definition-for-Bits-sensor.patch"
SRC_URI += "file://0002-Patch-for-PLDM-firmware-update.patch"

DEPENDS += " \
    gtest \
    "
