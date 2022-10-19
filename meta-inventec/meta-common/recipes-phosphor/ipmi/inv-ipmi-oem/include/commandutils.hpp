/*
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#pragma once

#include <ipmid/api.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>

#include <iostream>
#include <string>
#include <stdexcept>

constexpr char SYSTEMD_BUSNAME[] = "org.freedesktop.systemd1";
constexpr char SYSTEMD_PATH[] = "/org/freedesktop/systemd1";
constexpr char SYSTEMD_INTERFACE[] = "org.freedesktop.systemd1.Manager";

static constexpr bool debug = false;

inline static void printRegistration(unsigned int netfn, unsigned int cmd)
{
    if constexpr (debug)
    {
        std::cout << "Registering NetFn:[0x" << std::hex << std::uppercase
                  << netfn << "], Cmd:[0x" << cmd << "]\n";
    }
}

#define registerOemCmdHandler(netfn, cmd, priv, handler) \
do  \
{   \
    printRegistration((netfn), (cmd)); \
    if (false == ipmi::registerHandler(ipmi::prioOemBase, (netfn), (cmd), (priv), (handler))) \
    { \
        phosphor::logging::log<phosphor::logging::level::ERR>( \
            "Failed to register ", \
            phosphor::logging::entry("NetFn:[0x%x], ", netfn), \
            phosphor::logging::entry("Cmd:[0x%x]", cmd)); \
    } \
} while(0)

inline static void triggerUnit(sdbusplus::bus::bus& bus, const std::string& unit, bool isStart)
{
    const char* function = isStart ? "StartUnit" : "StopUnit";
    try
    {
        auto method = bus.new_method_call(SYSTEMD_BUSNAME, SYSTEMD_PATH,
                                          SYSTEMD_INTERFACE, function);
        method.append(unit.c_str(), "replace");
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>("Failed to restart service",
                        phosphor::logging::entry("ERR=%s", ex.what()),
                        phosphor::logging::entry("UNIT=%s", unit.c_str()));
    }
}
