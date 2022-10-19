#include "inventec-dcmi-power.hpp"
#include "util.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;

static constexpr bool DEBUG = false;

std::shared_ptr<sdbusplus::asio::connection> bus;
std::shared_ptr<sdbusplus::asio::dbus_interface> powerInterface;
PowerStore powerStore;

void setPowerPath(PowerStore &powerStore)
{
    std::ifstream sensorFile(POWER_READING_SENSOR);
    std::string objectPath;
    if (!sensorFile.is_open())
    {
        std::cerr << "Power reading configuration file not found\n";
        throw std::runtime_error("error");
    }

    auto data = nlohmann::json::parse(sensorFile, nullptr, false);
    if (data.is_discarded())
    {
        std::cerr << "Error in parsing configuration file\n";
        throw std::runtime_error("error");
    }

    objectPath = data.value("path", "");
    if (objectPath.empty())
    {
        std::cerr << "Power sensor D-Bus object path is empty\n";
        throw std::runtime_error("error");
    }

    powerStore.powerPath = objectPath;
}

double readPower(void)
{
    double value;
    try
    {
        auto service = getService(bus, SENSOR_VALUE_INTF, powerStore.powerPath);

        // Read the sensor value and scale properties
        auto properties = getAllDbusProperties(bus, service, powerStore.powerPath,
                                               SENSOR_VALUE_INTF, DBUS_TIMEOUT);

        value = std::get<double>(properties[SENSOR_VALUE_PROP]);
    }
    catch (std::exception &e)
    {
        std::cerr << "Failure to read power value from D-Bus object" << powerStore.powerPath << "\n";
        throw std::runtime_error("Failure to read power value from D-Bus object");
    }
    return value;
}

void power_cap_properties_init(void)
{
    try
    {
        auto service = getService(bus, PCAP_INTERFACE, PCAP_PATH);
        auto properties = getAllDbusProperties(bus, service, PCAP_PATH,
                                               PCAP_INTERFACE, DBUS_TIMEOUT);
        powerStore.powerCap = std::get<uint32_t>(properties[POWER_CAP_PROP]);
        powerStore.powerCapEnable = std::get<bool>(properties[POWER_CAP_ENABLE_PROP]);
        powerStore.exceptionAction = std::get<std::string>(properties[EXCEPTION_ACTION_PROP]);
        powerStore.correctionTime = std::get<uint32_t>(properties[CORRECTION_TIME_PROP]);
        powerStore.samplingPeriod = std::get<uint32_t>(properties[SAMPLING_PERIOD_PROP]);
    }
    catch (...)
    {
        std::cerr << "power_cap_properties_init fail\n";
        throw std::runtime_error("error");
    }

    if constexpr (DEBUG)
    {
        std::cerr << "Properties init:\n";
        std::cerr << "PowerStore.powerCap: " << powerStore.powerCap << "\n";
        std::cerr << "PowerStore.powerCapEnable: " << powerStore.powerCapEnable << "\n";
        std::cerr << "PowerStore.exceptionAction: " << powerStore.exceptionAction << "\n";
        std::cerr << "PowerStore.correctionTime: " << powerStore.correctionTime << "\n";
        std::cerr << "PowerStore.samplingPeriod: " << powerStore.samplingPeriod << "\n";
    }
}

inline static sdbusplus::bus::match::match
startPowerCapMonitor(std::shared_ptr<sdbusplus::asio::connection> conn, PowerStore &powerStore)
{
    auto powerCapMatcherCallback = [&](sdbusplus::message::message &msg)
    {
        std::string interface;
        boost::container::flat_map<std::string, std::variant<std::string, uint16_t, bool, uint32_t>> propertiesChanged;
        msg.read(interface, propertiesChanged);
        std::string event = propertiesChanged.begin()->first;

        if (propertiesChanged.empty() || event.empty())
        {
            return;
        }

        if (event == POWER_CAP_PROP)
        {
            auto value = std::get_if<uint32_t>(&propertiesChanged.begin()->second);
            powerStore.powerCap = *value;
        }
        else if (event == POWER_CAP_ENABLE_PROP)
        {
            auto value = std::get_if<bool>(&propertiesChanged.begin()->second);
            powerStore.powerCapEnable = *value;
            /* Clean status if powerCap disable*/
            if(!powerStore.powerCapEnable)
            {
                powerStore.actionTriggered = false;
                powerStore.correctionStart = false;
            }
        }
        else if (event == EXCEPTION_ACTION_PROP)
        {
            auto value = std::get_if<std::string>(&propertiesChanged.begin()->second);
            powerStore.exceptionAction = *value;
        }
        else if (event == CORRECTION_TIME_PROP)
        {
            auto value = std::get_if<uint32_t>(&propertiesChanged.begin()->second);
            powerStore.correctionTime = *value;
        }
        else if (event == SAMPLING_PERIOD_PROP)
        {
            auto value = std::get_if<uint32_t>(&propertiesChanged.begin()->second);
            powerStore.samplingPeriod = *value;
        }
        else
        {
            std::cerr << "Not supported properties changed event: " << event << "\n";
        }

        if constexpr (DEBUG)
        {
            std::cerr << "Properties changed event: " << event << "\n";
            std::cerr << "PowerStore.samplingPeriod: " << powerStore.samplingPeriod << "\n";
            std::cerr << "PowerStore.powerCapEnable: " << powerStore.powerCapEnable << "\n";
            std::cerr << "PowerStore.correctionTime: " << powerStore.correctionTime << "\n";
            std::cerr << "PowerStore.exceptionAction: " << powerStore.exceptionAction << "\n";
            std::cerr << "PowerStore.powerCap: " << powerStore.powerCap << "\n";
        }
    };

    sdbusplus::bus::match::match powerCapMatcher(
        static_cast<sdbusplus::bus::bus &>(*conn),
        "type='signal',interface='org.freedesktop.DBus.Properties',member='"
        "PropertiesChanged',arg0namespace='xyz.openbmc_project.Control.Power.Cap'",
        std::move(powerCapMatcherCallback));

    return powerCapMatcher;
}

void savePowerThresholdEventSEL(bool assert)
{
    std::vector<uint8_t> powerThresholdEventData{0x01, 0xFF, 0xFF};
    generateSELEvent(bus, POWER_THRESHOLD_EVENT_SENSOR_PATH, powerThresholdEventData, assert);
}

void savePowerOffEventSEL(bool assert)
{
    std::vector<uint8_t> powerOffEventData{0x02, 0xFF, 0xFF};
    generateSELEvent(bus, POWER_OFF_EVENT_SENSOR_PATH, powerOffEventData, assert);
}


void powerFaultCheck(void)
{
    auto _isPSUPowerOK = isPSUPowerOK(bus);
    if(_isPSUPowerOK){
        bool powerFault = !((_isPSUPowerOK) ? *_isPSUPowerOK : false);
        if (powerFault){
            savePowerThresholdEventSEL(true);
            savePowerOffEventSEL(true);
            // chassis status : power off due to power fault
            hostPowerControl(bus, "xyz.openbmc_project.State.Host.Transition.Off");
            setLastPowerEvent(bus, 1 << 3);
        }
    }else{
        fprintf(stderr,"%s: Got null pointer\n", __FUNCTION__);
    }
}

void powerLimitCheck(Power currentPower)
{
    bool action = false;

    if (powerStore.actionTriggered)
    {
        /* action has triggered, only check is threshold gets down */
        
        if (currentPower.value <= powerStore.powerCap)
        {
            if (powerStore.exceptionAction !=
                "xyz.openbmc_project.Control.Power.Cap.Action.None")
            {
                /* Save deassert SEL for Power Threshold */
                savePowerThresholdEventSEL(false);
            }
            powerStore.actionTriggered = false;
            if constexpr (DEBUG)
            {
                std::cerr << "actionTriggered deassert\n";
            }
        }
    }
    else
    {
        /* Only check condition if action not triggered */

        if (powerStore.correctionStart)
        {
            if (currentPower.value <= powerStore.powerCap)
            {
                /* Power below the threshold, reset correctionStart*/
                powerStore.correctionStart = false;
                if constexpr (DEBUG)
                {
                    std::cerr << "correctionStart deassert\n";
                }
            }
            else
            {
                /* Check is correctionTime expire, if not, wait for next check*/
                if (powerStore.correctionTimeout < currentPower.time)
                {
                    /* Set action, and clean correctionStart */
                    action = true;
                    powerStore.correctionStart = false;
                }
            }
        }
        else
        {
            if (currentPower.value > powerStore.powerCap)
            {
                /* First time exceed the threshold */
                if (powerStore.correctionTime == 0)
                {
                    /* Action immediately*/
                    action = true;
                }
                else
                {
                    /* Updatet the correctionTimeout*/
                    powerStore.correctionTimeout =
                        currentPower.time + powerStore.correctionTime;
                    powerStore.correctionStart = true;
                    if constexpr (DEBUG)
                    {
                        std::cerr << "correctionStart assert\n";
                    }
                }
            }
        }
    }

    if (action)
    {
        if (powerStore.exceptionAction == "xyz.openbmc_project.Control.Power.Cap.Action.None")
        {
            /* Do nothig just save log */
            std::cerr << "Action for power limit: None\n";
        }

        if (powerStore.exceptionAction == "xyz.openbmc_project.Control.Power.Cap.Action.OffAndLog")
        {
            std::cerr << "Action for power limit: Power Off and generate SEL event\n";

            savePowerThresholdEventSEL(true);
            savePowerOffEventSEL(true);
            //chassis status : power off due to power overload
            hostPowerControl(bus, "xyz.openbmc_project.State.Host.Transition.Off");
            setLastPowerEvent(bus, 1<<1);
        }

        if (powerStore.exceptionAction == "xyz.openbmc_project.Control.Power.Cap.Action.Log")
        {
            std::cerr << "Action for power limit: Generate SEL event\n";
            savePowerThresholdEventSEL(true);
        }

        /* save actionTriggered flag */
        powerStore.actionTriggered = true;
        if constexpr (DEBUG)
        {
            std::cerr << "actionTriggered\n";
        }
    }

    return;
}

void powerHandler(boost::asio::io_context &io, PowerStore &powerStore, double delay)
{
    static boost::asio::steady_timer timer(io);

    timer.expires_after(std::chrono::microseconds((long)delay));

    timer.async_wait([&io, &powerStore](const boost::system::error_code &)
    {
        double start, end, delayTime;
        Power currentPower;

        start = getCurrentTimeWithMs();

        currentPower.time = start * MILLI_OFFSET;

        try
        {
            currentPower.value = readPower();
        }
        catch (...)
        {
            end = getCurrentTimeWithMs();
            delayTime = (SAMPLING_INTERVEL - (end - start)) * MICRO_OFFSET;
            if (delayTime < 0)
            {
                delayTime = 0;
            }

            powerHandler(io, powerStore, delayTime);
            return;
        }

        if (powerStore.collectedPower.size() >= MAX_COLLECTION_POWER_SIZE)
        {
            powerStore.collectedPower.erase(powerStore.collectedPower.begin());
            powerStore.collectedPower.push_back(currentPower);
        }
        else
        {
            powerStore.collectedPower.push_back(currentPower);
        }

        if (powerStore.powerCapEnable)
        {
            powerLimitCheck(currentPower);
        }

        powerFaultCheck();

        end = getCurrentTimeWithMs();
        delayTime = (SAMPLING_INTERVEL - (end - start)) * MICRO_OFFSET;
        if (delayTime < 0)
        {
            delayTime = 0;
        }

        powerHandler(io, powerStore, delayTime);
        return;
    });
}

double get_power(void)
{
    Power currentPower;

    if (powerStore.collectedPower.size() == 0)
    {
        currentPower.time = getCurrentTimeWithMs() * MILLI_OFFSET;

        try
        {
            currentPower.value = readPower();
        }
        catch (...)
        {
            std::cerr << "cannot get power value\n";
            return 0;
        }
        powerStore.collectedPower.push_back(currentPower);
    }
    else
    {
        currentPower = *powerStore.collectedPower.rbegin();
    }

    return currentPower.value;
}

double get_power_max(void)
{
    double max = 0;
    double currentTime = getCurrentTimeWithMs() * MILLI_OFFSET;
    bool init = true;

    for (auto it = powerStore.collectedPower.rbegin(); it != powerStore.collectedPower.rend(); it++)
    {
        if (init)
        {
            max = it->value;
            init = false;
        }

        if (it->time + (powerStore.samplingPeriod * MILLI_OFFSET) < currentTime)
        {
            break;
        }
        if (max < it->value)
        {
            max = it->value;
        }
    }

    return max;
}

double get_power_min(void)
{
    double min = 0;
    double currentTime = getCurrentTimeWithMs() * MILLI_OFFSET;
    bool init = true;

    for (auto it = powerStore.collectedPower.rbegin(); it != powerStore.collectedPower.rend(); it++)
    {
        if (init)
        {
            min = it->value;
            init = false;
        }
        if (it->time + (powerStore.samplingPeriod * MILLI_OFFSET) < currentTime)
        {
            break;
        }
        if (min > it->value)
        {
            min = it->value;
        }
    }

    return min;
}

double get_power_average(void)
{
    double average = 0;
    uint32_t averageCount = 0;
    double currentTime = getCurrentTimeWithMs() * MILLI_OFFSET;

    for (auto it = powerStore.collectedPower.rbegin(); it != powerStore.collectedPower.rend(); it++)
    {
        if (it->time + (powerStore.samplingPeriod * MILLI_OFFSET) < currentTime)
        {
            if (averageCount)
            {
                average = average / averageCount;
            }
            else
            {
                average = powerStore.collectedPower.rbegin()->value;
            }
            break;
        }
        average = average + it->value;
        averageCount++;
    }

    return average;
}

uint32_t get_last_sample_time(void)
{
    Power currentPower;

    if (powerStore.collectedPower.size() == 0)
    {
        currentPower.time = getCurrentTimeWithMs() * MILLI_OFFSET;

        try
        {
            currentPower.value = readPower();
        }
        catch (...)
        {
            std::cerr << "cannot get power value\n";
            return 0;
        }
        powerStore.collectedPower.push_back(currentPower);
    }
    else
    {
        currentPower = *powerStore.collectedPower.rbegin();
    }

    return (uint32_t)currentPower.time / MILLI_OFFSET;
}

uint32_t get_average_count(void)
{
    uint32_t averageCount = 0;

    double currentTime = getCurrentTimeWithMs() * MILLI_OFFSET;

    for (auto it = powerStore.collectedPower.rbegin(); it != powerStore.collectedPower.rend(); it++)
    {
        if (it->time + (powerStore.samplingPeriod * MILLI_OFFSET) < currentTime)
        {
            break;
        }
        averageCount++;
    }

    return averageCount;
}

void dcmi_power_interface_init(void)
{
    bus->request_name(DCMI_SERVICE);

    sdbusplus::asio::object_server objectServer(bus);

    /* Init property setting*/
    powerInterface = objectServer.add_interface(DCMI_POWER_PATH, DCMI_POWER_INTERFACE);

    powerInterface->register_property_r(
        "TotalPower", double(),
        sdbusplus::vtable::property_::emits_change,
        [](const auto &)
        {
            return get_power();
        });

    powerInterface->register_property_r(
        "MaxValue", double(),
        sdbusplus::vtable::property_::emits_change,
        [](const auto &)
        {
            return get_power_max();
        });

    powerInterface->register_property_r(
        "MinValue", double(),
        sdbusplus::vtable::property_::emits_change,
        [](const auto &)
        {
            return get_power_min();
        });

    powerInterface->register_property_r(
        "AverageValue", double(),
        sdbusplus::vtable::property_::emits_change,
        [](const auto &)
        {
            return get_power_average();
        });

    powerInterface->register_property_r(
        "LastSampleTime", uint32_t(),
        sdbusplus::vtable::property_::emits_change,
        [](const auto &)
        {
            return get_last_sample_time();
        });

    powerInterface->register_property_r(
        "AverageCount", uint32_t(),
        sdbusplus::vtable::property_::emits_change,
        [](const auto &)
        {
            return get_average_count();
        });

    powerInterface->initialize();

    return;
}

int main(void)
{
    boost::asio::io_context io;
    bus = std::make_shared<sdbusplus::asio::connection>(io);

    try
    {
        setPowerPath(powerStore);
        power_cap_properties_init();
        dcmi_power_interface_init();
    }
    catch (std::exception &e)
    {
        return -1;
    }

    sdbusplus::bus::match::match powerCapMonitor = startPowerCapMonitor(bus, powerStore);

    io.post(
        [&]()
        { powerHandler(io, powerStore, 0); });

    io.run();

    return 0;
}
