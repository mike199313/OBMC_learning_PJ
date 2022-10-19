#pragma once

#include <iostream>
#include <vector>
#include <variant>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <unistd.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

constexpr auto MILLI_OFFSET = 1000;
constexpr auto MICRO_OFFSET = 1000000;
constexpr auto MAX_COLLECTION_POWER_SIZE = 86400; /*if interval is 1, sample for 24HR*/
constexpr auto SAMPLING_INTERVEL = 1; /*in sec*/

constexpr auto SENSOR_VALUE_INTF = "xyz.openbmc_project.Sensor.Value";
constexpr auto SENSOR_VALUE_PROP = "Value";
constexpr auto POWER_READING_SENSOR = "/usr/share/ipmi-providers/power_reading.json";

constexpr auto POWER_THRESHOLD_EVENT_SENSOR_PATH =
    "/xyz/openbmc_project/sensors/discrete_05h/power_unit/DCMI_Power_Threshold";
constexpr auto POWER_OFF_EVENT_SENSOR_PATH =
    "/xyz/openbmc_project/sensors/discrete_0ah/system_event/DCMI_Power_Off";

constexpr auto DCMI_SERVICE = "xyz.openbmc_project.DCMI";
constexpr auto DCMI_POWER_PATH = "/xyz/openbmc_project/DCMI/Power";
constexpr auto DCMI_POWER_INTERFACE = "xyz.openbmc_project.DCMI.Value";
constexpr auto PCAP_PATH = "/xyz/openbmc_project/control/host0/power_cap";
constexpr auto PCAP_INTERFACE = "xyz.openbmc_project.Control.Power.Cap";

constexpr auto PERIOD_MAX_PROP = "MaxValue";
constexpr auto PERIOD_MIN_PROP = "MinValue";
constexpr auto PERIOD_AVERAGE_PROP = "AverageValue";

constexpr auto POWER_CAP_PROP = "PowerCap";
constexpr auto POWER_CAP_ENABLE_PROP = "PowerCapEnable";
constexpr auto EXCEPTION_ACTION_PROP = "ExceptionAction";
constexpr auto CORRECTION_TIME_PROP = "CorrectionTime";
constexpr auto SAMPLING_PERIOD_PROP = "SamplingPeriod";

constexpr std::chrono::microseconds DBUS_TIMEOUT = std::chrono::microseconds(5 * 1000000);


typedef struct {
    double time = 0;
    double value = 0; /*in Watts*/
} Power;

/*
From Dbus interface setting:
exceptionAction
powerCap
powerCapEnable
samplingPeriod
correctionTime

Local variable:
correctionStart - Is correctionTime counting
correctionTimeout - The time that power execeed threshold and add correctionTime 
actionTriggered - Is the action has triggered
powerPath - dbus object path to read power value
collectedPower - The sampling result list
*/
typedef struct {
    std::string exceptionAction = "";
    uint32_t powerCap = 0;
    bool powerCapEnable = false;
    uint32_t samplingPeriod = 0;
    uint32_t correctionTime = 0;

    bool correctionStart = false;
    double correctionTimeout = 0;
    bool actionTriggered = false;

    std::string powerPath = "";
    std::vector<Power> collectedPower;
} PowerStore;

void savePowerThresholdEventSEL(bool assert);
void savePowerOffEventSEL(bool assert);

void setPowerPath(PowerStore& powerStore);
void power_cap_properties_init(void);
void dcmi_power_interface_init(void);

double readPower(void);
double get_power(void);
double get_power_max(void);
double get_power_min(void);
double get_power_average(void);
uint32_t get_last_sample_time(void);
uint32_t get_average_count(void);

void powerHandler(boost::asio::io_context& io, PowerStore& powerStore, double delay);
