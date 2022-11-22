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

#include <oemcommands.hpp>
#include <commandutils.hpp>
#include "platform_configuration_commands.hpp"
#include "configure_firmware_commands.hpp"


#include <ipmid/utils.hpp>
#include <ipmid/api-types.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message/types.hpp>
#include <phosphor-logging/log.hpp>
#include <systemd/sd-bus.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <variant>
#include <vector>
#include <peci.h>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using namespace std;
using namespace ipmi::inv::cmdsNetFnInventec;
using Json = nlohmann::json;

static constexpr const char* chassisIntf =
    "xyz.openbmc_project.State.Chassis";
static constexpr auto SYSTEMDMGR_DEST = "org.freedesktop.systemd1";
static constexpr auto SYSTEMDMGR_PATH = "/org/freedesktop/systemd1";
static constexpr auto SYSTEMDMGR_INTF = "org.freedesktop.systemd1.Manager";
static constexpr auto SYSTEMDMGR_UNIT_INTF = "org.freedesktop.systemd1.Unit";
static constexpr auto NETWORK_DEST = "xyz.openbmc_project.Network";
static constexpr auto ETH_USB_PATH = "/xyz/openbmc_project/network/usb0";
static constexpr auto ETH_INTF = "xyz.openbmc_project.Network.EthernetInterface";
static constexpr auto REDFISH_UNIT = "bmcweb.service";
static constexpr auto IPMID_SVC = "xyz.openbmc_project.Ipmi.Host";
static constexpr auto IPMID_INTF = "xyz.openbmc_project.Ipmi.Server";
static constexpr auto IPMID_PATH = "/xyz/openbmc_project/Ipmi";
static constexpr auto TELEMETRY_PROP = "telemetry";
static constexpr auto ROTATE_CONFIG = "/etc/logrotate.d/logrotate.rsyslog";
static constexpr auto ROTATE_TMP = "/etc/logrotate.d/logrotate.tmp";
static constexpr auto KEY_ROTATE = "rotate ";
static constexpr auto KEY_SIZE = "size ";
static constexpr const char* TELMETRY_CONFIGS[] = {
    "/var/log/kern.log",
    "/var/log/syslog",
    "/var/log/ipmi_cmd"
};

#define MAX_BIOS_CONFIG_NUM 17
#define MAX_NIC_NUM 2
#define NIC_SIZE 7
#define MAX_CPU_NUM 5
#define CPU_SIZE 5
#define MAX_DIMM_NUM 40
#define MEMORY_SIZE 9
#define MAX_PCIE_NUM 30
#define PCIE_SIZE 9
#define MAX_PCIE_SLOT 6
#ifdef BOARD_AST
#ifndef AST_VHUB_ADDR
#define AST_VHUB_ADDR "1e6a0000"
#endif
#define AST_VHUB_ID AST_VHUB_ADDR".usb-vhub"
static constexpr auto AST_VHUB_DRIVER = "/sys/devices/platform/ahb/" AST_VHUB_ID "/driver";
static constexpr auto AST_VHUB_BUS_PATH = "/sys/bus/platform/drivers/aspeed_vhub/";
static constexpr auto BIND = "bind";
static constexpr auto UNBIND = "unbind";
#endif //BOARD_AST

#ifdef BOARD_NUV
#define USB_GADGET_PATH "/sys/kernel/config/usb_gadget/"
static constexpr auto ETH_GADGET_PATH = USB_GADGET_PATH"g1";
static constexpr auto ETH_GADGET_ID = "f0839000.udc";
static constexpr auto USB_NETWORK = "usb_network.service";
#endif //BOARD_NUV

static const std::string DBUS_OBJPATH_SYSTEM_RECONFIGURE =
    "/xyz/openbmc_project/sensors/discrete_6fh/system_event/SystemReconfigure";
const static constexpr char* SYSTEM_CONFIG_FILE =
    "/usr/share/ipmi-providers/system_config_current.json";


static constexpr int POST_CODE_SIZE = 240;
static constexpr int LOG_SIZE_MIN = 4;
static constexpr int LOG_SIZE_MAX = 128;
static constexpr int ROTATE_MIN = 4;
static constexpr int ROTATE_MAX = 180;
static constexpr int CONFIG_INDENT = 4;
#define REQUEST_TO_INDEX(request_id) request_id+1

namespace ipmi
{
static void registerOEMFunctions() __attribute__((constructor));
ipmi::RspType<message::Payload> ipmiOemGenerateRandomPassword(const uint8_t paramSelector, const uint8_t bmcInst);


//ms media redirection
ipmi::RspType<message::Payload> ipmiOemGetRis(uint8_t serviceType, uint8_t paramSelector);

ipmi::RspType<message::Payload> ipmiOemSetRis(uint8_t serviceType,
        uint8_t paramSelector,
        uint8_t blockSelector,
        message::Payload &req);

ipmi::RspType<message::Payload> ipmiOemStartStopRis(uint8_t serviceType, uint8_t startStop);
ipmi::RspType<message::Payload> ipmiOemStartStopMediaRedirect(uint8_t paramSelector, message::Payload &req);
ipmi::RspType<message::Payload> ipmiOemGetMediaImageInfo(uint8_t paramSelector, message::Payload &req);
ipmi::RspType<message::Payload> ipmiOemSetMediaImageInfo(uint8_t paramSelector, message::Payload &req);

/*
An example of IPMI OEM command registration
*/
#if EXAMPLE
ipmi::RspType<bool, //just return the req param
     uint7_t // reserved
     >
     ipmiOemExampleCommand(bool req, uint7_t reserved1)
{
    return ipmi::responseSuccess(req, 0);
}
#endif
ipmi::RspType<std::vector<uint8_t>>
                                 ipmiOemSendRawPeci(uint8_t clientAddr, uint8_t writeLength, uint8_t readLength,
                                         std::vector<uint8_t> writeData)
{
    std::vector<uint8_t> rawResp(readLength);
    if (peci_raw(clientAddr, readLength, writeData.data(), writeData.size(),
                 rawResp.data(), rawResp.size()) != PECI_CC_SUCCESS)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "sendRawPeci command: PECI command failed");
        return ipmi::responseResponseError();
    }

    return ipmi::responseSuccess(rawResp);

}

ipmi::RspType<> ipmiChassisSetPowerInterval(uint8_t interval)
{
    try
    {
        sdbusplus::bus::bus bus(ipmid_get_sd_bus_connection());
        ipmi::DbusObjectInfo chassisPowerObject =
            ipmi::getDbusObject(bus, chassisIntf);
        ipmi::setDbusProperty(bus, chassisPowerObject.second,
                              chassisPowerObject.first, chassisIntf,
                              "RequestedPowerIntervalMs", ((int)interval) * 1000);
    }
    catch (std::exception& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Fail to set RequestedPowerIntervalMs property",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
    return ipmi::responseSuccess();
}

ipmi::RspType<message::Payload>
ipmiOemGetBmcIntfStatus(void)
{
    message::Payload ret;
    std::bitset<2> st_redfish = inv::BMC_INTF_NONE;
    std::bitset<2> st_usb_lan = inv::BMC_INTF_NONE;
    sd_bus_message *reply = NULL;
    char *path;
    char *str_ret;

    try
    {
        int rc, active = 0;
        sd_bus* bus = ipmid_get_sd_bus_connection();

        //Check Redfish status
        rc = sd_bus_call_method(
                 bus,
                 SYSTEMDMGR_DEST,
                 SYSTEMDMGR_PATH,
                 SYSTEMDMGR_INTF,
                 "GetUnit", NULL, &reply, "s", REDFISH_UNIT);
        if (rc >= 0)
        {
            st_redfish |= inv::BMC_INTF_SUPPORTED;
            rc = sd_bus_message_read(reply, "o", &path);
            if (rc >= 0)
            {
                rc = sd_bus_get_property_string(
                         bus,
                         SYSTEMDMGR_DEST,
                         path,
                         SYSTEMDMGR_UNIT_INTF,
                         "ActiveState", NULL, &str_ret);
                if (rc >= 0 && strcmp(str_ret, "active") == 0)
                {
                    st_redfish |= inv::BMC_INTF_ACTIVE;
                }
            }
        }

        //Check USB LAN status
        rc = sd_bus_get_property_trivial(
                 bus,
                 NETWORK_DEST,
                 ETH_USB_PATH,
                 ETH_INTF,
                 "NICEnabled", NULL, 'b', &active);
        if (rc >= 0 && active)
        {
            st_usb_lan |= inv::BMC_INTF_SUPPORTED;
            rc = sd_bus_get_property_trivial(
                     bus,
                     NETWORK_DEST,
                     ETH_USB_PATH,
                     ETH_INTF,
                     "LinkUp", NULL, 'b', &active);
            if (rc >= 0 && active)
            {
                st_usb_lan |= inv::BMC_INTF_ACTIVE;
            }
        }
    }
    catch (std::exception& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Fail to get BMC interface status",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
    ret.pack(st_redfish, uint6_t {});
    ret.pack(st_usb_lan, uint6_t {});
    reply = sd_bus_message_unref(reply);
    return ipmi::responseSuccess(std::move(ret));
}

ipmi::RspType<std::array<uint8_t, 2>,std::vector<uint8_t>>
        ipmiOemGetBiosConfig()
{
    vector<uint8_t> rep;

    try
    {
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "GetBiosConfig");
        auto reply = dbus->call(method);
        reply.read(rep);
        std::array<uint8_t, 2> biosConfig;
        biosConfig[0]=rep[0];
        biosConfig[1]=rep[1];
        //rep.erase(rep.begin(),rep.begin()+2);
        vector<uint8_t> AvailableBiosConfig;
        for (int i=2; i<rep.size(); i++)
        {
            AvailableBiosConfig.push_back(rep[i]);
        }

        return ipmi::responseSuccess(biosConfig,AvailableBiosConfig);
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error reading Bios Config",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
}
ipmi::RspType<>
ipmiOemSetBiosConfig(uint8_t setBiosConfig)
{
    bool setBiosConfig_reply;
    try
    {
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "SetBiosConfig");
        method.append(setBiosConfig);
        auto reply = dbus->call(method);
        reply.read(setBiosConfig_reply);
        if (setBiosConfig_reply == true)
        {
            return ipmi::responseSuccess();
        }
        else
        {
            std::cerr<<"Error setting Bios Config\n";
            return ipmi::responseUnspecifiedError();
        }
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error setting Bios Config",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
}
ipmi::RspType<>
ipmiOemSetBiosConfigInfo(vector<uint8_t>BiosConfigInfo)
{
    if (BiosConfigInfo.size()>MAX_BIOS_CONFIG_NUM)//Invalid number of entries 0x80
    {
        static constexpr ipmi::Cc ipmiCCcompletion = ccInvalidNumOfEntries;
        return ipmi::response(ipmiCCcompletion);
    }
    try
    {
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "SetBiosConfigInfo");
        method.append(BiosConfigInfo);
        auto reply = dbus->call(method);
        bool BiosConfigInfo_reply;
        reply.read(BiosConfigInfo_reply);
        if (BiosConfigInfo_reply == true)
        {
            return ipmi::responseSuccess();
        }
        else
        {
            std::cerr<<"Error setting Bios Config Info\n";
            return ipmi::responseUnspecifiedError();
        }
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error setting Bios Config info",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
}

void do_SystemEventRecordSEL(const std::string& eventObjPath,
                             const std::vector<uint8_t>& eventData,
                             const std::string& logMessage, const bool assert,
                             const uint8_t genID)
{
    const static constexpr char* service = "xyz.openbmc_project.Logging.IPMI";
    const static constexpr char* interface = "xyz.openbmc_project.Logging.IPMI";
    const static constexpr char* path = "/xyz/openbmc_project/Logging/IPMI";
    const static constexpr char* addsel = "IpmiSelAdd";

    try
    {
        std::shared_ptr<sdbusplus::asio::connection> bus = getSdBus();
        // Write SEL method
        sdbusplus::message::message writeSEL =
            bus->new_method_call(service, path, interface, addsel);
        // IPMI log
        writeSEL.append(logMessage, eventObjPath, eventData, assert,
                        static_cast<uint16_t>(genID));
        bus->call(writeSEL);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "exception:%s \n", e.what());
    }
}


void GenerateSELLog(std::string type_name, int device_type, Json old_data, Json json_item)
{
    std::vector<uint8_t> eventData = {0xFF, 0xFF, 0xFF};
    std::vector<int> id_list;
    std::string id;
    int index;

    for(int i=0; i<json_item.size(); i++)
    {
        Json item = json_item[i];
        try
        {
            id = item["path"];
            id = id.substr( id.find(type_name)+type_name.length(), 2 );
            index = stoi(id,nullptr,0);
            if(std::find(id_list.begin(),id_list.end(),index)==id_list.end() )
            {
                if(item["op"]=="replace" || item["op"]=="remove")
                {
                    eventData.at(0) = 0xA0;
                    eventData.at(1) = (device_type | inv::SystemReconfigEventData::Device_removed);
                    eventData.at(2) = index;
                    do_SystemEventRecordSEL(DBUS_OBJPATH_SYSTEM_RECONFIGURE,
                                            eventData,
                                            std::string("System Reconfigure event"),
                                            true, static_cast<uint8_t>(0x20));
                }
                if(item["op"]=="replace" || item["op"]=="add")
                {
                    eventData.at(0) = 0xA0;
                    eventData.at(1) = (device_type | inv::SystemReconfigEventData::Device_addedd);
                    eventData.at(2) = index;
                    do_SystemEventRecordSEL(DBUS_OBJPATH_SYSTEM_RECONFIGURE,
                                            eventData,
                                            std::string("System Reconfigure event"),
                                            true, static_cast<uint8_t>(0x20));
                }
                id_list.push_back(index);
            }
        }
        catch (const std::exception& e)
        {
            std::cerr<< "[" <<__FUNCTION__ << "] Error while generating SEL log " << e.what()<< std::endl;
        }


    }

}

void WriteSystemConfigJson(std::string type_name, Json root, Json json_item)
{
    std::ofstream gfile;
    try
    {
        root[type_name] = json_item;
        gfile.open(SYSTEM_CONFIG_FILE, std::ofstream::trunc);
        gfile << root.dump(4);
        gfile.close();
    }
    catch (const std::exception& e)
    {
        std::cerr<< "[" <<__FUNCTION__ << "] Error while writing json file " << e.what()<< std::endl;
    }
}

void CompareJson(int device_type, Json json_item)
{
    std::ifstream jsonfile(SYSTEM_CONFIG_FILE);

    Json root_data, diff_patch;
    auto type_name = inv::TypeToName[device_type];

    if (jsonfile.is_open())
    {
        Json root = Json::parse(jsonfile, nullptr, false);
        switch(device_type)
        {
        case inv::DeviceType::cpu:
        case inv::DeviceType::memory:
        case inv::DeviceType::pcie:
        case inv::DeviceType::nic:
        {
            try
            {
                root_data = root[type_name];
                diff_patch = Json::diff(root_data,json_item);
                if(!diff_patch.empty())
                {
                    GenerateSELLog(type_name,device_type,root_data,diff_patch);
                    WriteSystemConfigJson(type_name,root,json_item);
                }
            }
            catch (const std::exception& e)
            {
                std::cerr<< "[" <<__FUNCTION__ << "] Error while generating SEL log " << e.what()<< std::endl;
                WriteSystemConfigJson(type_name,root,json_item);
            }
            break;
        }
        default:
        {
            return;
            break;
        }
        }
    }
    else
    {
        fprintf(stderr,"Failed to read json file %s\n", SYSTEM_CONFIG_FILE);
    }
}

struct NICinfo
{
    uint8_t NIC_index;
    std::vector<uint8_t> macAddr;
};
ipmi::RspType<std::vector<uint8_t>>
                                 ipmiOemGetNICInfo(uint8_t index)
{
    NICinfo repData;
    try
    {
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "GetNicInfo");

        method.append(index);
        auto reply = dbus->call(method);
        reply.read(repData.macAddr);
        return ipmi::responseSuccess(repData.macAddr);
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error getting NIC info",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
}
ipmi::RspType<>
ipmiOemSetNICInfo(std::array<uint8_t, 3>& signature, uint16_t reserved,
                  uint8_t number, vector<uint8_t>NIC_Value)
{
    vector<NICinfo>setData;
    static constexpr ipmi::Cc ipmiCCcompletion = ccInvalidNumOfEntries;
    int MAC_ADDR_LENGTH = 50;
    try
    {
        if (NIC_Value.size() != number*(NIC_SIZE) || number > MAX_NIC_NUM)//Invalid number of entries 0x80
        {
            return ipmi::response(ipmiCCcompletion);
        }
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "SetNicInfo");

        NIC_Value.insert(NIC_Value.begin(),number);//append number
        method.append(NIC_Value);
        auto reply = dbus->call(method);
        //NIC_Value.erase(NIC_Value.begin());//erase number
        int nicCount = 1;//skip the [0]num
        for (uint8_t a = 0 ; a < number ; a++)
        {
            NICinfo tmpData;
            tmpData.NIC_index = NIC_Value[nicCount];
            for (int i = 1 ; i < NIC_SIZE ; i++)
            {
                tmpData.macAddr.push_back(NIC_Value[nicCount+i]);
            }
            setData.push_back(tmpData);
            //NIC_Value.erase(NIC_Value.begin(),NIC_Value.begin()+NIC_SIZE);
            nicCount += NIC_SIZE;
        }
        bool NIC_reply;
        reply.read(NIC_reply);
        if (NIC_reply == false)
        {
            static constexpr ipmi::Cc ipmiCCcompletion = ccInvalidIndex;//Invalid NIC index
            return ipmi::response(ipmiCCcompletion);
        }
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error setting NIC info",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }

    nlohmann::json nic_list = Json::object();
    for(auto item : setData)
    {
        char mac_addr[MAC_ADDR_LENGTH];
        std::string name;
        std::ostringstream ss;
        ss << "nic" << std::setw(2) << std::setfill('0') << int(item.NIC_index);
        name = ss.str();
        snprintf( mac_addr, MAC_ADDR_LENGTH, "%02x:%02x:%02x:%02x:%02x:%02x", item.macAddr[0],item.macAddr[1],item.macAddr[2],item.macAddr[3],item.macAddr[4],item.macAddr[5]);
        Json nic = { {"MAC", std::string(mac_addr)} };
        nic_list[name]=nic;
    }

    CompareJson(inv::DeviceType::nic, nic_list);
    return ipmi::responseSuccess();
}
struct Cpuinfo
{
    uint8_t cpuIndex;
    uint8_t cpuType;
    uint16_t cpuFrequency;
    uint8_t cpuStatus;

};
ipmi::RspType<uint8_t,uint16_t,uint8_t>
ipmiOemGetCpuInfo(uint8_t index)
{
    Cpuinfo repData;
    vector<uint8_t> cpuData;
    try
    {
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "GetCpuInfo");
        method.append(index);
        auto reply = dbus->call(method);
        reply.read(cpuData);
        repData.cpuType = cpuData[0];
        repData.cpuFrequency = 0;
        repData.cpuFrequency |= cpuData[2];
        repData.cpuFrequency = repData.cpuFrequency<<8;
        repData.cpuFrequency |= cpuData[1];
        repData.cpuStatus = cpuData[3];
        return ipmi::responseSuccess(repData.cpuType,repData.cpuFrequency,repData.cpuStatus);
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error getting Processor info",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
}
ipmi::RspType<>
ipmiOemSetCpuInfo(std::array<uint8_t, 3>& signature, uint16_t reserved,
                  uint8_t number, vector<uint8_t>Cpu_Value)
{
    vector<Cpuinfo>setData;
    try
    {
        if(Cpu_Value.size()!= number*CPU_SIZE || number > MAX_CPU_NUM)//80h – Invalid number of entries
        {
            static constexpr ipmi::Cc ipmiCCcompletion = ccInvalidNumOfEntries;
            return ipmi::response(ipmiCCcompletion);
        }
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "SetCpuInfo");

        Cpu_Value.insert(Cpu_Value.begin(),number);//append number
        method.append(Cpu_Value);
        auto reply = dbus->call(method);
        //(Cpu_Value.begin());//erase number
        int cpuCount = 1;//skip [0]num
        for (uint8_t a = 0 ; a < number ; a++)
        {
            Cpuinfo tmpData;
            tmpData.cpuIndex = Cpu_Value[cpuCount];
            tmpData.cpuType = Cpu_Value[cpuCount+1];
            tmpData.cpuFrequency = 0;
            tmpData.cpuFrequency |= Cpu_Value[cpuCount+3];
            tmpData.cpuFrequency = tmpData.cpuFrequency<<8;
            tmpData.cpuFrequency |= Cpu_Value[cpuCount+2];
            if (Cpu_Value[cpuCount+4] == inv::CpuStatus::present || Cpu_Value[cpuCount+4] == inv::CpuStatus::not_present)
            {
                tmpData.cpuStatus = Cpu_Value[cpuCount+4];
            }
            else
            {
                ipmi::Cc ipmiCCcompletion = ccInvalidIndex;//Invalid processor index
                return ipmi::response(ipmiCCcompletion);
            }
            setData.push_back(tmpData);
            cpuCount += CPU_SIZE;
        }

        bool Cpu_reply;
        reply.read(Cpu_reply);
        if (Cpu_reply == false)
        {
            static constexpr ipmi::Cc ipmiCCcompletion = ccInvalidIndex;//81h – Invalid processor index
            return ipmi::response(ipmiCCcompletion);
        }
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error setting Processor info",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
    Json cpu_list = Json::object();
    for(auto item : setData)
    {
        std::string name;
        std::ostringstream ss;
        std::string status="N/A";

        ss << "cpu" << std::setw(2) << std::setfill('0') << int(item.cpuIndex);
        name = ss.str();
        std::string type = inv::ToCpuType[item.cpuType];
        if(item.cpuStatus == inv::CpuStatus::present)
        {
            status = "present";
        }
        else if (item.cpuStatus == inv::CpuStatus::not_present)
        {
            status = "not present";
        }
        else
        {
            fprintf(stderr,"Received wrong cpu status\n");
            return ipmi::responseUnspecifiedError();
        }

        Json cpu = { {"type", type }, {"frequency", item.cpuFrequency}, {"status",status} };
        cpu_list[name] = (cpu);
    }

    CompareJson(inv::DeviceType::cpu, cpu_list);
    return ipmi::responseSuccess();
}
struct Meminfo
{
    vector<uint8_t> Mem_value;
    uint8_t Mem_index;
    uint8_t Dimm_info;
    uint16_t Dimm_speed;
    uint32_t Dimm_size;
    uint8_t status;
};
ipmi::RspType<vector<uint8_t>>
                            ipmiOemGetMemInfo(uint8_t index)
{
    Meminfo repData;
    try
    {
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "GetMemInfo");
        if (index > 0)
        {
            method.append(index);
            auto reply = dbus->call(method);
            reply.read(repData.Mem_value);
            return ipmi::responseSuccess(repData.Mem_value);
        }
        else //show presence
        {
            auto method = dbus->new_method_call("xyz.openbmc_project.ObjectMapper",
                                                "/xyz/openbmc_project/object_mapper",
                                                "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths");
            method.append("/", 0, std::array<const char*, 1> {"xyz.openbmc_project.Sensor.IpmiSensor"});
            auto reply = dbus->call(method);
            vector<std::string> resp;
            reply.read(resp);
            uint8_t Dimm_bitmap[5]= {0};
            vector<uint8_t>Dimm_bitmapv;
            for (const auto a : resp)
            {
                if (a.empty())
                {
                    continue; // should be impossible
                }
                try
                {
                    string str1 = "/xyz/openbmc_project/sensors/temperature/DIMM_";
                    if (a.find(str1)!=std::string::npos)
                    {
                        int  pos = a.find(str1);
                        std::string str2 = a.substr(pos+str1.length(),2);//A1
                        Dimm_bitmap[str2[1]-'0']  |= 1<<(str2[0]-'A');//put A in 1
                    }
                }
                catch (const sdbusplus::exception_t& e)
                {
                    phosphor::logging::log<phosphor::logging::level::ERR>(
                        "ipmiOemGetMemInfo: can't get object path",
                        phosphor::logging::entry("ERR=%s", e.what()));
                }
            }
            Dimm_bitmapv.clear();
            for (int i=0; i<MAX_DIMM_NUM/8; i++) //max 40 ,array to vector
            {
                Dimm_bitmapv.push_back(Dimm_bitmap[i]);
            }
            return ipmi::responseSuccess(Dimm_bitmapv);
        }
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error getting Memory info",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
}
ipmi::RspType<>
ipmiOemSetMemInfo(std::array<uint8_t, 3>& signature, uint16_t reserved,
                  uint8_t number, vector<uint8_t>Mem_Value)
{
    if (Mem_Value.size()!= number*MEMORY_SIZE || number > MAX_DIMM_NUM)//Invalid number of entries
    {
        static constexpr ipmi::Cc ipmiCCcompletion = ccInvalidNumOfEntries;
        return ipmi::response(ipmiCCcompletion);
    }
    vector<Meminfo>setData;
    try
    {
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "SetMemInfo");

        Mem_Value.insert(Mem_Value.begin(),number);//append number
        method.append(Mem_Value);
        auto reply = dbus->call(method);
        //Mem_Value.erase(Mem_Value.begin());//erase number
        int memCount = 1;//skip number
        for (uint8_t a = 0 ; a < number ; a++)
        {
            Meminfo tmpData;
            tmpData.Mem_index = Mem_Value[memCount];
            tmpData.Dimm_info = Mem_Value[memCount+1];
            tmpData.Dimm_speed = 0;
            tmpData.Dimm_speed |= Mem_Value[memCount+3];
            tmpData.Dimm_speed = tmpData.Dimm_speed<<8;
            tmpData.Dimm_speed |= Mem_Value[memCount+2];
            tmpData.Dimm_size = 0;
            for (int i=0; i<4; i++)
            {
                tmpData.Dimm_size |= Mem_Value[memCount + 7 - i];
                if (i<3)
                {
                    tmpData.Dimm_size = tmpData.Dimm_size<<8;
                }
            }
            tmpData.status = Mem_Value[memCount+8];
            setData.push_back(tmpData);
            //Mem_Value.erase(Mem_Value.begin(),Mem_Value.begin()+MEMORY_COUNT);
            memCount += MEMORY_SIZE;
        }

        bool Mem_reply;
        reply.read(Mem_reply);
        if (Mem_reply == false)
        {
            static constexpr ipmi::Cc ipmiCCcompletion = ccInvalidIndex;
            return ipmi::response(ipmiCCcompletion);
        }
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error setting Mem info",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
    nlohmann::json dimm_list = Json::object();
    for(auto item : setData)
    {
        std::string name;
        std::ostringstream ss;
        std::string status ="n/a";

        ss << "dimm" << std::setw(2) << std::setfill('0') << int(item.Mem_index);
        name = ss.str();

        uint6_t type= (item.Dimm_info & 0x3f); /* type info oare in bit 5-0*/

        if(item.status==inv::DimmStatus::unknown_dimm){
            status = "unknown dimm type";
        }else if(item.status==inv::DimmStatus::ok){
            status = "ok";
        }else if(item.status==inv::DimmStatus::not_preseent){
            status = "not present";
        }else if(item.status==inv::DimmStatus::single_bit_err){
            status = "single bit error";
        }else if(item.status==inv::DimmStatus::single_bit_err){
            status = "multi bit error";
        }else{
            fprintf(stderr,"Received wrong dimm status\n");
            return ipmi::responseUnspecifiedError();
        }
        
        Json dimm = { {"type", inv::ToDimmType[type] }, {"speed", item.Dimm_speed }, {"size", item.Dimm_size }, {"status", status} };
        dimm_list[name] = dimm;
    }
    CompareJson(inv::DeviceType::memory, dimm_list);
    return ipmi::responseSuccess();
}
struct Pcieinfo
{
    uint8_t Pcie_index;
    vector<uint8_t> Pcie_value;
    uint16_t Pcie_vendorID;
    uint16_t Pcie_deviceID;
    uint16_t Pcie_SubsysVendorID;
    uint16_t Pcie_SubsysID;
};
ipmi::RspType<std::vector<uint8_t>>
                                 ipmiOemGetPcieInfo(uint8_t index)
{
    Pcieinfo repData;
    try
    {
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "GetPcieInfo");
        if (index > 0)
        {
            method.append(index);
            auto reply = dbus->call(method);
            reply.read(repData.Pcie_value);
            return ipmi::responseSuccess(repData.Pcie_value);
        }
        else //show presence
        {
            std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
            uint32_t pcie_bit = 0;
            vector<uint8_t> pcie_bitmap;
            for(int i=1; i<=MAX_PCIE_SLOT; i++) //PCIe max slot=6
            {
                string intStr =std::to_string(i);
                std::string obj_path("/xyz/openbmc_project/inventory/system/chassis/motherboard/pcie_slot");
                obj_path.append(intStr);
                try
                {
                    ipmi::Value present = ipmi::getDbusProperty(*dbus,
                                          "xyz.openbmc_project.Inventory.Manager",
                                          obj_path,
                                          "xyz.openbmc_project.Inventory.Item","Present");
                    bool pcie_presence = std::get<bool>(present);
                    if (pcie_presence == true)
                    {
                        if(i==1)
                        {
                            pcie_bit |= (1<<31);
                        }
                        else if(i==2)
                        {
                            pcie_bit |= (1<<29);
                        }
                        else
                        {
                            int shiftBit = 31-(i-2)*4;
                            pcie_bit |= (1<<shiftBit);
                        }
                    }
                    else
                    {
                        continue;
                    }
                }
                catch (const sdbusplus::exception_t& e)
                {
                    std::cerr<<"Failed to introspect object"<<obj_path<<"\n";
                }
            }
            pcie_bitmap.push_back(pcie_bit>>24);
            pcie_bitmap.push_back(pcie_bit>>16);
            pcie_bitmap.push_back(pcie_bit>>8);
            pcie_bitmap.push_back(pcie_bit);
            return ipmi::responseSuccess(pcie_bitmap);
        }
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error getting PCIe info",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
}
ipmi::RspType<>
ipmiOemSetPcieInfo(std::array<uint8_t, 3>& signature, uint16_t reserved,
                   uint8_t number, vector<uint8_t>Pcie_Value)
{
    if (Pcie_Value.size()!= number*PCIE_SIZE || number > MAX_PCIE_NUM)//Invalid number of entries
    {
        static constexpr ipmi::Cc ipmiCCcompletion = ccInvalidNumOfEntries;
        return ipmi::response(ipmiCCcompletion);
    }
    vector<Pcieinfo>setData;
    try
    {
        std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
        sdbusplus::message::message method = dbus->new_method_call(
                "com.inventec.EepromDevice","/com/inventec/EepromDevice",
                "com.inventec.MsEepromManager", "SetPcieInfo");

        Pcie_Value.insert(Pcie_Value.begin(),number);//append number
        method.append(Pcie_Value);
        auto reply = dbus->call(method);
        //Pcie_Value.erase(Pcie_Value.begin());//erase number
        int  pcieCount = 1;//skip num
        for (uint8_t a = 0 ; a < number ; a++)
        {
            Pcieinfo tmpData;
            tmpData.Pcie_index = Pcie_Value[pcieCount];
            tmpData.Pcie_vendorID = 0;
            tmpData.Pcie_vendorID |= Pcie_Value[pcieCount+2];
            tmpData.Pcie_vendorID = tmpData.Pcie_vendorID<<8;
            tmpData.Pcie_vendorID |= Pcie_Value[pcieCount+1];
            tmpData.Pcie_deviceID = 0;
            tmpData.Pcie_deviceID |= Pcie_Value[pcieCount+4];
            tmpData.Pcie_deviceID = tmpData.Pcie_deviceID<<8;
            tmpData.Pcie_deviceID |= Pcie_Value[pcieCount+3];
            tmpData.Pcie_SubsysVendorID = 0;
            tmpData.Pcie_SubsysVendorID |= Pcie_Value[pcieCount+6];
            tmpData.Pcie_SubsysVendorID = tmpData.Pcie_SubsysVendorID<<8;
            tmpData.Pcie_SubsysVendorID |= Pcie_Value[pcieCount+5];
            tmpData.Pcie_SubsysID = 0;
            tmpData.Pcie_SubsysID |= Pcie_Value[pcieCount+8];
            tmpData.Pcie_SubsysID = tmpData.Pcie_SubsysID<<8;
            tmpData.Pcie_SubsysID |= Pcie_Value[pcieCount+7];
            setData.push_back(tmpData);
            //Pcie_Value.erase(Pcie_Value.begin(),Pcie_Value.begin()+PCIE_COUNT);
            pcieCount += PCIE_SIZE;
        }

        bool Pcie_reply;
        reply.read(Pcie_reply);
        if (Pcie_reply == false)
        {
            static constexpr ipmi::Cc ipmiCCcompletion = ccInvalidIndex;
            return ipmi::response(ipmiCCcompletion);
        }
    }
    catch (const sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error setting Pcie info",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
    nlohmann::json pcie_list = Json::object();
    for(auto item : setData)
    {
        std::string name;
        std::ostringstream ss;
        ss << "pcie" << std::setw(2) << std::setfill('0') << int(item.Pcie_index);
        name = ss.str();
        Json pcie = { {"vendor id", item.Pcie_vendorID }, {"device id", item.Pcie_deviceID }, {"subsystem vendor id",item.Pcie_SubsysVendorID }, {"subststem device id", item.Pcie_SubsysID} };
        pcie_list[name] = pcie;
    }
    CompareJson(inv::DeviceType::pcie, pcie_list);
    return ipmi::responseSuccess();
}
#ifdef SUPPORT_BIOS_OEM_CMD
ipmi::RspType<message::Payload>
ipmiBiosGetBmcIntfStatus(uint8_t param, uint8_t block, uint8_t interfaces)
{
    message::Payload ret;
    std::bitset<3> st_usb_ipmi = inv::BIOS_INTF_NONE;
    std::bitset<3> st_usb_lan = inv::BIOS_INTF_NONE;
    std::bitset<3> st_redfish = inv::BIOS_INTF_NONE;
    sd_bus_message *reply = NULL;
    char *path;
    char *str_ret;

    if (param != 0x01 || block != 0)
    {
        return ipmi::responseInvalidFieldRequest();
    }

    try
    {
        int rc, active = 0;
        sd_bus* bus = ipmid_get_sd_bus_connection();

        if (interfaces & inv::BIOS_LAN_USB_INTF)
        {
            //Check USB LAN status
            rc = sd_bus_get_property_trivial(
                     bus,
                     NETWORK_DEST,
                     ETH_USB_PATH,
                     ETH_INTF,
                     "NICEnabled", NULL, 'b', &active);
            if (rc >= 0 && active)
            {
                st_usb_lan = inv::BIOS_INTF_STARTED;
                rc = sd_bus_get_property_trivial(
                         bus,
                         NETWORK_DEST,
                         ETH_USB_PATH,
                         ETH_INTF,
                         "LinkUp", NULL, 'b', &active);
                if (rc >= 0 && active)
                {
                    st_usb_lan = inv::BIOS_INTF_READY;
                }
            }
        }

        if (interfaces & inv::BIOS_REDFISH_INTF)
        {
            //Check Redfish status
            rc = sd_bus_call_method(
                     bus,
                     SYSTEMDMGR_DEST,
                     SYSTEMDMGR_PATH,
                     SYSTEMDMGR_INTF,
                     "GetUnit", NULL, &reply, "s", REDFISH_UNIT);
            if (rc >= 0)
            {
                rc = sd_bus_message_read(reply, "o", &path);
                if (rc >= 0)
                {
                    st_redfish = inv::BIOS_INTF_ERROR;
                    rc = sd_bus_get_property_string(
                             bus,
                             SYSTEMDMGR_DEST,
                             path,
                             SYSTEMDMGR_UNIT_INTF,
                             "ActiveState", NULL, &str_ret);
                    if (rc >= 0)
                    {
                        if (strcmp(str_ret, "active") == 0)
                        {
                            st_redfish = inv::BIOS_INTF_READY;
                        }
                        else if (strcmp(str_ret, "reloading") == 0 || strcmp(str_ret, "activating") == 0)
                        {
                            st_redfish = inv::BIOS_INTF_STARTED;
                        }
                    }
                }
            }
        }
    }
    catch (std::exception& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Fail to get BMC interface status for BIOS",
            phosphor::logging::entry("ERROR=%s", e.what()));
        return ipmi::responseUnspecifiedError();
    }
    ret.pack(st_usb_ipmi, uint5_t {});
    ret.pack(st_usb_lan, uint5_t {});
    ret.pack(st_redfish, uint5_t {});
    reply = sd_bus_message_unref(reply);
    return ipmi::responseSuccess(std::move(ret));
}

ipmi::RspType<message::Payload>
ipmiGetTelmetryConfig(void)
{
    message::Payload ret;
    uint8_t limit = 0;
    uint8_t rotate = 0;
    std::string readLine;
    std::string value;
    bool found_section = false;
    bool found_rotate = false;
    bool found_size = false;
    std::ifstream inFile(ROTATE_CONFIG);

    while (std::getline(inFile, readLine))
    {
        if (found_section)
        {
            // check rotate config value
            auto pos = readLine.find(KEY_ROTATE);
            if (pos != std::string::npos)
            {
                value = readLine.substr(pos + std::strlen(KEY_ROTATE));
                try {
                    rotate = static_cast<uint8_t>(std::stoi(value));
                    found_rotate = true;
                }
                catch (const std::invalid_argument& ia) {
                    phosphor::logging::log<phosphor::logging::level::ERR>(
                        "Fail to convert rotate value.");
                }
            }

            // check size config value
            pos = readLine.find(KEY_SIZE);
            if (pos != std::string::npos)
            {
                value = readLine.substr(pos + std::strlen(KEY_SIZE));
                pos = value.find_last_of("k");
                if (pos != std::string::npos)
                {
                    value = value.substr(0, pos);
                    try {
                        limit = static_cast<uint8_t>(std::stoi(value));
                        found_size = true;
                    }
                    catch (const std::invalid_argument& ia) {
                        phosphor::logging::log<phosphor::logging::level::ERR>(
                            "Fail to convert size value.");
                    }
                }
            }

            // early stop when both config values are acquired
            if (found_rotate && found_size)
            {
                break;
            }
        }
        else
        {
            // look for target section
            for (auto path : TELMETRY_CONFIGS)
            {
                if (readLine.find(path) != std::string::npos)
                {
                    found_section = true;
                    break;
                }
            }
        }
    }

    // config values incomplete
    if (!found_rotate || !found_size)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "[ipmiGetTelmetryConfig] Fail to find config values");
        return ipmi::responseUnspecifiedError();
    }

    ret.pack(limit);
    ret.pack(rotate);
    return ipmi::responseSuccess(std::move(ret));
}

ipmi::RspType<message::Payload>
ipmiSetTelmetryConfig(uint8_t limit, uint8_t rotate)
{
    // validate limit value
    if (limit < LOG_SIZE_MIN || limit > LOG_SIZE_MAX)
    {
        return ipmi::responseInvalidFieldRequest();
    }

    // validate rotate value
    if (rotate < ROTATE_MIN || rotate > ROTATE_MAX)
    {
        return ipmi::responseInvalidFieldRequest();
    }

    std::string readLine;
    std::ofstream outFile(ROTATE_TMP);
    std::ifstream inFile(ROTATE_CONFIG);
    bool found_section = false;
    bool found_rotate = false;
    bool found_size = false;

    while (std::getline(inFile, readLine))
    {
        if (found_section)
        {
            if (readLine.find(KEY_ROTATE) != std::string::npos)
            {
                // modify rotate config value
                outFile << setw(CONFIG_INDENT) << "" << KEY_ROTATE << static_cast<int>(rotate) << std::endl;
                found_rotate = true;
            }
            else if (readLine.find(KEY_SIZE) != std::string::npos)
            {
                // modify size config value
                outFile << setw(CONFIG_INDENT) << "" << KEY_SIZE << static_cast<int>(limit) << "k" << std::endl;
                found_size = true;
            }
            else
            {
                // leave rest of config values intact
                outFile << readLine << std::endl;
            }

            // finish section modification
            if (found_rotate && found_size)
            {
                found_section = found_rotate = found_size = false;
            }
        }
        else
        {
            // look for target section
            for (auto path : TELMETRY_CONFIGS)
            {
                if (readLine.find(path) != std::string::npos)
                {
                    found_section = true;
                    break;
                }
            }
            outFile << readLine << std::endl;
        }
    }

    // Need to close files first so that we can rename the file
    inFile.close();
    outFile.close();

    // replace old config file with new one
    int ret_rm = std::remove(ROTATE_CONFIG);
    int ret_mv = std::rename(ROTATE_TMP, ROTATE_CONFIG);

    // error during file replacement
    if (ret_rm || ret_mv)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "[ipmiSetTelmetryConfig] Fail to replace original rotate config file");
        return ipmi::responseUnspecifiedError();
    }

    return ipmi::responseSuccess();
}

ipmi::RspType<> ipmiSetTelmetryStatus(uint8_t enable)
{
    setTelemetryStatus(enable ? true : false);
    return ipmi::responseSuccess();
}

ipmi::RspType<uint8_t> ipmiGetTelmetryStatus(void)
{
    uint8_t status = getTelemetryStatus() ? 1 : 0;
    return ipmi::responseSuccess(status);
}

#ifdef BOARD_AST
ipmi::RspType<message::Payload>
ipmiBiosEnableVHub(uint8_t disable)
{
    std::string path = AST_VHUB_BUS_PATH;
    bool enabled = std::filesystem::exists(AST_VHUB_DRIVER);
    bool proceed = false;

    if (disable)
    {
        proceed = enabled;
        path.append(UNBIND);
    }
    else
    {
        proceed = !enabled;
        path.append(BIND);
    }

    if (!std::filesystem::exists(path))
    {
        return ipmi::responseCommandNotAvailable();
    }

    if (proceed)
    {
        ofstream out(path);
        out << AST_VHUB_ID;
        out.close();
    }

    return ipmi::responseSuccess();
}

ipmi::RspType<message::Payload>
ipmiBiosGetVHubStatus(void)
{
    message::Payload ret;
    uint8_t result = 0x01;

    if (std::filesystem::exists(AST_VHUB_DRIVER))
    {
        result = 0x0;
    }

    ret.pack(result);
    return ipmi::responseSuccess(std::move(ret));
}
#endif //BOARD_AST

#ifdef BOARD_NUV
ipmi::RspType<message::Payload>
ipmiBiosEnableGadget(uint8_t disable)
{
    std::string eth_path = ETH_GADGET_PATH;
    bool exist = std::filesystem::exists(eth_path);

    if (exist == !disable)
    {
        // Tommy. No need to proceed since device is already in state specified.
        return ipmi::responseSuccess();
    }

    sdbusplus::bus::bus bus(ipmid_get_sd_bus_connection());
    triggerUnit(bus, USB_NETWORK, !disable);

    return ipmi::responseSuccess();
}

ipmi::RspType<message::Payload>
ipmiBiosGetGadgetStatus(void)
{
    message::Payload ret;
    uint8_t result = 0x01;
    std::string eth_path = ETH_GADGET_PATH;

    if (std::filesystem::exists(eth_path))
    {
        eth_path.append("/UDC");
        std::stringstream stream;
        ifstream in(eth_path);
        stream << in.rdbuf();
        in.close();
        if (stream.str().find(ETH_GADGET_ID) != string::npos)
        {
            result = 0x00;
        }
    }

    ret.pack(result);
    return ipmi::responseSuccess(std::move(ret));
}
#endif //BOARD_NUV
#endif //SUPPORT_BIOS_OEM_CMD

ipmi::RspType< std::vector<uint8_t> >
ipmiBiosGetBIOSCode(uint8_t request)
{
    std::vector<std::tuple<uint64_t, std::vector<uint8_t>>> postcodes;
    std::vector<uint8_t> ret;
    std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();

    switch(request)
    {
    case inv::cmdsNetFnOem32::CURRENT_POST_CODE:
    case inv::cmdsNetFnOem32::PREVIOUS_POST_CODE:
        try
        {
            auto method = dbus->new_method_call(
                              inv::cmdsNetFnOem32::POSTCODE_SERVICE, inv::cmdsNetFnOem32::POSTCODE_OBJ, inv::cmdsNetFnOem32::POSTCODE_INTF, "GetPostCodes" );
            method.append( static_cast<uint16_t>(REQUEST_TO_INDEX(request)) );
            auto reply = dbus->call(method);
            reply.read(postcodes);
            if( postcodes.empty() )
            {
                fprintf(stderr,"[%s] Unable to get post code: %x\n", __FUNCTION__, request);
                return ipmi::responseUnspecifiedError();
            }
            for (const auto& [primary_code, secendary_code] : postcodes)
            {
                ret.push_back(static_cast<uint8_t>(primary_code));
                /*If there are secendary codes, also copy to ret vector*/
                if(secendary_code.size()==0)
                {
                    ret.push_back(static_cast<uint8_t>(0));
                }
                else
                {
                    std::copy( secendary_code.begin(), secendary_code.end(), ret.end() );
                }
            }
            if(ret.size()>POST_CODE_SIZE)
            {
                ret.erase( ret.begin(), ret.end()-POST_CODE_SIZE );
            }
        }
        catch (std::exception &e)
        {
            fprintf(stderr, "[%s] %s\n", __FUNCTION__ , e.what());
            return ipmi::responseUnspecifiedError();
        }
        break;
    default:
        return ipmi::responseInvalidFieldRequest();
    }


    return ipmi::responseSuccess(ret);
}


static void registerOEMFunctions(void)
{
    phosphor::logging::log<phosphor::logging::level::INFO>(
        "Registering INV OEM commands");

    // Chassis command 0x00, 0x0B
    registerOemCmdHandler(ipmi::netFnChassis, ipmi::chassis::cmdSetPowerCycleInterval,
                          Privilege::Admin, ipmiChassisSetPowerInterval);

#ifdef SUPPORT_BIOS_OEM_CMD
    // Inventec OEM command for BIOS
    registerOemCmdHandler(inv::netFnBios, inv::cmdsNetFnBios::cmdGetBmcInfStatus,
                          Privilege::Admin, ipmiBiosGetBmcIntfStatus);
#ifdef BOARD_AST
    registerOemCmdHandler(inv::netFnBios, inv::cmdsNetFnBios::cmdEnableVHub,
                          Privilege::Admin, ipmiBiosEnableVHub);
    registerOemCmdHandler(inv::netFnBios, inv::cmdsNetFnBios::cmdGetVHubStatus,
                          Privilege::Admin, ipmiBiosGetVHubStatus);
#endif //BOARD_AST
#ifdef BOARD_NUV
    registerOemCmdHandler(inv::netFnBios, inv::cmdsNetFnBios::cmdEnableVHub,
                          Privilege::Admin, ipmiBiosEnableGadget);
    registerOemCmdHandler(inv::netFnBios, inv::cmdsNetFnBios::cmdGetVHubStatus,
                          Privilege::Admin, ipmiBiosGetGadgetStatus);
#endif //BOARD_NUV
#endif //SUPPORT_BIOS_OEM_CMD

    // Inventec OEM command
    registerOemCmdHandler(inv::netFnInventec, inv::cmdsNetFnInventec::cmdGetBmcInterfaceStatus,
                          Privilege::Admin, ipmiOemGetBmcIntfStatus);

    /*This is an example of IPMI OEM command registration*/
#if EXAMPLE
    registerOemCmdHandler(inv::netFnOem3e, inv::cmdsNetFnOem3e::cmdExample,
                          Privilege::Admin, ipmiOemExampleCommand);
#endif
    registerOemCmdHandler(inv::netFnOem30, inv::cmdsNetFnOem30::cmdSendRawPeci,
                          Privilege::Admin, ipmiOemSendRawPeci);

    //Inventec OEM command  Generated password 0x3a 0x5d
    registerOemCmdHandler(inv::netFnInventec, inv::cmdsNetFnInventec::cmdOemGenerateRandomPassword,
                          Privilege::Admin, ipmiOemGenerateRandomPassword);

    registerOemCmdHandler(inv::netFnOem38, inv::cmdsEEpromInventec::cmdOemGetBiosConfig,
                          Privilege::Admin, ipmiOemGetBiosConfig);
    registerOemCmdHandler(inv::netFnOem38, inv::cmdsEEpromInventec::cmdOemSetBiosConfig,
                          Privilege::Admin, ipmiOemSetBiosConfig);
    registerOemCmdHandler(inv::netFnOem38, inv::cmdsEEpromInventec::cmdOemSetBiosConfigInfo,
                          Privilege::Admin, ipmiOemSetBiosConfigInfo);

    registerOemCmdHandler(inv::netFnOem30, inv::cmdsEEpromInventec::cmdOemGetNICInfo,
                          Privilege::Admin, ipmiOemGetNICInfo);
    registerOemCmdHandler(inv::netFnOem30, inv::cmdsEEpromInventec::cmdOemSetNICInfo,
                          Privilege::Admin, ipmiOemSetNICInfo);
    registerOemCmdHandler(inv::netFnOem30, inv::cmdsEEpromInventec::cmdOemGetMemInfo,
                          Privilege::Admin, ipmiOemGetMemInfo);
    registerOemCmdHandler(inv::netFnOem30, inv::cmdsEEpromInventec::cmdOemSetMemInfo,
                          Privilege::Admin, ipmiOemSetMemInfo);
    registerOemCmdHandler(inv::netFnOem30, inv::cmdsEEpromInventec::cmdOemGetCpuInfo,
                          Privilege::Admin, ipmiOemGetCpuInfo);
    registerOemCmdHandler(inv::netFnOem30, inv::cmdsEEpromInventec::cmdOemSetCpuInfo,
                          Privilege::Admin, ipmiOemSetCpuInfo);
    registerOemCmdHandler(inv::netFnOem30, inv::cmdsEEpromInventec::cmdOemGetPcieInfo,
                          Privilege::Admin, ipmiOemGetPcieInfo);
    registerOemCmdHandler(inv::netFnOem30, inv::cmdsEEpromInventec::cmdOemSetPcieInfo,
                          Privilege::Admin, ipmiOemSetPcieInfo);


    // Platform configuration commands
    using namespace ipmi::inv::configuration;
    registerOemCmdHandler(netFnPlatformConfiguration, cmdOemGetServiceConfigInfo, Privilege::Admin, ipmiOemGetServiceConfigInfo);
    registerOemCmdHandler(netFnPlatformConfiguration, cmdOemRestoreGoldenConfigs, Privilege::Admin, ipmiOemRestoreGoldenConfigs);
    registerOemCmdHandler(netFnPlatformConfiguration, cmdOemActivateUserConfigs, Privilege::Admin, ipmiOemActivateUserConfigs);
    //configure firmware commands
    using namespace ipmi::inv::firmware;
    registerOemCmdHandler(netFnConfigureFirmware, cmdOemConfigureFirmware, Privilege::Admin, ipmiOemStartConfigureFirmware);

    registerOemCmdHandler(inv::netFnOem32, inv::cmdsNetFnOem32::cmdGetBIOSCode, Privilege::Admin, ipmiBiosGetBIOSCode);

    // Inventec OEM command MS media redirect netfn=0x34
    registerOemCmdHandler(inv::netFnMsMediaRedirect, inv::cmdsNetFnMsMediaRedirect::cmdOemGetRis,
                          Privilege::Admin, ipmiOemGetRis);

    registerOemCmdHandler(inv::netFnMsMediaRedirect, inv::cmdsNetFnMsMediaRedirect::cmdOemSetRis,
                          Privilege::Admin, ipmiOemSetRis);

    registerOemCmdHandler(inv::netFnMsMediaRedirect, inv::cmdsNetFnMsMediaRedirect::cmdOemStartStopRis,
                          Privilege::Admin, ipmiOemStartStopRis);

    registerOemCmdHandler(inv::netFnMsMediaRedirect, inv::cmdsNetFnMsMediaRedirect::cmdOemStartStopMediaRedirect,
                          Privilege::Admin, ipmiOemStartStopMediaRedirect);

    registerOemCmdHandler(inv::netFnMsMediaRedirect, inv::cmdsNetFnMsMediaRedirect::cmdOemGetMediaImageInfo,
                          Privilege::Admin, ipmiOemGetMediaImageInfo);

    registerOemCmdHandler(inv::netFnMsMediaRedirect, inv::cmdsNetFnMsMediaRedirect::cmdOemSetMediaImageInfo,
                          Privilege::Admin, ipmiOemSetMediaImageInfo);

    registerOemCmdHandler(inv::netFnMsOem34, inv::cmdsNetFnMsOem34::cmdGetTelemetryConfig,
                          Privilege::Admin, ipmiGetTelmetryConfig);

    registerOemCmdHandler(inv::netFnMsOem34, inv::cmdsNetFnMsOem34::cmdSetTelemetryConfig,
                          Privilege::Admin, ipmiSetTelmetryConfig);

    registerOemCmdHandler(inv::netFnMsOem34, inv::cmdsNetFnMsOem34::cmdSetTelemetryStatus,
                          Privilege::Admin, ipmiSetTelmetryStatus);

    registerOemCmdHandler(inv::netFnMsOem34, inv::cmdsNetFnMsOem34::cmdGetTelemetryStatus,
                          Privilege::Admin, ipmiGetTelmetryStatus);

}


} // namespace ipmi
