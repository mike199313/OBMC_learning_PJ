FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG:append = " resizefru"
PACKAGECONFIG[resizefru] = "-Dfru-device-resizefru=true, -Dfru-device-resizefru=false, ,"

SRC_URI:append  = " \
                    file://0001-Add-a-writable-dbus-interface.patch \
                    file://0002-Remove-fru-format-checking-rules.patch \
                    file://0003-Make-the-Fru-data-with-writable-attributes.patch \
                  "
