#include "configuration_manager.hpp"
#include "watch.hpp"

#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>

const constexpr char *SOFTWARE_OBJPATH = "/xyz/openbmc_project/configurtion";
const constexpr char *BUSNAME = "xyz.openbmc_project.ConfigurationManager";

int main()
{
    using namespace ipmi::inv::configuration;
    auto bus = sdbusplus::bus::new_default();
    sd_event *loop = nullptr;
    sd_event_default(&loop);

    sdbusplus::server::manager::manager objManager(bus, SOFTWARE_OBJPATH);
    bus.request_name(BUSNAME);

    try
    {
        ipmi::inv::configuration::Manager configManager(bus);
        ipmi::inv::configuration::Watch watch(
            loop, std::bind(std::mem_fn(&Manager::processConfigurations), &configManager,
                            std::placeholders::_1));
        bus.attach_event(loop, SD_EVENT_PRIORITY_NORMAL);
        sd_event_loop(loop);
    }
    catch (const std::exception &e)
    {
        using namespace phosphor::logging;
        log<level::ERR>(e.what());
        std::cerr << e.what() << std::endl;
        return -1;
    }
    sd_event_unref(loop);

    return 0;
}
