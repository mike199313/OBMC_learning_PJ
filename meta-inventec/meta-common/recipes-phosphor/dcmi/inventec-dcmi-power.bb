DESCRIPTION = "Inventec DCMI power handler"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

S = "${WORKDIR}/${BPN}"
EXTERNALSRC_SYMLINKS = ""
EXTERNALSRC = "${THISDIR}/${PN}"
EXTERNALSRC_BUILD = "${B}"

SYSTEMD_SERVICE:${PN} += "inventec-dcmi-power.service"

DEPENDS = "boost sdbusplus nlohmann-json"
inherit meson systemd externalsrc pkgconfig

