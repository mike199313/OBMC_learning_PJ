DESCRIPTION = "Inventec utilities"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESEXTRAPATHS:prepend := "${THISDIR}:"

inherit cmake systemd logging obmc-phosphor-sdbus-service

DEPENDS = "boost \
           dbus \
           phosphor-dbus-interfaces \
           phosphor-logging \
           sdeventplus \
           nlohmann-json \
           sdbusplus \
           valijson \
           systemd \
           openssl"

S = "${WORKDIR}/${BPN}"

SRC_URI = "file://${BPN}/CMakeLists.txt \
           file://${BPN}/include/configuration_manager.hpp \
           file://${BPN}/include/watch.hpp \
           file://${BPN}/src/configuration_manager_main.cpp \
           file://${BPN}/src/configuration_manager.cpp \
           file://${BPN}/src/watch.cpp \
           file://configuration_upload.service \
           file://schema.json \
"

SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN} += "configuration_upload.service"

EXTRA_OECMAKE=""

FILES:${PN} += "${systemd_unitdir}/configuration_upload.service \
                /etc/conf/configs/schema.json \
                "

do_install:append() {
  install -d ${D}/var/wcs/home/configs
  install -d ${D}/etc/defconfig/configs
  install -d ${D}/etc/conf/configs/user
  install -d ${D}/etc/conf/configs/pending
  
  install -m 0644 -D ${WORKDIR}/schema.json ${D}/etc/conf/configs/schema.json

  install -d ${D}/${systemd_unitdir}
  install -m 0644 ${WORKDIR}/configuration_upload.service ${D}/${systemd_unitdir}/configuration_upload.service
}
