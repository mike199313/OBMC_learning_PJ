FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRCREV = "239aa7d705e69d37383df37f6cbb67c0d9425423"

EXTRA_OECONF:append = " enable_configure_inv_pi_algorithm=true"

SRC_URI:append = " \
                  file://0001-Add-moving-average-method.patch \
                  file://0002-Modify-zone-startup-mechanism.patch  \
                  file://0003-Accept-missing-flag-for-dynamic-inputs-in-zone.patch \
                  file://0004-Add-inventec-maxError-pi-control-algorithm.patch \
                  file://0005-Change-pid-config-into-entity-manager-config.patch \
                 "
