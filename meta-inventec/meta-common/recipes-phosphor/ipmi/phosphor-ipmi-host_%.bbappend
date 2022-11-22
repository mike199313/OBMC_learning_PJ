EXTRA_OECONF:append = " --disable-i2c-whitelist-check"
EXTRA_OECONF:append = " --disable-ipmi-whitelist"
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://0001-Sensor-Implement-SetSensorThreshold-command.patch \
                   file://0002-Sensor-Implement-GetSensorReadingFactors-command.patch \
                   file://0003-Save-the-pre-timeout-interrupt-in-dbus-property.patch \
                   file://0004-Watchdog-Store-Don-t-log-flag-to-dbus.patch \
                   file://0005-App-Compose-Aux-Firmware-Rev-Info-in-Get-Device-Id-c.patch \
                   file://0006-Implement-LAN-Config-IPv6-Static-Hop-Limit.patch \
                   file://0007-Implement-LAN-Config-Community-String.patch \
                   file://0008-Implement-LAN-Config-Primary-RMCP-Port.patch \
                   file://0009-Dcmi-Implement-DCMI-get-power-reading.patch \
                   file://0010-Implement-LAN-Config-Destination-Addresses.patch \
                   file://0011-Dcmi-Enhance-set-get-power-limit.patch \
                   file://0012-Implement-LAN-Config-VLAN-Priority.patch \
                   file://0013-Bug-354-Transformers-OpenBMC-IPMI-Get-Enhanced-Syste.patch \
                   file://0014-Refine-IPFamilyEnables-command.patch \
                   file://0015-Add-MAC-address-support-for-destination-address.patch \
                   file://0016-Add-IPv6Only-mode-support-in-IPFamilySupport-command.patch \
                   file://0017-Add-IPv6-Header-Traffic-Class-support.patch \
                   file://0018-Add-encoding-byte-support-for-System-Info-Parameters.patch \
                   file://0019-Support-IPMI-v1.5-Session-Management.patch \
                   file://0020-Add-error-checking-to-prevent-core-dump.patch \
                   file://0021-Bug-378-Transformers-OpenBMC-IPMI-Authentication-typ.patch \
                   file://0022-Channel-Add-special-case-for-system-interface-in-Get.patch \
                   file://0023-Channel-Implement-set-channel-security-key.patch \
                   file://0024-Bug623-SW-Transformers-OpenBMC-IPMI-Implement-get-ch.patch \
                   file://0025-Reset-global-enable-configs-to-default-when-cold-res.patch \
                   file://0026-Bug-809-SW-Transformers-OpenBMC-IPv6-function-enhanc.patch \
                   file://0027-Master-write-read-IPMB.patch \
                   file://0028-Fix-firware-version-is-null-issue.patch \
                   file://0029-Bug-1278-getCPUInfo-completion-code.patch \
                   file://0030-user-management-framework-concurrency-access-issue.patch \
                   file://0031-Remove-setLastPowerEvent-function.patch \
                   file://0032-Telemetry-Service-IPMI-Commands.patch \
"

SYSTEMD_SERVICE:${PN}:append = " phosphor-ipmi-host.service"

IPMI_HOST_NEEDED_SERVICES = "\
    mapper-wait@-xyz-openbmc_project-control-host{}-boot.service \
    mapper-wait@-xyz-openbmc_project-control-host{}-boot-one_time.service \
    mapper-wait@-xyz-openbmc_project-control-host{}-power_restore_policy.service \
    mapper-wait@-xyz-openbmc_project-control-host{}-restriction_mode.service \
    "

do_install:append() {

    # Create service override file.
    override_file=${D}${systemd_system_unitdir}/phosphor-ipmi-host.service.d/10-override.conf
    rm ${override_file}
    echo "[Unit]" > ${override_file}

    # Insert host-instance based service dependencies.
    for i in ${OBMC_HOST_INSTANCES};
    do
        for s in ${IPMI_HOST_NEEDED_SERVICES};
        do
            service=$(echo ${s} | sed "s/{}/${i}/g")
            echo "Wants=${service}" >> ${override_file}
        done
    done

  install -d ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/sensorhandler.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/selutility.hpp ${D}${includedir}/phosphor-ipmi-host
}


