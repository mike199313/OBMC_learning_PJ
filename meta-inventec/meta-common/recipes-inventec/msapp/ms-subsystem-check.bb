DESCRIPTION = "Inventec MS subsystem sensors checking"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESEXTRAPATHS:prepend := "${THISDIR}:"

inherit cmake pkgconfig obmc-phosphor-systemd

DEPENDS += "boost \
                phosphor-ipmi-host \
                phosphor-dbus-interfaces \
                phosphor-logging \
                systemd \
                sdbusplus \
                nlohmann-json \
                i2c-tools"

S = "${WORKDIR}/${BPN}"

SRC_URI = "file://${BPN}/CMakeLists.txt             \
           file://${BPN}/src/ms-subsystem-check.cpp           \
           file://${BPN}/src/Utils.cpp           \
           file://${BPN}/src/i2cbusses.c           \
           file://${BPN}/include/Utils.hpp           \
           file://${BPN}/include/i2cbusses.h           \
           file://${BPN}/include/VariantVisitors.hpp           \
           file://${BPN}/xyz.openbmc_project.ms-subsystem-check.service   \
"

SYSTEMD_SERVICE:${BPN} += "xyz.openbmc_project.ms-subsystem-check.service"

EXTRA_OECMAKE=""

