SUMMARY = "configure-firmware-status"
DESCRIPTION = "configure-firmware-status"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESEXTRAPATHS:prepend := "${THISDIR}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

S = "${WORKDIR}"
SRC_URI = "file://meson.build       \
           file://include/configure-firmware-status.hpp \
           file://src/configure-firmware-status.cpp \
           file://src/meson.build \
           file://service_files/configure-firmware-status.service \
           file://service_files/meson.build \
          "

SYSTEMD_SERVICE:${PN} += "configure-firmware-status.service"

DEPENDS = "boost sdbusplus nlohmann-json"
inherit meson systemd pkgconfig
