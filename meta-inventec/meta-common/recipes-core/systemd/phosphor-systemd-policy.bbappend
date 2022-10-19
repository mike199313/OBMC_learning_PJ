FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

SRC_URI += "file://journald-storage-policy.conf"

FILES:${PN} += "${systemd_unitdir}/journald.conf.d/journald-storage-policy.conf"

do_install:append() {
        install -m 644 -D ${WORKDIR}/journald-storage-policy.conf ${D}${systemd_unitdir}/journald.conf.d/journald-storage-policy.conf
}
