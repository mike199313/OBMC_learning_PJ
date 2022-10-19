#include <ipmid/api.hpp>
#include <ipmid/utils.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message/types.hpp>
#include <phosphor-logging/log.hpp>
#include <systemd/sd-bus.h>
#include <iostream>
#include <unistd.h>

#include <boost/crc.hpp>  // for boost::crc_32_type
#include <vector>
#include <filesystem>
#include <fstream>
#include <variant>

namespace fs = std::filesystem;

namespace ipmi::inv::firmware
{
    using ConfFirmValue = std::variant<uint8_t, bool>;
    static constexpr uint8_t netFnConfigureFirmware = 0x38;
    static constexpr uint8_t cmdOemConfigureFirmware = 0x84;
    static constexpr uint8_t cmdOemBIOS = 0x02;
    static constexpr uint8_t splitIt = 0x3a;//char ':'

    static constexpr uint8_t START_FW_UPDATE = 0x01;
    static constexpr uint8_t ABORT_FW_UPDATE = 0x02;
    static constexpr uint8_t QUERY_FW_UPDATE = 0x03;
    static constexpr uint8_t FORCE_FW_UPDATE = 0x04;

    static constexpr uint8_t resultSuccess = 0x00;
    static constexpr uint8_t FW_IMAGE_NOT_FOUND = 0x01;
    static constexpr uint8_t FW_UPDATE_NOT_STARTED = 0x02;
    static constexpr uint8_t FW_UPDATE_IN_PROGRESS = 0x03;
    static constexpr uint8_t FW_UPDATE_COMPLETED_SUCCESSFULLY = 0x04;
    static constexpr uint8_t FW_UPDATE_ABORTED = 0x05;
    static constexpr uint8_t FW_IMAGE_CORRUPTED = 0x06;
    static constexpr uint8_t FW_UPDATE_INITIATE_ERROR = 0x07;
    static constexpr uint8_t COMPONENT_FAULT_ERROR = 0x08;

    static constexpr char const* CONFIGURE_FIRMWARE_SERVICE = "xyz.openbmc_project.configurefirmware";
    static constexpr char const* CONFIGURE_FIRMWARE_PATH = "/xyz/openbmc_project/configurefirmware/";
    static constexpr char const* SYSTEMD_PRO_INTERFACE = "org.freedesktop.DBus.Properties";
    static constexpr char const* CONFIGURE_FIRMWARE_INTERFACE = "xyz.openbmc_project.configurefirmware.status";
    constexpr auto METHOD_SET = "Set";
    constexpr auto METHOD_GET = "Get";

    static constexpr char const* SYSTEMD_BUSNAME = "org.freedesktop.systemd1";
    static constexpr char const* SYSTEMD_PATH = "/org/freedesktop/systemd1";
    static constexpr char const* SYSTEMD_INTERFACE = "org.freedesktop.systemd1.Manager";

    const int64_t buffer_size = 1024;
    static std::map<unsigned long, std::string> component = {{1, "psu"}, {2, "bios"}, 
        {3, "bmc"}, {4, "cpld"}};
    static std::map<std::string, std::string> startupService = {{"psu", ""},
        {"bios", "obmc-flash-host-bios@"}, {"bmc", ""}, {"cpld", ""}};

    const int64_t abortResponseTime = 10;

    ipmi::RspType<uint8_t> ipmiOemStartConfigureFirmware(ipmi::Context::ptr ctx, const uint8_t componentType, const uint8_t imageType, const uint8_t operation, const std::vector<uint8_t> &configure);
    bool validChecksum(std::string path, std::vector<uint8_t> checksum);
    void setProperty(unsigned long target, std::string property, ConfFirmValue targetStatus);
    ConfFirmValue getProperty(unsigned long target, std::string property);
    bool callFWUpdate(std::string target, std::string path);
}
