
# Overwrite the service configuration "bmc_booted.conf"
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI:append = " file://bmc_booted.conf"

do_compile:prepend:transformers() {
    install -m 0644 ${STAGING_DATADIR_NATIVE}/${PN}/led.yaml ${S}
}
