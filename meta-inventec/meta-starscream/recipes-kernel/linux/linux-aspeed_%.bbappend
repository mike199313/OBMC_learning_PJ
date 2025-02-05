FILESEXTRAPATHS:append:starscream := "${THISDIR}/${PN}:"

SRC_URI:append = " file://starscream.cfg \
                   file://arch \
                   file://drivers \
                   file://0001-Kernel-sync-Aspeed-tag-00.05.06-misc-drivers.patch \
                   file://0002-Kernel-sync-Aspeed-tag-00.05.06-soc-drivers.patch \
                   file://0003-Kernel-sync-Aspeed-tag-00.05.06-dsti.patch \
                   file://0004-Bug-752-SW-common-kernel-Fix-MTD-partitions-were-del.patch \
                   file://0005-Initial-Common-Add-common-hwmon-drivers.patch \
                   file://0006-Initial-Common-Add-virtual-driver-to-simulate-driver.patch \
                   file://0007-Subject-PATCH-Revise-ADT7462-driver-in-kernel.patch \
                   file://0008-Subject-PATCH-Kernel-dts-driver-Patched-hsc-setting-.patch \
                   file://0009-Subject-Patch-kernel-driver-Set-max31790-default-ena.patch \
                   file://0010-Subject-Patch-kernel-RTC-Set-a-default-timestamp-for.patch \
                   file://0011-Enable-mmcblk0-block.patch \
                   file://0012-Implement-a-memory-driver-share-memory.patch \
                   file://0013-Eth0-error-and-dump-kernel-logs.patch \
                "


do_add_overwrite_files () {
    cp -r "${WORKDIR}/arch" \
          "${STAGING_KERNEL_DIR}"
    cp -r "${WORKDIR}/drivers" \
          "${STAGING_KERNEL_DIR}"
}

addtask do_add_overwrite_files after do_patch before do_compile
