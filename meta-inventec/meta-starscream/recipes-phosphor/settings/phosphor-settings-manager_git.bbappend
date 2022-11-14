FILESEXTRAPATHS:append:starscream := ":${THISDIR}/${PN}"
SRC_URI:append = " file://chassis-capabilities.override.yml \
                 "