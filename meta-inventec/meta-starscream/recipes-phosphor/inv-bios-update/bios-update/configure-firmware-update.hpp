#include <sdbusplus/bus.hpp>
#include <sdbusplus/message/types.hpp>
#include <systemd/sd-bus.h>
#include <iostream>
#include <unistd.h>
#include <variant>

using ConfFirmValue = std::variant<uint8_t, bool>;
constexpr uint8_t FW_IMAGE_NOT_FOUND = 0x01;
constexpr uint8_t FW_UPDATE_NOT_STARTED = 0x02;
constexpr uint8_t FW_UPDATE_IN_PROGRESS = 0x03;
constexpr uint8_t FW_UPDATE_COMPLETED_SUCCESSFULLY = 0x04;
constexpr uint8_t FW_UPDATE_ABORTED = 0x05;
constexpr uint8_t FW_IMAGE_CORRUPTED = 0x06;
constexpr uint8_t FW_UPDATE_INITIATE_ERROR = 0x07;
constexpr uint8_t COMPONENT_FAULT_ERROR = 0x08;

constexpr char const* CONFIGURE_FIRMWARE_SERVICE = "xyz.openbmc_project.configurefirmware";
constexpr char const* CONFIGURE_FIRMWARE_PATH = "/xyz/openbmc_project/configurefirmware/bios";
constexpr char const* SYSTEMD_PRO_INTERFACE = "org.freedesktop.DBus.Properties";
constexpr char const* CONFIGURE_FIRMWARE_INTERFACE = "xyz.openbmc_project.configurefirmware.status";

constexpr auto METHOD_SET = "Set";
constexpr auto METHOD_GET = "Get";

const int64_t abortResponseTime = 10;

static bool is_oemcmd = false;

void setProperty(std::string property, ConfFirmValue targetStatus);
ConfFirmValue getProperty(std::string property);
