SUMMARY = "Inventec GPIO library"
DESCRIPTION = "Inventec GPIO library"
PR = "r0"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESEXTRAPATHS:prepend := "${THISDIR}:"

S = "${WORKDIR}/${BPN}"

SRC_URI = "file://${BPN}/CMakeLists.txt                   \
           file://${BPN}/Configuration/gpio_defs_inv.json \
           file://${BPN}/include/invgpiolib.hpp           \
           file://${BPN}/README                           \
           file://${BPN}/src/gpioconf.cpp                 \
           file://${BPN}/src/invgpiolib.cpp               \
"


inherit cmake

DEPENDS = " \
      boost \
      libgpiod \
      nlohmann-json \
   "

EXTRA_OECMAKE=""

LIBRARY_NAMES = "libinvgpiolib.so"
