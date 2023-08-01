FILESEXTRAPATHS:prepend:nuvoton:= "${THISDIR}/${PN}:"

SRC_URI:append:nuvoton= " file://0001-fansensor-add-Nuvoton-npcm845-fan-support.patch"