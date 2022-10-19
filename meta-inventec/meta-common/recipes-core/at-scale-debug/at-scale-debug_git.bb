# Inventec copy from
# Intel-BMC/openbmc/blob/intel/meta-openbmc-mods/meta-common/recipes-core/at-scale-debug/at-scale-debug_git.bb
inherit obmc-phosphor-systemd

SUMMARY = "At Scale Debug Service"
DESCRIPTION = "At Scale Debug Service exposes remote JTAG target debug capabilities"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8929d33c051277ca2294fe0f5b062f38"


inherit cmake
# Inventec - Modify libsafec for build issue
#DEPENDS = "sdbusplus openssl libpam libgpiod safec"
DEPENDS = "sdbusplus openssl libpam libgpiod libsafec"

do_configure[depends] += "virtual/kernel:do_shared_workdir"

SRC_URI = "git://github.com/Intel-BMC/asd;protocol=git"
SRCREV = "37997e3fde81dc118f9431a49c673cfdaf443bdb"

# Inventec - Markdown useradd for build issue
#inherit useradd

#USERADD_PACKAGES = "${PN}"

# add a special user asdbg
#USERADD_PARAM:${PN} = "-u 999 asdbg"

S = "${WORKDIR}/git"

SYSTEMD_SERVICE:${PN} += "com.intel.AtScaleDebug.service"

# Specify any options you want to pass to cmake using EXTRA_OECMAKE:
EXTRA_OECMAKE = "-DBUILD_UT=OFF"

CFLAGS:append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
CFLAGS:append = " -I ${STAGING_KERNEL_DIR}/include"

# Copying the depricated header from kernel as a temporary fix to resolve build breaks.
# It should be removed later after fixing the header dependency in this repository.
SRC_URI += "file://asm/rwonce.h"
do_configure:prepend() {
    cp -r ${WORKDIR}/asm ${S}/asm
}
CFLAGS:append = " -I ${S}"
