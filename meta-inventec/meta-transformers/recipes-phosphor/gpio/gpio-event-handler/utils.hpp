#pragma once
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/message/types.hpp>

#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

struct DbusRspData
{
    std::vector<uint8_t> retData;
    boost::system::error_code ec;
};

// forward the request onto the main ipmi queue
using IpmiDbusRspType =
    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t, std::vector<uint8_t>>;

using IpmiDbusValueType =
    std::variant<bool, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t,
                 uint64_t, double, std::string>;



/**
 * @brief ipmi_method_call will using channel 8 to call the ipmi call.
 * The channel is always 8. and privilege is fixed at 4(Admin). 
 * The method is used as synchronize. It has its own asio service and bus object.
 * The API will wait for IPC completed for most 5000 milliseconds otherwise expired
 * and return
 * 
 * @param lun always be 0
 * @param netfn 
 * @param cmd 
 * @param cmdParameter content is determined by netfn/cmd
 * @param convey It take the return data and asio error code result
 */
void ipmi_method_call(uint8_t& lun, uint8_t& netfn, uint8_t& cmd,
                      std::vector<uint8_t>& cmdParameter,
                      std::shared_ptr<DbusRspData> convey);

void dbg_payload(std::vector<uint8_t>& data, const char* prompt);


void stopSystemdUnit(sdbusplus::bus::bus& bus, const std::string& unit);

void startSystemdUnit(sdbusplus::bus::bus& bus, const std::string& unit);
