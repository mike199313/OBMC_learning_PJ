FILESEXTRAPATHS:prepend:starscream := "${THISDIR}:"


SRC_URI:append:starscream = " file://${BPN}/src/ms-subsystem-check.cpp           \
"

