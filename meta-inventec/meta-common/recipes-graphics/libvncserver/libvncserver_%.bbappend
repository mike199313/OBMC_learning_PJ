PACKAGECONFIG += "ffmpeg"

FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

SRC_URI:append = " file://0001-video-recorder-changes.patch \
                 "

#DEPENDS += " \ 
#        ffmpeg \
#        "

