FILESEXTRAPATHS:prepend:evb-npcm845 := "${THISDIR}/${PN}:"

SRC_URI:append:evb-npcm845 = " \
    file://0001-hwmon-temp-add-tmp100-support.patch \
    file://0002-HwmonTempSensor-Add-BMCOnDieThermalSensor-type.patch \
    "