FILESEXTRAPATHS:prepend:transformers-nuv := "${THISDIR}/${PN}:"
SRC_URI:append:transformers-nuv = " file://chassis-capabilities.override.yml"
SRC_URI:append:transformers-nuv = " file://sol-parameters.override.yml"
