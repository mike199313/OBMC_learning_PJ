#!/bin/sh

# Init GPIO setting
# gpio init
gpioset `gpiofind CPLD_PWRBRK_N`=1
gpioset `gpiofind ASSERT_CPU0_PROCHOT_R_N`=1
gpioset `gpiofind ASSERT_CPU1_PROCHOT_R_N`=1
gpioset `gpiofind BIOS_RECOVERY_BUF_N`=1
gpioset `gpiofind IRQ_BMC_CPU0_BUF_NMI_N`=1
gpioset `gpiofind NCSI_OCP_CLK_EN_N`=0
gpioset `gpiofind SCM_JTAG_MUX_SE`=0

#check platform type and inform cpld what platform it is by BMC PIN122.(GPIOX4/GPIO177)
OPENBMC_TARGET_MACHINE=`cat /etc/os-release | grep OPENBMC_TARGET_MACHINE | awk -F '"' '{print $2}'`
if [ "$OPENBMC_TARGET_MACHINE" = "starscream" ]; then
    echo "OPENBMC_TARGET_MACHINE = Aspeed"
    gpioset `gpiofind PLATFORM_TYPE`=0
else
    echo "OPENBMC_TARGET_MACHINE = Nuvoton"
    gpioset `gpiofind PLATFORM_TYPE`=1
fi
sleep 0.1

#inform cpld that sgio is ready for use
gpioset `gpiofind RST_BMC_SGPIO_R_N`=1
# sgpio init
gpioset `gpiofind RST_BMC_CPU0_I2C_N`=1
gpioset `gpiofind RST_BMC_CPU1_I2C_N`=1
gpioset `gpiofind I2C_BUS7_RESET_N`=1
gpioset `gpiofind BMC_USB2514_1_RESET_N`=1
gpioset `gpiofind BMC_CPU0_UART_EN`=0
gpioset `gpiofind HDT_BUF_EN_N`=0
gpioset `gpiofind BMC_ASSERT_CLR_CMOS`=0
gpioset `gpiofind HDT_MUX_SELECT_MON`=0
gpioset `gpiofind CPLD_JTAG_OE_R_N`=1
gpioset `gpiofind CPLD_HDT_RESET_N`=1
gpioset `gpiofind SPI_MUX_SELECT`=0

FAN_DEF_SH="/usr/bin/fan-default-speed.sh"
bash $FAN_DEF_SH

#if bmc reboot while mb power stays on , following conditions need to be checked
pe_rst=$(gpioget `gpiofind CPU0_PE_RST0`)
if [ "$pe_rst" = "1" ]; then
    echo "pe rst is high"
    systemctl start post-complete@init.service
    systemctl start dimm-plug@init.service
else
    echo "pe rst is low, set i2c-mux to cpu"
    #set dimm i2c mux to cpu as default setting
    gpioset `gpiofind I3C_MUX_SELECT`=0
fi

psu1_present=$(gpioget `gpiofind PSU0_CPLD_PRESENT_N`)
if [ "$psu1_present" = "0" ]; then
    echo "PSU1 pluged"
    systemctl start inv-psu-update@11plug.service
else
    echo "PSU1 unpluged"
    systemctl start inv-psu-update@11unplug.service
fi

psu2_present=$(gpioget `gpiofind PSU1_CPLD_PRESENT_N`)
if [ "$psu2_present" = "0" ]; then
    echo "PSU2 pluged"
    systemctl start inv-psu-update@12plug.service
else
    echo "PSU2 unpluged"
    systemctl start inv-psu-update@12unplug.service
fi

