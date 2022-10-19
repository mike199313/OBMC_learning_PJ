FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
                    file://0001-fix-sensor-reading-logging-problem.patch \
                    file://0002-Change-service-setting-from-name-to-readpath.patch \
                    file://0003-Adding-moving-average-method-length-2-for-thermal.patch \
                    file://0004-Modify-zone-startup-mechanism.patch \
                    file://0005-Add-upatetime-attribute-to-config-json-for-different.patch \
                 "
