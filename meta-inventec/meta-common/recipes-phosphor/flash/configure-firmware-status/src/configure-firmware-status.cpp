#include <iostream>
#include <string>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include "configure-firmware-status.hpp"

boost::asio::io_context configureFirmwareIo;
std::shared_ptr<sdbusplus::asio::connection> configureFirmwareBus;
std::vector<std::shared_ptr<sdbusplus::asio::dbus_interface>> configureInterface;

void interface_init(void){
    configureFirmwareBus->request_name(CONFIGURE_FIRMWARE_SERVICE);
    sdbusplus::asio::object_server objectServer(configureFirmwareBus);
    int64_t configureSize = configure.size();
    for(int64_t i = 0;i < configureSize;i++){
        configureInterface.push_back(objectServer.add_interface(CONFIGURE_FIRMWARE_PATH + std::string(configure[i].component), CONFIGURE_FIRMWARE_INTERFACE));
        configureInterface[i]->register_property_rw(
        "status", uint8_t(),
        sdbusplus::vtable::property_::emits_change,
            [i](const auto &newPropertyValue, const auto&){
               configure[i].status = newPropertyValue;
               return 1;
            },[i](const auto &){return configure[i].status;}
        );
        configureInterface[i]->register_property_rw(
        "force", bool(),
        sdbusplus::vtable::property_::emits_change,
            [i](const auto &newPropertyValue, const auto&){
               configure[i].force = newPropertyValue;
               return 1;
            },[i](const auto &){return configure[i].force;}
        );
        configureInterface[i]->initialize();
    }
    return;
}

int main(){
    configureFirmwareBus = std::make_shared<sdbusplus::asio::connection>(configureFirmwareIo);
    interface_init();
    configureFirmwareIo.run();
	return 0;
}
