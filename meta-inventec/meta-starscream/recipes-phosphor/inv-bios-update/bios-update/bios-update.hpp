#pragma once

#include <stdint.h>
#include <vector>
#include <xyz/openbmc_project/State/Host/server.hpp>

#define BIOS_SPI "1e630000.spi"
#define BIOS_DRIVER_PATH "/sys/bus/platform/drivers/aspeed-smc/"
constexpr uint32_t biosFileSize = (16*1024*1024); // 64M ; platform dependent
const uint32_t CHIP_BUFFER_SIZE = 32;
constexpr const char* spiLineName = "SPI_MUX_SELECT";
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
    int8_t setBiosMtdDevice(uint8_t state);
    int8_t getMtdNum();
};
