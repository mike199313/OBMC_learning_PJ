FILESEXTRAPATHS:prepend := "${THISDIR}/file:"


# Change SRCREV to newer version for 2.14, should check the version later then 2.15(in future)
SRCREV = "4e1ba8a736a4272e15f8e4541858407821d6b59e"

SRC_URI:append = " file://0001-Bug-1746-SW-libpldm-Add-Intel-BMC-libpldm-features-t.patch "

