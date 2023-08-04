FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

SRC_URI:append = " file://transformers.cfg \
                   file://arch \
                   file://drivers \
                   file://0001-Kernel-sync-Aspeed-tag-00.05.06-misc-drivers.patch \
                   file://0002-Kernel-sync-Aspeed-tag-00.05.06-soc-drivers.patch \
                   file://0003-Kernel-sync-Aspeed-tag-00.05.06-dsti.patch \
                   file://0004-Kernel-sync-Aspeed-tag-00.05.06-espi-modification.patch \
                   file://0005-Bug-752-SW-common-kernel-Fix-MTD-partitions-were-del.patch \
                   file://0006-Initial-Common-Add-common-hwmon-drivers.patch \
                   file://0007-Initial-Common-Add-virtual-driver-to-simulate-driver.patch \
                   file://0008-Subject-PATCH-Revise-ADT7462-driver-in-kernel.patch \
                   file://0009-Subject-PATCH-Kernel-dts-driver-Patched-hsc-setting-.patch \
                   file://0010-Subject-Patch-kernel-driver-Set-max31790-default-ena.patch \
                   file://0011-Subject-Patch-kernel-RTC-Set-a-default-timestamp-for.patch \
                   file://0012-Modify-RGMII-TX-Clock-delay.patch \
                   file://0013-Fix-KCS3-cannot-be-created.patch \
                   file://0014-Enable-mmcblk0-block.patch \
                   file://0015-add-aspeed-pwm-tachometer-driver-and-modify-the-dtsi.patch \
                   file://0016-Implement-a-memory-driver-share-memory.patch \
                   file://0017-roll-back-the-ipmi-driver-to-2.10.patch \
                   file://0018-Add-aspeed-mctp-dts-definition.patch \
                   file://0019-Patch-for-Slave-mqueue-driver-porting.patch \
                 "

do_add_overwrite_files () {
    cp -r "${WORKDIR}/arch" \
          "${STAGING_KERNEL_DIR}"
    cp -r "${WORKDIR}/drivers" \
          "${STAGING_KERNEL_DIR}"
}

addtask do_add_overwrite_files after do_patch before do_compile
