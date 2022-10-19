FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://0001-Disable-rtti-because-of-boost-version-issue.patch \
"


CFLAGS:append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
CFLAGS:append = " -I ${STAGING_KERNEL_DIR}/include"
CXXFLAGS:append = " -I ${STAGING_KERNEL_DIR}/include/uapi"
CXXFLAGS:append = " -I ${STAGING_KERNEL_DIR}/include"
do_configure[depends] += "virtual/kernel:do_shared_workdir"

