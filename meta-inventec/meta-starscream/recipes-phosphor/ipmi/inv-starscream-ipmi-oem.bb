SUMMARY = "INV IPMI platform OEM commands"
DESCRIPTION = "INV IPMI platform OEM commands"
PR = "r0"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESEXTRAPATHS:prepend := "${THISDIR}:"
SRC_URI = "file://${BPN}/CMakeLists.txt           \
           file://${BPN}/include/commandutils.hpp \
           file://${BPN}/include/oemcommands.hpp  \
           file://${BPN}/LICENSE                  \
           file://${BPN}/src/oemcommands.cpp      \
"

S = "${WORKDIR}/${BPN}"

DEPENDS = "boost phosphor-ipmi-host phosphor-logging systemd sdbusplus libpeci libgpiod"
DEPENDS += "nlohmann-json"

inherit cmake obmc-phosphor-ipmiprovider-symlink

EXTRA_OECMAKE=""

LIBRARY_NAMES = "libinvstarscreamoemcmds.so"

HOSTIPMI_PROVIDER_LIBRARY += "${LIBRARY_NAMES}"
NETIPMI_PROVIDER_LIBRARY += "${LIBRARY_NAMES}"

FILES:${PN}:append = " ${libdir}/ipmid-providers/lib*${SOLIBS}"
FILES:${PN}:append = " ${libdir}/host-ipmid/lib*${SOLIBS}"
FILES:${PN}:append = " ${libdir}/net-ipmid/lib*${SOLIBS}"
FILES:${PN}-dev:append = " ${libdir}/ipmid-providers/lib*${SOLIBSDEV}"

#linux-libc-headers guides this way to include custom uapi headers
CFLAGS:append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
CFLAGS:append = " -I ${STAGING_KERNEL_DIR}/include"
CXXFLAGS:append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
CXXFLAGS:append = " -I ${STAGING_KERNEL_DIR}/include"
do_configure[depends] += "virtual/kernel:do_shared_workdir"

do_install:append(){
   install -d ${D}${includedir}/inv-starscream-ipmi-oem
   install -m 0644 -D ${S}/include/*.hpp ${D}${includedir}/inv-starscream-ipmi-oem
}
