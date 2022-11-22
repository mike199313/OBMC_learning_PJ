FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG:append = " rmcp-ping"
PACKAGECONFIG[rmcp-ping] = "--enable-rmcp-ping=yes,--enable-rmcp-ping=no"

SRC_URI += " \
    file://0001-Fix-session-handle-not-change-issue.patch \
    file://0002-inventec-common-Implement-LAN-Config-Primary-RMCP-Po.patch \
    file://0003-IPMI-Session-RMCP-RMCPplus.patch \
    file://0004-Implement-generate-SIK-by-bmckey.patch \
    file://0005-IPMI-RMCPplus-support-HMAC-MD5.patch\
"               
