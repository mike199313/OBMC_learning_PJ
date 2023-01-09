SRC_URI += "file://0001-Add-Bits-sensor-and-fix-composite-state-sensor-error.patch"
SRC_URI += "file://0002-Patch-for-PLDM-sensor-reading-and-FWU.patch"
SRC_URI += "file://0003-PLDMD-with-multiple-interface-support.patch"
SRC_URI += "file://0004-PLDM-firmware-update-with-specified-PID.patch"
SRC_URI += "file://0005-Custom-modification-of-E810-PLDM-firmware-update.patch"
SRC_URI += "file://0006-Custom-modification-of-ACC100-for-PLDMD.patch"
SRC_URI += "file://0007-Custom-modification-of-BlueField2-for-PLDMD.patch"

PCIE_BINDING = "pcie"
SMBUS_BINDING = "smbus"

FILES:${PN} += "${systemd_system_unitdir}/xyz.openbmc_project.pldmd@.service"

SYSTEMD_SERVICE:${PN} += "xyz.openbmc_project.pldmd@${PCIE_BINDING}.service"
SYSTEMD_SERVICE:${PN} += "xyz.openbmc_project.pldmd@${SMBUS_BINDING}.service"
