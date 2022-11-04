FILESEXTRAPATHS:prepend :=  "${THISDIR}/file:"

SRC_URI += "file://BootBlockAndHeader_RunBMC.xml"
SRC_URI += "file://UbootHeader_RunBMC.xml"

do_install:append() {
	install -d ${DEST}
	install -m 0644 ${WORKDIR}/BootBlockAndHeader_RunBMC.xml ${DEST}/BootBlockAndHeader_${IGPS_MACHINE}.xml
	install -m 0644 ${WORKDIR}/UbootHeader_RunBMC.xml ${DEST}/UbootHeader_${IGPS_MACHINE}.xml 
}
