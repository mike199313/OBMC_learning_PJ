#pragma once

#include <iostream>
#include <vector>
#include <filesystem>
#include <regex>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdbusplus/bus.hpp>


using DbusObjectPath = std::string;
using DbusService = std::string;
using DbusInterface = std::string;
using DbusObjectInfo = std::pair<DbusObjectPath, DbusService>;
using DbusProperty = std::string;
using Value = std::variant<bool, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
      int64_t, uint64_t, double, std::string>;
using PropertyMap = std::map<DbusProperty, Value>;

using ObjectTree =
    std::map<DbusObjectPath, std::map<DbusService, std::vector<DbusInterface>>>;

using InterfaceList = std::vector<std::string>;

using DbusInterfaceMap = std::map<DbusInterface, PropertyMap>;

using ObjectValueTree =
    std::map<sdbusplus::message::object_path, DbusInterfaceMap>;

constexpr auto MAPPER_BUS_NAME = "xyz.openbmc_project.ObjectMapper";
constexpr auto MAPPER_OBJ = "/xyz/openbmc_project/object_mapper";
constexpr auto MAPPER_INTF = "xyz.openbmc_project.ObjectMapper";

constexpr auto PROP_INTERFACE = "org.freedesktop.DBus.Properties";
constexpr auto METHOD_SET = "Set";
constexpr auto METHOD_GET_ALL = "GetAll";
constexpr auto METHOD_GET = "Get";

constexpr auto HOST_STATE_PATH = "/xyz/openbmc_project/state/host0";
constexpr auto HOST_INTERFACE = "xyz.openbmc_project.State.Host";

constexpr auto IPMI_SEL_Service = "xyz.openbmc_project.Logging.IPMI";
constexpr auto IPMI_SEL_Path = "/xyz/openbmc_project/Logging/IPMI";
constexpr auto IPMI_SEL_Add_Interface = "xyz.openbmc_project.Logging.IPMI";

constexpr std::chrono::microseconds GET_DBUS_TIMEOUT = std::chrono::microseconds(5 * 1000000);

/**
 * @brief Get current timestamp in milliseconds.
 *
 * @param[in] Null.
 * @return current timestamp in milliseconds.
 */
double getCurrentTimeWithMs()
{
    time_t s;
    long ms;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999) {
        s++;
        ms = 0;
    }

    double result = (intmax_t)s + ((double)ms / 1000);

    return result;
}

/**
 * @brief Get the DBUS Service name for the input dbus path.
 * @param[in] bus - DBUS Bus Object.
 * @param[in] intf - DBUS Interface.
 * @param[in] path - DBUS Object Path.
 */
std::string getService(std::shared_ptr<sdbusplus::asio::connection> &bus, const std::string &intf,
                       const std::string &path)
{
    auto mapperCall =
        bus->new_method_call("xyz.openbmc_project.ObjectMapper",
                             "/xyz/openbmc_project/object_mapper",
                             "xyz.openbmc_project.ObjectMapper", "GetObject");

    mapperCall.append(path);
    mapperCall.append(std::vector<std::string>({intf}));

    auto mapperResponseMsg = bus->call(mapperCall);

    if (mapperResponseMsg.is_method_error()) {
        throw std::runtime_error("ERROR in mapper call");
    }

    std::map<std::string, std::vector<std::string>> mapperResponse;
    mapperResponseMsg.read(mapperResponse);

    if (mapperResponse.begin() == mapperResponse.end()) {
        throw std::runtime_error("ERROR in reading the mapper response");
    }

    return mapperResponse.begin()->first;
}


Value getDbusProperty(std::shared_ptr<sdbusplus::asio::connection> &bus, const std::string& service,
                      const std::string& objPath, const std::string& interface,
                      const std::string& property,
                      std::chrono::microseconds timeout = GET_DBUS_TIMEOUT)
{

    Value value;

    auto method = bus->new_method_call(service.c_str(), objPath.c_str(),
                                      PROP_INTERFACE, METHOD_GET);

    method.append(interface, property);

    auto reply = bus->call(method, timeout.count());

    if (reply.is_method_error())
    {
        fprintf(stderr, "%s getDbusProperty %s, %s, %s, %s failed ",
             __func__, service.c_str(), objPath.c_str(), interface.c_str(), property.c_str());
    }

    reply.read(value);

    return value;
}

PropertyMap getAllDbusProperties(std::shared_ptr<sdbusplus::asio::connection> &bus,
                                 const std::string &service,
                                 const std::string &objPath,
                                 const std::string &interface,
                                 std::chrono::microseconds timeout)
{
    PropertyMap properties;

    auto method = bus->new_method_call(service.c_str(), objPath.c_str(),
                                       PROP_INTERFACE, METHOD_GET_ALL);

    method.append(interface);

    auto reply = bus->call(method, timeout.count());

    if (reply.is_method_error()) {
        std::cerr << "Failed to get all properties\n";
    }

    reply.read(properties);
    return properties;
}


ObjectTree getAllDbusObjects(std::shared_ptr<sdbusplus::asio::connection> &bus,
                                   const std::string& serviceRoot,
                                   const std::string& interface,
                                   const std::string& match)
{
    std::vector<std::string> interfaces;
    interfaces.emplace_back(interface);

    auto depth = 0;

    auto mapperCall = bus->new_method_call(MAPPER_BUS_NAME, MAPPER_OBJ,
                                          MAPPER_INTF, "GetSubTree");

    mapperCall.append(serviceRoot, depth, interfaces);

    auto mapperReply = bus->call(mapperCall);
    if (mapperReply.is_method_error())
    {
        fprintf(stderr, "%s  %s, %s failed ",
             __func__, serviceRoot.c_str(), interface.c_str());
    }

    ObjectTree objectTree;
    mapperReply.read(objectTree);

    for (auto it = objectTree.begin(); it != objectTree.end();)
    {
        if (it->first.find(match) == std::string::npos)
        {
            it = objectTree.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return objectTree;
}


void setDbusProperty(std::shared_ptr<sdbusplus::asio::connection> &bus, const std::string &service,
                     const std::string &objPath, const std::string &interface,
                     const std::string &property, const Value &value)
{
    auto method = bus->new_method_call(service.c_str(), objPath.c_str(),
                                      PROP_INTERFACE, METHOD_SET);

    method.append(interface, property, value);

    if (!bus->call(method))
    {
        std::cerr << "Failed to set properties\n";
    }
}

void setPowerControlReturnCode(std::shared_ptr<sdbusplus::asio::connection> &bus, int value)
{
    try
    {
        const std::string objPath = "/xyz/openbmc_project/state/chassis0";
        const std::string intf = "xyz.openbmc_project.State.Chassis";
        auto service = getService(bus, intf, objPath);
        setDbusProperty(bus, service, objPath, intf,
                              "PowerControlReturnCode", value);
    }
    catch (sdbusplus::exception::SdBusError &e)
    {
        fprintf(stderr, "exception:%s \n", e.what());
    }
}

void setLastPowerEvent(std::shared_ptr<sdbusplus::asio::connection> &bus, uint32_t value)
{
    try
    {
        const std::string objPath = "/xyz/openbmc_project/state/chassis0";
        const std::string intf = "xyz.openbmc_project.State.Chassis";
        auto service = getService(bus, intf, objPath);
        setDbusProperty(bus, service, objPath, intf, "LastPowerEvent", value);
    }
    catch (sdbusplus::exception::SdBusError &e)
    {
        fprintf(stderr, "exception:%s \n", e.what());
    }
}

/**
 * @brief Generate SEL event.
 *
 * @param[in] bus - DBUS Bus Object.
 * @param[in] SensorPath - Dbus service name of sensor.
 * @param[in] eventData - SEL event data 1 to 3.
 */
void generateSELEvent(std::shared_ptr<sdbusplus::asio::connection> &bus,
                      char const *SensorPath, std::vector<uint8_t> eventData, bool assert)
{
    uint16_t generateID = 0x20;

    sdbusplus::message::message writeSEL = bus->new_method_call(
            IPMI_SEL_Service, IPMI_SEL_Path, IPMI_SEL_Add_Interface, "IpmiSelAdd");
    writeSEL.append("SEL Entry", std::string(SensorPath), eventData, assert, generateID);
    try {
        bus->call(writeSEL);
    } catch (sdbusplus::exception_t &e) {
        std::cerr << "call IpmiSelAdd failed\n";
    }
}

/**
 * @brief Control host power state.
 *
 * @param[in] bus - DBUS Bus Object.
 * @param[in] control - Power state.
 */
void hostPowerControl(std::shared_ptr<sdbusplus::asio::connection> &bus, std::string control)
{
    try {
        auto service = getService(bus, HOST_INTERFACE, HOST_STATE_PATH);

        auto method = bus->new_method_call(
                          service.c_str(), HOST_STATE_PATH, PROP_INTERFACE, METHOD_SET);
        method.append(HOST_INTERFACE, "RequestedHostTransition",
                      std::variant<std::string>(control));

        bus->call(method);
    } catch (sdbusplus::exception_t &e) {
        std::cerr << "Failed to change power state " << control << "\n";
    }
}


std::optional<bool> isPSUInputPowerOK(std::shared_ptr<sdbusplus::asio::connection> &bus)
{
    bool isPSUInputPowerOK = true;

    constexpr const char* PSUInputPowerObjectPath =
        "/xyz/openbmc_project/sensors/power";
    constexpr const char* PSUInputPowerIntf =
        "xyz.openbmc_project.Sensor.Value";
    ObjectTree objectTree;

    try{
        objectTree =
            getAllDbusObjects(bus, PSUInputPowerObjectPath,
                                    PSUInputPowerIntf, "Input_Power");
    }catch(std::exception& e){
        fprintf(stderr, "%s : Exception %s \n", __func__, e.what());
        return std::nullopt;
    }
    
    for (auto& treeItr : objectTree)
    {
        std::string objPath;

        objPath = treeItr.first;
        auto& serviceMap = treeItr.second;
        for (auto& itr : serviceMap)
        {
            try
            {
                Value v;
                auto service = itr.first;
                v = getDbusProperty(bus, service, objPath,
                                          PSUInputPowerIntf, "Value");
                auto value = std::get<double>(v);

                if( isnan(value) ){
                    fprintf(stderr, "Checking input power value of %s, but got nan (not valid).\n", objPath.c_str() );
                    return std::nullopt;
                }

                v = getDbusProperty(bus, service, objPath,
                                          PSUInputPowerIntf, "MinValue");
                auto minValue = std::get<double>(v);

                v = getDbusProperty(bus, service, objPath,
                                          PSUInputPowerIntf, "MaxValue");
                auto maxValue = std::get<double>(v);

                // in transformers platform, the RPM of fan is not measured
                // well. So the value may be larger then maxValue The threshold
                // value of the RPM will be defined by software.
                if (value <= maxValue && value >= minValue)
                {
                    isPSUInputPowerOK &= true;
                }
                else
                {
                    isPSUInputPowerOK &= false;
                }
            }
            catch (const std::exception& e)
            {
                fprintf(stderr, "%s objPath=%s Exception:%s \n", __func__,
                        objPath.c_str(), e.what());
                return std::nullopt;
            }
        }
    }
    return std::make_optional<bool>(isPSUInputPowerOK);
}

std::optional<bool> isPSUOutputPowerOK(std::shared_ptr<sdbusplus::asio::connection> &bus)
{
    bool isPSUOutputPowerOK = true;

    constexpr const char* PSUOutputPowerObjectPath =
        "/xyz/openbmc_project/sensors/power";
    constexpr const char* PSUOutputPowerIntf =
        "xyz.openbmc_project.Sensor.Value";
    ObjectTree objectTree;

    try
    {
        objectTree = getAllDbusObjects(
            bus, PSUOutputPowerObjectPath, PSUOutputPowerIntf, "Output_Power");
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "%s : Exception %s \n", __func__, e.what());
        return std::nullopt;
    }

    for (auto& treeItr : objectTree)
    {
        std::string objPath;

        objPath = treeItr.first;
        auto& serviceMap = treeItr.second;
        for (auto& itr : serviceMap)
        {
            try
            {
                Value v;
                auto service = itr.first;
                v = getDbusProperty(bus, service, objPath,
                                          PSUOutputPowerIntf, "Value");
                auto value = std::get<double>(v);

                v = getDbusProperty(bus, service, objPath,
                                          PSUOutputPowerIntf, "MinValue");
                auto minValue = std::get<double>(v);

                v = getDbusProperty(bus, service, objPath,
                                          PSUOutputPowerIntf, "MaxValue");
                auto maxValue = std::get<double>(v);

                if( isnan(value) ){
                    fprintf(stderr, "Checking output power value of %s, but got nan (not valid).\n", objPath.c_str() );
                    return std::nullopt;
                }

                // in transformers platform, the RPM of fan is not measured
                // well. So the value may be larger then maxValue The threshold
                // value of the RPM will be defined by software.
                if (value <= maxValue && value >= minValue)
                {
                    isPSUOutputPowerOK &= true;
                }
                else
                {
                    isPSUOutputPowerOK &= false;
                }
            }
            catch (const std::exception& e)
            {
                fprintf(stderr, "%s objPath=%s Exception:%s \n", __func__,
                        objPath.c_str(), e.what());
                return std::nullopt;
            }
        }
    }
    return std::make_optional<bool>(isPSUOutputPowerOK);
}


std::optional<bool> isPSUPowerOK(std::shared_ptr<sdbusplus::asio::connection> &bus)
{
    auto psuInput = isPSUInputPowerOK(bus);
    auto psuOutput = isPSUOutputPowerOK(bus);
    if(psuInput && psuOutput){
        return std::make_optional<bool>(*psuInput && *psuOutput);
    }else{
        return std::nullopt;
    }

}


