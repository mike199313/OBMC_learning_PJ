FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-support-mmap-smbios-table-from-video-shared-memory-a.patch \
            file://0002-Implement-OperationalStatus-dbus-interface.patch \
            file://0003-Fix-error-for-Redfish-can-t-get-BIOS-version.patch \
"
