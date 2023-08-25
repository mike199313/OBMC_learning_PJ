FILESEXTRAPATHS:prepend := "${THISDIR}/file:"


# Change SRCREV to newer version for 2.14, should check the version later then 2.15(in future)
SRCREV = "4e1ba8a736a4272e15f8e4541858407821d6b59e"


SRC_URI:append = " file://0001-Add-encoders-and-decoders-for-RDE-support.patch "
SRC_URI:append = " file://0002-Add-support-for-RDE-Discovery-Negotiate-and-Medium-p.patch "
SRC_URI:append = " file://0003-Bug-1746-SW-libpldm-Add-Intel-BMC-libpldm-features-t.patch "
SRC_URI:append = " file://0004-Bug-1756-SW-libpldm-Fix-build-error-for-applying-goo.patch "
SRC_URI:append = " file://0005-Bug-1803-SW-libpldm-Enable-pldm-RDE-functions.patch "
