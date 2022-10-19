#include "configure_firmware_commands.hpp"

namespace ipmi::inv::firmware
{
    /** @brief Constructs RedundancyPriority.
     *
     *  @param[in] configure - contains four parts
     *                          1. componentType : specific psu, bios, bmc, or cpld
     *                          2. imageType : reserved byte and only 0x01, 
     *                          3. operation : operation of commands, start update, abort, query, and force update
     *                          4. the remaining is image file (converted to ASCII) and checksum,
     *                             separate image file and checksum with char ':' (0x3a)
     *  @param[out] response - response the status of configure firmware commands
     */
    ipmi::RspType<uint8_t> ipmiOemStartConfigureFirmware(ipmi::Context::ptr ctx, const uint8_t componentTypeIndex, const uint8_t imageType, const uint8_t operation, const std::vector<uint8_t> &configure){
        //now only support bios
        uint8_t componentType = (componentTypeIndex >> 4);
        if(componentType != cmdOemBIOS){
            return ipmi::responseResponseError();
        }
        //start update or force update
        if(operation == START_FW_UPDATE || operation == FORCE_FW_UPDATE){
            //prevent update is in progress and update again
            if(std::get<uint8_t>(getProperty((unsigned long)componentType, "status")) 
                    == FW_UPDATE_IN_PROGRESS){
                return ipmi::responseSuccess(FW_UPDATE_IN_PROGRESS);
            }
            //split path and checksum
            int64_t splitIndex = -1;
            std::string path = "";
            std::vector<uint8_t> checksum;
            for(int64_t i = 0;i < configure.size();i++){
                if(configure[i] == splitIt){
                    splitIndex = i;
                }else if(splitIndex > -1){
                    checksum.push_back(configure[i]);
                }else{
                    path.append(1, (char)configure[i]);
                }
            }
            //check image exist
            if (!fs::is_regular_file(path)){
                setProperty((unsigned long)componentType, "status", FW_IMAGE_NOT_FOUND);
                return ipmi::responseSuccess(FW_IMAGE_NOT_FOUND);
            }
            //check checksum
            if(!validChecksum(path, checksum)){
                setProperty((unsigned long)componentType, "status", FW_IMAGE_CORRUPTED);
                return ipmi::responseSuccess(FW_IMAGE_CORRUPTED);
            }
            //force update should set force flag to dbus for update service to know
            if(operation == FORCE_FW_UPDATE){
                setProperty((unsigned long)componentType, "force", true);
            }else{
                setProperty((unsigned long)componentType, "force", false);
            }
            //if abort command is executed right after startup command, valid checksum need few seconds
            if(std::get<uint8_t>(getProperty((unsigned long)componentType, "status")) == FW_UPDATE_ABORTED 
                    && !std::get<bool>(getProperty((unsigned long)componentType, "force"))){
                //abort success
                setProperty((unsigned long)componentType, "status", FW_UPDATE_NOT_STARTED);
                return ipmi::responseSuccess(FW_UPDATE_ABORTED);
            }
            setProperty((unsigned long)componentType, "status", FW_UPDATE_IN_PROGRESS);
            if(callFWUpdate(component[(unsigned long)componentType], path)){
                return ipmi::responseSuccess(FW_UPDATE_IN_PROGRESS);
            }else{//call update service fail
                setProperty((unsigned long)componentType, "status", FW_UPDATE_NOT_STARTED);
                return ipmi::responseResponseError();
            }
        }else if(operation == ABORT_FW_UPDATE){
            //only in progress can been aborted
            if(std::get<uint8_t>(getProperty((unsigned long)componentType, "status")) != FW_UPDATE_IN_PROGRESS){
                return ipmi::responseSuccess(FW_UPDATE_NOT_STARTED);
            }
            //if not force update
            if(!std::get<bool>(getProperty((unsigned long)componentType, "force"))){
                //set abort flag to dbus
                setProperty((unsigned long)componentType, "status", FW_UPDATE_ABORTED);
                for(int64_t i = 0;i < abortResponseTime;i++){
                    sleep(1);
                    if(std::get<uint8_t>(getProperty((unsigned long)componentType, "status"))
                            == FW_UPDATE_NOT_STARTED){
                        //abort success
                        return ipmi::responseSuccess(FW_UPDATE_ABORTED);
                    }
                }
            }
            return ipmi::responseSuccess(std::get<uint8_t>(getProperty((unsigned long)componentType, "status")));
        }else if(operation == QUERY_FW_UPDATE){
            return ipmi::responseSuccess(std::get<uint8_t>(getProperty((unsigned long)componentType, "status")));
        }
        return ipmi::responseSuccess(resultSuccess);
    }

    bool validChecksum(std::string path, std::vector<uint8_t> checksum){ 
        boost::crc_32_type result;
        try{
            std::ifstream ifs(path, std::ios_base::binary);
            do{
                char buffer[buffer_size];
                ifs.read(buffer, buffer_size);
                result.process_bytes(buffer, ifs.gcount());
            }while(ifs);
        }catch ( std::exception &e ){
            std::cerr << "Found an exception with '" << e.what() << "'." << std::endl;
            return false;
        }catch ( ... ){
            std::cerr << "Found an unknown exception." << std::endl;
            return false;
        }
        uint32_t checksumI32 = checksum[3] | (checksum[2] << 8) | (checksum[1] << 16) | (checksum[0] << 24);
        uint32_t validChecksum = result.checksum();
        if(checksumI32 != validChecksum){
            return false;
        }
        return true;
    }

    void setProperty(unsigned long target, std::string property, ConfFirmValue targetStatus){
        auto configureFirmwareBus = sdbusplus::bus::new_default();
        sdbusplus::message::message writeStatus = configureFirmwareBus.new_method_call(
            CONFIGURE_FIRMWARE_SERVICE, (CONFIGURE_FIRMWARE_PATH + component[target]).c_str(), 
            SYSTEMD_PRO_INTERFACE, METHOD_SET);
        writeStatus.append(CONFIGURE_FIRMWARE_INTERFACE, property, targetStatus);
        try{
            configureFirmwareBus.call(writeStatus);
        }catch (sdbusplus::exception_t& e){
            std::cerr << "Failed to set status " << std::endl;
        }
        return ;
    }

    ConfFirmValue getProperty(unsigned long target, std::string property){
        auto configureFirmwareBus = sdbusplus::bus::new_default();
        sdbusplus::message::message getStatus = configureFirmwareBus.new_method_call(
            CONFIGURE_FIRMWARE_SERVICE, (CONFIGURE_FIRMWARE_PATH + component[target]).c_str(), 
            SYSTEMD_PRO_INTERFACE, METHOD_GET);
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
    bool callFWUpdate(std::string target, std::string path){
        auto callBus = sdbusplus::bus::new_default();
        std::replace(path.begin(), path.end(), '/', '-');
        auto method = callBus.new_method_call(SYSTEMD_BUSNAME, SYSTEMD_PATH,
                                      SYSTEMD_INTERFACE, "StartUnit");
        auto serviceFile = startupService[target] + path + ".service";
        method.append(serviceFile, "replace");
        try{
            auto reply = callBus.call(method);
        }catch (const sdbusplus::exception::exception& e){
            std::cerr << "Failed to call update";
            return false;
        }
        return true;
    }
}
