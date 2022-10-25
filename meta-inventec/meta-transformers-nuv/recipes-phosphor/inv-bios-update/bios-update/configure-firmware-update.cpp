#include "configure-firmware-update.hpp"

void setProperty(std::string property, ConfFirmValue targetStatus){
    auto configureFirmwareBus = sdbusplus::bus::new_default();
    sdbusplus::message::message writeStatus = configureFirmwareBus.new_method_call(
        CONFIGURE_FIRMWARE_SERVICE, CONFIGURE_FIRMWARE_PATH, SYSTEMD_PRO_INTERFACE, METHOD_SET);
    writeStatus.append(CONFIGURE_FIRMWARE_INTERFACE, property, targetStatus);
    try{
        configureFirmwareBus.call(writeStatus);
    }catch (sdbusplus::exception_t& e){
        std::cerr << "Failed to set status " << std::endl;
    }
    return ;
}

ConfFirmValue getProperty(std::string property){
    auto configureFirmwareBus = sdbusplus::bus::new_default();
    sdbusplus::message::message getStatus = configureFirmwareBus.new_method_call(
        CONFIGURE_FIRMWARE_SERVICE, CONFIGURE_FIRMWARE_PATH, SYSTEMD_PRO_INTERFACE, METHOD_GET);
    getStatus.append(CONFIGURE_FIRMWARE_INTERFACE, property);
    try{
        ConfFirmValue value;
        auto reply = configureFirmwareBus.call(getStatus);
        reply.read(value);
        return value;
    }catch (sdbusplus::exception_t& e){
        std::cerr << "Failed to get status" << std::endl;
    }
    return false;
}
