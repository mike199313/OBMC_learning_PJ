FILESEXTRAPATHS:prepend:scm-npcm845 := "${THISDIR}/${PN}:"

PACKAGECONFIG:append:scm-npcm845 = " verify_signature flash_bios"

SRC_URI:append:scm-npcm845 = " file://restore_verify_bios.patch"

EXTRA_OEMESON:append:scm-npcm845 = " -Doptional-images=image-bios"