FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " file://features.hpp \
                 "

do_copyfile () {
    if [ -e ${WORKDIR}/features.hpp ] ; then
        cp -v ${WORKDIR}/features.hpp ${S}/include/
    else
        # if use devtool modify, then the append files were stored under oe-local-files
        cp -v ${S}/oe-local-files/features.hpp ${S}/include/
    fi
}

addtask copyfile after do_patch before do_configure
