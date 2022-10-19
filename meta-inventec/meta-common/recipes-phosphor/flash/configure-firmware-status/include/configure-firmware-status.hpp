#pragma once

#include <iostream>
#include <string>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

constexpr auto CONFIGURE_FIRMWARE_SERVICE = "xyz.openbmc_project.configurefirmware";
const std::string CONFIGURE_FIRMWARE_PATH = "/xyz/openbmc_project/configurefirmware/";
constexpr auto CONFIGURE_FIRMWARE_INTERFACE = "xyz.openbmc_project.configurefirmware.status";

static constexpr uint8_t FW_IMAGE_NOT_FOUND = 0x01;
static constexpr uint8_t FW_UPDATE_NOT_STARTED = 0x02;
static constexpr uint8_t FW_UPDATE_IN_PROGRESS = 0x03;
static constexpr uint8_t FW_UPDATE_COMPLETED_SUCCESSFULLY = 0x04;
static constexpr uint8_t FW_UPDATE_ABORTED = 0x05;
static constexpr uint8_t FW_IMAGE_CORRUPTED = 0x06;
static constexpr uint8_t FW_UPDATE_INITIATE_ERROR = 0x07;
static constexpr uint8_t COMPONENT_FAULT_ERROR = 0x08;

typedef struct{
    std::string component;
    uint8_t status;
    bool force;
} property;
std::vector<property> configure = {{"psu", FW_UPDATE_NOT_STARTED, false}, {"bios", FW_UPDATE_NOT_STARTED, false},
    {"bmc", FW_UPDATE_NOT_STARTED, false}, {"cpld", FW_UPDATE_NOT_STARTED, false}};

void interface_init();
