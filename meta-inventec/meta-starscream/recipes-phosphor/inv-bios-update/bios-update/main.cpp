#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bios-update.hpp"
#include <sys/types.h>
#include <fcntl.h>
#include <algorithm>

#include "configure-firmware-update.hpp"

using GetSubTreeType = std::vector<std::pair<
                       std::string,
                       std::vector<std::pair<std::string, std::vector<std::string>>>>>;

auto OBJECT_MAPPER_SERVICE ="xyz.openbmc_project.ObjectMapper";
auto OBJECT_MAPPER_OBJECT ="/xyz/openbmc_project/object_mapper";
auto OBJECT_MAPPER_INTF= "xyz.openbmc_project.ObjectMapper";

auto PROP_INTF = "org.freedesktop.DBus.Properties";

auto ACTIVATION_PROGRESS_INTF = "xyz.openbmc_project.Software.ActivationProgress";

constexpr auto UPDATE_RETRY = 3;
void usage () {
    std::cout << "Usage: bios-update [file name] : update bios firmware\n";
}

int main(int argc, char **argv) {

    std::string object = "";
    std::string service = "";
    GetSubTreeType object_list;
    auto bus = sdbusplus::bus::new_default();
    auto reply =sdbusplus::message::message();
    bool is_remote = false;

    auto method = bus.new_method_call(OBJECT_MAPPER_SERVICE,OBJECT_MAPPER_OBJECT,OBJECT_MAPPER_INTF,"GetSubTree");
    method.append("/");
    method.append(0);

    std::vector<std::string> interface_list;
    interface_list.push_back(ACTIVATION_PROGRESS_INTF);

    method.append(interface_list);

    try {
        reply = bus.call(method);
        reply.read(object_list);
    } catch (const sdbusplus::exception::SdBusError& e) {
        printf("error: sdbus %s\n", e.what());
        return 0;
    }

    bus.release();
    bus.close();

    if(!object_list.empty()) {
        is_remote = true;
        object = object_list[0].first;
        service = object_list[0].second[0].first;
        BIOS_UPDATE_DEBUG("object:%s service:%s \n", object.c_str(), service.c_str());
    }

    //If status is FW_UPDATE_IN_PROGRESS, its means startup from oem cmd.
    //But there may be one situation, this situation happen before this if-statement.
    //Oem cmd start bios-update, but abort bios-update immediately. The status may be changed from UPDATE_IN_PROGRESS to UPDATE_ABORTED.
    //When bios-update start and arrive to this if-statement, status=UPDATE_ABORTED.
    //At this situation, both are from oem, is_oemcmd=true.
    //In UPDATE_ABORTED, we can return earlier to avoid unnecessary process below.
    //As a result, avoid nested if-statement, sperate if-statement of FW_UPDATE_ABORTED and FW_UPDATE_IN_PROGRESS.
    if(std::get<uint8_t>(getProperty("status")) == FW_UPDATE_ABORTED
            && !std::get<bool>(getProperty("force"))){
        is_oemcmd = true;
        setProperty("status", FW_UPDATE_NOT_STARTED);
        std::cerr << "Configure firmware command: FW_UPDATE_ABORTED\n";
        return -1;
    }else if(std::get<uint8_t>(getProperty("status")) == FW_UPDATE_IN_PROGRESS){
        is_oemcmd = true;
    }else{/* not oem cmd*/
        is_oemcmd = false;
    }

    int fd = 0;
    
    if (argc != 2) {
        usage();
        return -1;
    }
    std::string newFilePath(argv[1]);
    int cnt = 0;	
    if (is_remote || is_oemcmd) {
	/*when this utility is invoked as a systmed@.service with argument %i 
        the absolute path need to be converted*/ 
        printf("remote filepath convert! \n");
        for (auto it = newFilePath.begin(); it != newFilePath.end(); it++) {
            if (*it == '-') {
                *it = '/';
                cnt++;
            } 
            if (cnt == 4) {
                break;
            }
        }
    }
    if (access(newFilePath.c_str(), F_OK) < 0)
    {
        std::cerr << "Missing BIOS FW file.\n";
        if(is_oemcmd){
            setProperty("status", FW_IMAGE_NOT_FOUND);
            std::cerr << "Configure firmware command: FW_IMAGE_NOT_FOUND\n";
        }
        return -1;
    }
    uint32_t imageSize = 0;
    struct stat buf;

    if (stat(newFilePath.c_str(), &buf) < 0)
    {
        std::cerr << "Failed to get BIOS FW file info.\n";
        if(is_oemcmd){
            setProperty("status", FW_UPDATE_INITIATE_ERROR);
            std::cerr << "Configure firmware command: FW_UPDATE_INITIATE_ERROR\n";
        }
        return -1;
    }

    if (!S_ISREG(buf.st_mode))
    {
        std::cerr << "Invalid BIOS FW file type.\n";
        if(is_oemcmd){
            setProperty("status", FW_UPDATE_INITIATE_ERROR);
            std::cerr << "Configure firmware command: FW_UPDATE_INITIATE_ERROR\n";
        }
        return -1;
    }

    if (buf.st_size != biosFileSize)
    {
        std::cerr << "Invalid BIOS FW file size.\n";
        if(is_oemcmd){
            setProperty("status", FW_UPDATE_INITIATE_ERROR);
            std::cerr << "Configure firmware command: FW_UPDATE_INITIATE_ERROR\n";
        }
        return -1;
    }

    //wait some time for abort
    if(is_oemcmd && !std::get<bool>(getProperty("force"))){
        for(int64_t i =0;i < abortResponseTime;i++){
            sleep(1);
            if(std::get<uint8_t>(getProperty("status")) == FW_UPDATE_ABORTED){
                setProperty("status", FW_UPDATE_NOT_STARTED);
                std::cerr << "Configure firmware command: FW_UPDATE_ABORTED\n";
                return -1;
            }
        }
    }

    imageSize = buf.st_size;
    int ret = -1;
    BiosUpdateManager bios_updater;

    ret = bios_updater.biosUpdatePrepare();
    if (ret < 0)
    {
        std::cerr << "Failed in bios update prepare.\n";
        if(is_oemcmd){
            setProperty("status", COMPONENT_FAULT_ERROR);
            std::cerr << "Configure firmware command: COMPONENT_FAULT_ERROR\n";
        }
        return -1;
    }
    if (is_remote) {
        if (bios_updater.setMonitorProgress(object, SERVICE_START) < 0) {
            std::cerr << "start monitor progress fail\n";
            bios_updater.biosUpdateFinished(newFilePath.c_str());
            return -1;
        } 
    }
    int retry = 0;
    for (retry = 0; retry < UPDATE_RETRY; retry++) {
        if ((ret = bios_updater.biosUpdate(newFilePath.c_str())) == 0) {
            break;
        }
        std::cerr << "Failed to bios update, retry " << retry << "\n";
        sleep(1);
    }
    if (retry >= UPDATE_RETRY) {
        std::cerr << "bios update retry fail\n";
        ret = bios_updater.biosUpdateFinished(newFilePath.c_str());
        if(is_oemcmd){
            setProperty("status", COMPONENT_FAULT_ERROR);
            std::cerr << "Configure firmware command: COMPONENT_FAULT_ERROR\n";
        }
        return -1;
    }
    ret = bios_updater.biosUpdateFinished(newFilePath.c_str());
    if (ret < 0)
    {
        std::cerr << "Failed in bios update finished.\n";
        if(is_oemcmd){
            setProperty("status", COMPONENT_FAULT_ERROR);
            std::cerr << "Configure firmware command: COMPONENT_FAULT_ERROR\n";
        }
        return -1;
    }
    if(is_remote) {                
        bus = sdbusplus::bus::new_default();
        method = bus.new_method_call(service.c_str(), object.c_str() ,PROP_INTF, "Set");
        method.append(ACTIVATION_PROGRESS_INTF,"Progress");
        uint8_t p = 100;
        method.append(std::variant<uint8_t>(p));
        try {
            reply = bus.call(method);
        } catch (const sdbusplus::exception::SdBusError& e) {
            printf("error: sdbus %s\n", e.what());
        }
        std::cout << "remote set progress done\n";
        sleep(3);
    }
    if(is_oemcmd){
        setProperty("status", FW_UPDATE_COMPLETED_SUCCESSFULLY);
        setProperty("force", false);
        std::cerr << "Configure firmware command: FW_UPDATE_COMPLETED_SUCCESSFULLY\n";
    }
    std::cout << "BIOS FW update finished\n";
    return 0;
}
