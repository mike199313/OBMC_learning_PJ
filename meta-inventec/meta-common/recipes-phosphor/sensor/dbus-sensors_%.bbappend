FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

SRC_URI += "\
            file://0001-IpmiInfo-Patch-to-support-IPMI-sensor-info-and-add-I.patch \
            file://0002-Sensors-Patch-to-support-sensor-number-entity-number.patch \
            file://0003-Fan-Patch-to-support-sensor-number-entity-number-and.patch \
            file://0004-ADCSensor-Support-I2C-adc-sensor.patch \
            file://0005-HwmonTemp-Add-tmp468-support-and-Label-search-for-ex.patch \
            file://0006-PSUSensor-Add-support-for-inventec-virtual-driver.patch \
            file://0007-EventSensor-Create-EventSensor-to-setup-event-only-s.patch \
            file://0008-HwmonTemp-Add-totalThresholdNumber-when-HwmonTempSen.patch \
            file://0009-Sensors-Skip-sub-sensor-if-sensorInfo-not-config.patch \
            file://0010-ExitAirTemp-Patch-to-support-sensor-number-entity-nu.patch \
            file://0011-AverageSensor-averagesensor-initial-commit.patch \
            file://0012-InvCfmSensor-Initial-Inventec-CFM-sensor.patch \
            file://0013-Bug-573-SW-Common-Accumulate-sensor-initial-commit.patch \
            file://0014-PSUSensor-add-extra-driver-config.patch \
            file://0015-IiohwmonSensor-add-the-iio_hwmon-sensor-initial-and-.patch \
            file://0016-TsodSensor-add-TsodSensor-support-temp-sensor-on-dim.patch \
            file://0017-watchdog-Add-WATCHDOG-sensor-DBUS-interface.patch \
            file://0018-NvmeSensor-patches.patch \
            file://0024-Event-generation-enabled-disabled.patch \
            file://0025-get-sensor-enable-after-setting.patch \
           "

PACKAGECONFIG:append =" \
            nvmesensor \
            eventsensor \
            averagesensor \
            invcfmsensor \
            accumulatesensor \
            iiohwmonsensor \
            tsodsensor \
            wdtsensor \
            "

PACKAGECONFIG:remove ="mcutempsensor intrusionsensor"

PACKAGECONFIG[eventsensor] = "-Devent=enabled, -Devent=disabled"
PACKAGECONFIG[averagesensor] = "-Daverage=enabled, -Daverage=disabled"
PACKAGECONFIG[invcfmsensor] = "-Dinvcfm=enabled, -Dinvcfm=disabled"
PACKAGECONFIG[accumulatesensor] = "-Daccumulate=enabled, -Daccumulate=disabled"
PACKAGECONFIG[iiohwmonsensor] = "-Diiohwmon=enabled, -Diiohwmon=disabled"
PACKAGECONFIG[tsodsensor] = "-Dtsod=enabled, -Dtsod=disabled"
PACKAGECONFIG[wdtsensor] = "-Dwdt=enabled, -Dwdt=disabled"

SYSTEMD_SERVICE:${PN} += "${@bb.utils.contains('PACKAGECONFIG', 'nvmesensor', \
                                               'xyz.openbmc_project.nvmesensor.service', \
                                               '', d)}"
SYSTEMD_SERVICE:${PN} += "${@bb.utils.contains('PACKAGECONFIG', 'eventsensor', \
                                               'xyz.openbmc_project.eventsensor.service', \
                                               '', d)}"
SYSTEMD_SERVICE:${PN} += "${@bb.utils.contains('PACKAGECONFIG', 'averagesensor', \
                                               'xyz.openbmc_project.averagesensor.service', \
                                               '', d)}"
SYSTEMD_SERVICE:${PN} += "${@bb.utils.contains('PACKAGECONFIG', 'invcfmsensor', \
                                               'xyz.openbmc_project.invcfmsensor.service', \
                                               '', d)}"
SYSTEMD_SERVICE:${PN} += "${@bb.utils.contains('PACKAGECONFIG', 'accumulatesensor', \
                                               'xyz.openbmc_project.accumulatesensor.service', \
                                               '', d)}"
SYSTEMD_SERVICE:${PN} += "${@bb.utils.contains('PACKAGECONFIG', 'iiohwmonsensor', \
                                               'xyz.openbmc_project.iiohwmonsensor.service', \
                                               '', d)}"
SYSTEMD_SERVICE:${PN} += "${@bb.utils.contains('PACKAGECONFIG', 'tsodsensor', \
                                               'xyz.openbmc_project.tsodsensor.service', \
                                               '', d)}"
SYSTEMD_SERVICE:${PN} += "${@bb.utils.contains('PACKAGECONFIG', 'wdtsensor', \
                                               'xyz.openbmc_project.wdtsensor.service', \
                                               '', d)}"
