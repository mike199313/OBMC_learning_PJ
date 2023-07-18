FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SMBUS_BINDING = "smbus"
PCIE_BINDING = "pcie"

FILES:${PN} += "${systemd_system_unitdir}/xyz.openbmc_project.nvme-mi@.service"

SYSTEMD_SERVICE:${PN} += "xyz.openbmc_project.nvme-mi@${SMBUS_BINDING}.service"
SYSTEMD_SERVICE:${PN} += "xyz.openbmc_project.nvme-mi@${PCIE_BINDING}.service"

FILES:${PN}:remove = "${systemd_system_unitdir}/xyz.openbmc_project.nvme-mi.service"

SYSTEMD_SERVICE:${PN}:remove = "xyz.openbmc_project.nvme-mi.service"


SRC_URI += "file://0001-nvme-mi-support-smbus-pcie-binding-interfaces.patch"

