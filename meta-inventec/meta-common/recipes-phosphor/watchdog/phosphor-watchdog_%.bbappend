FILESEXTRAPATHS:append := ":${THISDIR}/${PN}"

SRC_URI += "  \
            "

#            file://0001-Watchgog-Support-IPMI-Watchgog-event-and-sensor-dbus.patch
#            file://0002-Watchdog-Store-Don-t-log-flag-to-dbus.patch 

# Remove the override to keep service running after DC cycle
SYSTEMD_OVERRIDE:${PN}:remove = "poweron.conf:phosphor-watchdog@poweron.service.d/poweron.conf"
SYSTEMD_SERVICE:${PN} = "phosphor-watchdog.service"
