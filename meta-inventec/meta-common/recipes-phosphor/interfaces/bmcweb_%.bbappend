FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

inherit pkgconfig

DEPENDS += " phosphor-dbus-interfaces"
SRCREV = "e155ab54ec5ad4c31937f4d7de8b502e91468e43"

# To enable debug msg
# Note:
#   If eable this setting, the app 'bmcweb' will be larger than default one
#   and needs more time to compile it. It should be disabled, if do a formal relese build

#EXTRA_OEMESON:remove = "--buildtype=minsize"
#EXTRA_OEMESON += " --buildtype=debug -Dbmcweb-logging=enabled"
EXTRA_OEMESON += "-Dhttp-body-limit=512"
EXTRA_OEMESON += "-Drest=enabled"

PACKAGECONFIG[journal] = "-Dredfish-bmc-journal=enabled,-Dredfish-bmc-journal=disabled,"
PACKAGECONFIG[dump] = "-Dredfish-dump-log=enabled,-Dredfish-dump-log=disabled,"
PACKAGECONFIG[debug] = "-Dbmcweb-logging=enabled,-Dbmcweb-logging=disabled,"
PACKAGECONFIG[dbus-log] = "-Dredfish-dbus-log=enabled,-Dredfish-dbus-log=disabled,"
PACKAGECONFIG[cpu-log] = "-Dredfish-cpu-log=enabled,-Dredfish-cpu-log=disabled,"
#PACKAGECONFIG[peci] = "-Dredfish-raw-peci=enabled,-Dredfish-raw-peci=disabled,"
PACKAGECONFIG[tftp] = "-Dinsecure-tftp-update=enabled,-Dinsecure-tftp-update=disabled,"
PACKAGECONFIG[etag] = "-Dredfish-enable-etag=enabled,-Dredfish-enable-etag=disabled,"
PACKAGECONFIG[inv-psu] = "-Dredfish-enable-inv-psu=enabled,-Dredfish-enable-inv-psu=disabled,"
PACKAGECONFIG[query] = "-Dinsecure-enable-redfish-query=enabled,-Dinsecure-enable-redfish-query=disabled,"

PACKAGECONFIG:append = " journal dump tftp etag inv-psu query"

SRC_URI:append = " \
    file://0001-Empty-base-dn-error.patch \
    file://0002-Fix-ldap-localRole-invalid-privilege-causing-interna.patch \
    file://0003-Fix-invalid-ldap-server-uri-causing-internal-server-.patch \
    file://0004-Ip-fix.patch \
    file://0005-Add-redfish-managers-serialInterfaces_updated.patch \
    file://0006-delete-event-log.patch \
    file://0007-Boot-flag.patch \
    file://0008-Redfish-add-power-matrics-support.patch \
    file://0009-Add-system-airflow-sensor-support.patch \
    file://0010-Add-ETag-support-for-Redfish.patch \
    file://0011-CollectionCapabilities.patch \
    file://0012-GET-redfish_v1_SessionService-without-authentication-return-401.patch \
    file://0013-Fix-no-PowerSupplies-under-chassis-power-URI.patch \
    file://0014-Redfish-Support-for-composition-service.patch \
    file://0015-Bug-1078-Transformers-OpenBMC-Redfish-Health-status-.patch \
    file://0016-NTP-Server-not-loss-after-reboot.patch \
    file://0017-enable-fru-to-set-product-properties-by-PATCH.patch \
    file://0018-log_service-Fix-error-message-for-non-existing-log-e.patch \
    file://0019-Fix-excerpt-query-fail.patch \
    file://0020-Bug-1436-System-count-doesnt-match.patch \
    file://0021-Bug-1431-Members-of-PCIe-Functions-are-missing.patch \
    file://0022-Bug-1075-Transformers-OpenBMC-WebUI-network-Unable-t.patch \
    file://0023-Bug-1506-Transformers-OpenBMC-Redfish-Modify-IPv4-ad.patch \
    file://0024-Simple-Rack-mounted-Server-for-Sensors.patch \
    file://0025-Redfish-Chassis-schema-enhancement.patch \
"
