FILESEXTRAPATHS:prepend:transformers-nuv := "${THISDIR}/linux-nuvoton:"

SRC_URI:append:transformers-nuv = " \
  file://arch \
  file://drivers \
  file://buv-runbmc.cfg \
  file://0001-Add-peci-driver-of-Nuvoton.patch \
  file://0002-driver-SPI-add-W25Q01JV-and-W25Q512JVFIM-support.patch \
  file://0003-driver-misc-seven-segment-display-gpio-driver.patch \
  file://0004-virtual-Add-virtual-driver-config.patch \
  file://0005-inventec-transformers-nuv-Modify-TOCK-to-PLL0-for-RGMII-issue.patch \
  file://0006-Add-tmp468-driver.patch \
  file://0007-Modify-sgpio-driver.patch \
  file://0008-Add-watchdog-time.patch \
  file://0009-reset-DMA-again-if-stmmac_reset-failed.patch \
  file://0010-Bug-1028-SW-Transformers-nuv-kernel-Add-SAPPHIRERAPI.patch \
  "

# Merge source tree by original project with our layer of additional files
do_add_vesnin_files () {
    cp -r "${WORKDIR}/arch" \
          "${STAGING_KERNEL_DIR}"
    cp -r "${WORKDIR}/drivers" \
          "${STAGING_KERNEL_DIR}"
}
addtask do_add_vesnin_files after do_patch before do_compile

