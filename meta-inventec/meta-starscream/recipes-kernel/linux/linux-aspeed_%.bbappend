FILESEXTRAPATHS:append := "${THISDIR}/${PN}:"

SRC_URI:append = " file://starscream.cfg \
                   file://arch \
                   file://drivers \
                   file://0001-Kernel-sync-Aspeed-tag-00.05.03-misc-drivers.patch \
                   file://0002-Kernel-sync-Aspeed-tag-00.05.03-soc-drivers.patch \
                   file://0003-Kernel-sync-Aspeed-tag-00.05.03-dsti.patch \
                   file://0004-Bug-752-SW-common-kernel-Fix-MTD-partitions-were-del.patch \
                   file://0005-Kernel-sync-Intel-BMC-branch-dev-5.15-intel-peci-dri.patch \
                   file://0006-Initial-Common-Add-common-hwmon-drivers.patch \
                   file://0007-Initial-Common-Add-virtual-driver-to-simulate-driver.patch \
                   file://0008-Subject-PATCH-Revise-ADT7462-driver-in-kernel.patch \
                   file://0009-Subject-PATCH-Kernel-dts-driver-Patched-hsc-setting-.patch \
                   file://0010-Subject-Patch-kernel-driver-Set-max31790-default-ena.patch \
                   file://0011-Subject-Patch-kernel-RTC-Set-a-default-timestamp-for.patch \
                   file://0012-Enable-mmcblk0-block.patch \
                   file://0013-Implement-a-memory-driver-share-memory.patch \
                 "

do_add_overwrite_files () {
    cp -r "${WORKDIR}/arch" \
          "${STAGING_KERNEL_DIR}"
    cp -r "${WORKDIR}/drivers" \
          "${STAGING_KERNEL_DIR}"
}

addtask do_add_overwrite_files after do_patch before do_compile

