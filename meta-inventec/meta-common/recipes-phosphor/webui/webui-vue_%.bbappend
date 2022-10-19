FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-Fix-Fan-and-Powersupply-path.patch \
    file://0002-Bug1059-1060-1062-Transformers-OpenBMC-WebUI-status-.patch \
    file://0003-Bug-1122-1123-Transformers-Transformers-nuv-OpenBMC-.patch \
    file://0004-Bug-1117-Transformers-OpenBMC-WebUI-Display-incorrec.patch \
    file://0005-bug-1073-1074-Transformers-OpenBMC-WebUI-log-error-m.patch \
    file://0006-Bug1120-1121-Common-OpenBMC-WebUI-Loading-bar-won-t-.patch \
"
                      
