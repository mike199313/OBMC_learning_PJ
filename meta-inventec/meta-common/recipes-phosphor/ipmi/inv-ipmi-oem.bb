SUMMARY = "INV IPMI OEM commands"
DESCRIPTION = "INV IPMI OEM commands"
PR = "r0"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

S = "${WORKDIR}/${BPN}"

DEPENDS = "boost phosphor-ipmi-host phosphor-logging systemd sdbusplus libpeci "
DEPENDS += "nlohmann-json valijson"

inherit cmake pkgconfig externalsrc obmc-phosphor-ipmiprovider-symlink
EXTERNALSRC_SYMLINKS = ""
EXTERNALSRC = "${THISDIR}/${PN}"
EXTERNALSRC_BUILD = "${B}"
LIBRARY_NAMES = "libinvoemcmds.so"

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

PACKAGECONFIG[bios-oem] = "-DWITH_BIOS_OEM_CMD=ON,-DWITH_BIOS_OEM_CMD=OFF,"
PACKAGECONFIG[ast] = "-DWITH_AST_BMC=ON,-DWITH_AST_BMC=OFF,"
PACKAGECONFIG[nuv] = "-DWITH_NUV_BMC=ON,-DWITH_NUV_BMC=OFF,"

PACKAGECONFIG ??= ""

FILES:${PN} += "/usr/bin/*"
SRC_URI:append += " file://ms_mediaredirect.sh \
           "
do_install:append(){
   install -d ${D}/usr/bin
   install -m 0755 ${WORKDIR}/ms_mediaredirect.sh ${D}/usr/bin/ms_mediaredirect.sh
   install -d ${D}${includedir}/inv-ipmi-oem
   install -m 0644 -D ${S}/include/*.hpp ${D}${includedir}/inv-ipmi-oem
}
