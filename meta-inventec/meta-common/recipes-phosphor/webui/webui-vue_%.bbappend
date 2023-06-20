FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-Bug1059-1060-1062-Transformers-OpenBMC-WebUI-status-.patch \
    file://0002-Bug-1122-1123-Transformers-Transformers-nuv-OpenBMC-.patch \
    file://0003-bug-1073-1074-Transformers-OpenBMC-WebUI-log-error-m.patch \
    file://0004-Bug1120-1121-Common-OpenBMC-WebUI-Loading-bar-won-t-.patch \
    file://0005-Bug-1574-1575-Starscream-ast-OpenBMC-WebUI-Network-A.patch \
    file://0006-video-recorder-changes.patch \
"


