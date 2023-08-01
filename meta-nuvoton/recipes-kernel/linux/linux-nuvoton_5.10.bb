KSRC = "git://github.com/Nuvoton-Israel/linux;protocol=https;branch=${KBRANCH}"
KBRANCH = "NPCM-5.10-OpenBMC"
LINUX_VERSION = "5.10.161"
SRCREV = "66c397881e1f1968f42bb8f5b26e967fcd0880af"

require linux-nuvoton.inc
SRC_URI:append:nuvoton = " file://enable_emmc_510.cfg"
