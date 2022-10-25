#pragma once

#include <stdint.h>
#include <vector>
#include <xyz/openbmc_project/State/Host/server.hpp>

#define PROPERTY_INTERFACE "org.freedesktop.DBus.Properties"
#define POWER_STATE_OBJ "org.openbmc.control.Power"
#define POWER_STATE_PATH "/org/openbmc/control/power0"
#define POWER_STATE_INTERFACE "org.openbmc.control.Power"

#define HOST_STATE_OBJ "xyz.openbmc_project.State.Host"
#define HOST_STATE_PATH "/xyz/openbmc_project/state/host0"
#define HOST_STATE_INTERFACE "xyz.openbmc_project.State.Host"

#define IPMB_BRIDGE_OBJ "xyz.openbmc_project.Ipmi.Channel.Ipmb"
#define IPMB_BRIDGE_PATH "/xyz/openbmc_project/Ipmi/Channel/Ipmb"
#define IPMB_BRIDGE_INTERFACE "org.openbmc.Ipmb"

#define BIOS_SPI "c0000000.spi"
#define BIOS_DRIVER_PATH "/sys/bus/platform/drivers/NPCM-FIU/"
const uint32_t CHIP_BUFFER_SIZE = 32;
constexpr uint32_t biosFileSize = (64*1024*1024); // 64M ; platform dependent
constexpr const char* spiLineName = "SPI_BMC_PROG_CPLD_R_EN";
constexpr auto BIOS_MTD_NAME = "bios";
//#define DEBUG

#ifdef DEBUG
#define BIOS_UPDATE_DEBUG(fmt, args...) printf(fmt, ##args);
#else
#define BIOS_UPDATE_DEBUG(fmt, args...)
#endif

constexpr int MAX_RETRY_RECOVERY_MODE = 3;

// phosphor-dbus-interfaces
namespace State = sdbusplus::xyz::openbmc_project::State::server;

enum mtd_mount_state : uint8_t
{
    unbind = 0,
    bind
};

enum service_action : uint8_t
{
    SERVICE_STOP = 0,
    SERVICE_START
};

class BiosUpdateManager{

public:
    int8_t biosUpdatePrepare();
    int8_t biosUpdate(const char* image_str);
    int8_t biosUpdateFinished(const char* image_str);
    int8_t setMonitorProgress(std::string object, enum service_action action);
    //int8_t verify(const char* image_str, uint32_t imageSize);
private:
    int8_t setMeToRecoveryMode();
    int8_t resetMeToBoot();
    int8_t setBiosMtdDevice(uint8_t state);
    int8_t checkMeToRecoveryMode();
    bool isRecoveryMode;
    int8_t getMtdNum();
};
