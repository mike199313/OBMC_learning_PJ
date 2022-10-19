FILESEXTRAPATHS:prepend:transformers := "${THISDIR}/${PN}:"
SRC_URI:append:transformers = " file://chassis-capabilities.override.yml"
SRC_URI:append:transformers = " file://sol-parameters.override.yml"
