FILESEXTRAPATHS:prepend:transformers-nuv := "${THISDIR}/${PN}:"

SRC_URI:transformers-nuv := "git://github.com/Nuvoton-Israel/libmctp.git;protocol=ssh;branch=main"
SRCREV:transformers-nuv := "c3ec452185615560d82fee2fb7f9b61e496de93e"

TARGET_CFLAGS += "-DMCTP_HAVE_FILEIO"


