SUMMARY = "OpenBMC for Inventec - Applications"
PR = "r1"

inherit packagegroup

PROVIDES = "${PACKAGES}"
PACKAGES = " \
        ${PN}-chassis \
        ${PN}-fans \
        ${PN}-flash \
        ${PN}-system \
        "

PROVIDES += "virtual/obmc-chassis-mgmt"
PROVIDES += "virtual/obmc-fan-mgmt"
PROVIDES += "virtual/obmc-flash-mgmt"
PROVIDES += "virtual/obmc-system-mgmt"

RPROVIDES:${PN}-chassis += "virtual-obmc-chassis-mgmt"
RPROVIDES:${PN}-fans += "virtual-obmc-fan-mgmt"
RPROVIDES:${PN}-flash += "virtual-obmc-flash-mgmt"
RPROVIDES:${PN}-system += "virtual-obmc-system-mgmt"

SUMMARY:${PN}-chassis = "Inventec Chassis"
RDEPENDS:${PN}-chassis = " \
        x86-power-control \
        obmc-host-failure-reboots \
        "

SUMMARY:${PN}-fans = "Inventec Fans"
RDEPENDS:${PN}-fans = " \
        phosphor-pid-control \
        "

SUMMARY:${PN}-flash = "Inventec Flash"
RDEPENDS:${PN}-flash = " \
        obmc-control-bmc \
        phosphor-ipmi-blobs \
        phosphor-ipmi-flash \
        "

SUMMARY:${PN}-system = "Inventec System"
RDEPENDS:${PN}-system = " \
        bmcweb \
        entity-manager \
        intel-ipmi-oem \
        dbus-sensors \
        webui-vue \
        phosphor-snmp \
        phosphor-sel-logger \
        phosphor-gpio-monitor \
        phosphor-gpio-monitor-monitor \
        vlan \
        tzdata \
        phosphor-host-postd \
        e2fsprogs \
        phosphor-post-code-manager \
	iptables \
	nbd-server \
        nfs-utils \
	libtirpc \
        "

