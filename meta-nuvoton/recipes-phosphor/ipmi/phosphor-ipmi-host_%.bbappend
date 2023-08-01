FILESEXTRAPATHS:append:nuvoton:= "${THISDIR}/${PN}:"

# Fix ERROR opening IPMI provider when enable dbus-sdr
SRC_URI:append:nuvoton= " file://0001-move-depend-from-libipmi20-to-entity_map_json.patch"