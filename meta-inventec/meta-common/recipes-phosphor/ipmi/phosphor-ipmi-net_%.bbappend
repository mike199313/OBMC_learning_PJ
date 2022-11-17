FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG:append = " rmcp-ping"
PACKAGECONFIG[rmcp-ping] = "--enable-rmcp-ping=yes,--enable-rmcp-ping=no"

SRC_URI += " \
    file://0001-Fix-session-handle-not-change-issue.patch \
    file://0002-inventec-common-Implement-LAN-Config-Primary-RMCP-Po.patch \
    file://0003-Implement-generate-SIK-by-bmckey.patch \
    file://0004-IPMI-RMCPplus-support-HMAC-MD5.patch\
    file://0005-Fix-phosphor-ipmi-net-eth0.service-coredump-after-ac.patch \
"               
