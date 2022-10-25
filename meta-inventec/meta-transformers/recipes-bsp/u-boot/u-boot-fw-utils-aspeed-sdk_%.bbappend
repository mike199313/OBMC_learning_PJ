FILESEXTRAPATHS:append := "${THISDIR}/${BPN}:"
# In order to reuse and easily maintain, we use the same patch files among u-boot-aspeed-sdk
FILESEXTRAPATHS:append:= "${THISDIR}/u-boot-aspeed-sdk:"


SRC_URI:append = " file://fw_env.config \
                   file://transformers-ast2600.cfg \
                   file://transformers-ast2600_defconfig \
                   file://ast2600-transformers.dts \
                   file://0001-Modify-bootfile-name-and-env-offset.patch \
                 "

do_install:append () {
        install -d ${D}${sysconfdir}
        install -m 0644 ${WORKDIR}/fw_env.config ${D}${sysconfdir}/fw_env.config
        install -m 0644 ${WORKDIR}/fw_env.config ${S}/tools/env/fw_env.config
}

do_copyfile () {
    if [ -e ${WORKDIR}/ast2600-transformers.dts ] ; then
        cp -v ${WORKDIR}/ast2600-transformers.dts ${S}/arch/arm/dts/
    else
        # if use devtool modify, then the append files were stored under oe-local-files
        cp -v ${S}/oe-local-files/ast2600-transformers.dts ${S}/arch/arm/dts/
    fi

    if [ -e ${WORKDIR}/transformers-ast2600_defconfig  ] ; then
        cp -v ${WORKDIR}/transformers-ast2600_defconfig  ${S}/configs/
    else
        cp -v ${S}/oe-local-files/transformers-ast2600_defconfig ${S}/configs/
    fi
}

addtask copyfile after do_patch before do_configure
