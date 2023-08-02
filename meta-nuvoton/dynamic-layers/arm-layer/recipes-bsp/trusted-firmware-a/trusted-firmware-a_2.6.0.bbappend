SRC_URI:remove = "git://git.trustedfirmware.org/TF-A/trusted-firmware-a.git;protocol=https;name=tfa;branch=master"
SRC_URI:append = "git://github.com/Nuvoton-Israel/arm-trusted-firmware.git;protocol=https;name=tfa;branch=nuvoton"
SRCREV_tfa = "009fd73ba58ac744bf9168e425afa19357ea331d"

# Enable no warning for loading segment with RWX permissions
EXTRA_OEMAKE += "LDFLAGS='--no-warn-rwx-segments'"
