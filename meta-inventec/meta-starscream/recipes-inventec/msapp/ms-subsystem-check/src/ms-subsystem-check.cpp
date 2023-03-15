#define BOOST_BIND_NO_PLACEHOLDERS

#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
#include <map>

#include <Utils.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/bind/bind.hpp>
#include <boost/ref.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus/match.hpp>
#include <phosphor-logging/log.hpp>
#include <ipmid/api.hpp>
#include <ipmid/types.hpp>
#include <ipmid/utils.hpp>
#include <nlohmann/json.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <i2cbusses.h>
#include <unistd.h>

#ifdef DEBUG
#define dbg(...) do{ {dbg_impl_v1(__FILE__, __LINE__, __func__, __VA_ARGS__);} }while(0)

[[maybe_unused]]inline static void dbg_impl_v1(const char* srcname, int linenum, const char* funcname, const char* fmt, ...)
{
		va_list ap;
		char *filename = basename(strdup(srcname));

		fprintf(stderr, "{%s:%d:%s}:",filename, linenum, funcname);
		va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fflush(stderr);
}
#else
#define dbg(...) do{}while(0)
#endif



#ifdef __cplusplus
}
#endif

using namespace phosphor::logging;
using Json = nlohmann::json;

using Value =
    std::variant<bool, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t,
                 uint64_t, double, std::string, std::vector<std::string>>;

using GetSubTreeType = std::vector<
    std::pair<std::string,
              std::vector<std::pair<std::string, std::vector<std::string>>>>>;

using PropertyMap = std::map<std::string, Value>;

using pathsOfServiceAndObject = std::pair<std::string, std::string>;

static const std::string DBUS_OBJPATH_SUBYSTEM_HEALTH_CHECK =
    "/xyz/openbmc_project/sensors/discrete_6fh/management_subsystem_health/"
    "Subsystem_health";

static const std::string DBUS_OBJPATH_BMC_HEALTH_CHECK =
    "/xyz/openbmc_project/sensors/oem_event_70h/oem_e0h/BMC_health";

static const std::string DBUS_OBJPATH_SYSTEM_RECONFIGURE =
    "/xyz/openbmc_project/sensors/discrete_6fh/system_event/SystemReconfigure";

static std::string DBUS_OBJPATH_PSU_STATE_CHECK =
    "/xyz/openbmc_project/sensors/discrete_6fh/power_supply/PSUStateCheck";


const int I2C_READ_MAX_RETRY=5;
const int I2C_WAIT_SLEEP_MS = 250; //microseconds
static int i2c_smbus_read_byte_data_retry(int fh, int cmd)
{
    int i;
    int retv;
    for(i=0; i<I2C_READ_MAX_RETRY; i++){
        retv = i2c_smbus_read_byte_data(fh, cmd);
        if(retv > 0) return retv;
        usleep(I2C_WAIT_SLEEP_MS);
    }
    return retv;
}

static int i2c_smbus_read_word_data_retry(int fh, int cmd)
{
    int i;
    int retv;
    for(i=0; i<I2C_READ_MAX_RETRY; i++){
        retv = i2c_smbus_read_word_data(fh, cmd);
        if(retv > 0) return retv;
        usleep(I2C_WAIT_SLEEP_MS);
    }
    return retv;

}



static const int MAX_SLEEP_TIME = 5;
static const int FULL_DATA = 0xFF;

static const int RESP_INDEX_POWER_STATUS = 1;
static const int RESP_INDEX_COMPLETE_CODE = 0;
static const int RESP_INDEX_SENSOR_NUMBER = 10;
static const int RESP_INDEX_NEXT_RECORD_ID_LSB = 1;
static const int RESP_INDEX_NEXT_RECORD_ID_MSB = 2;
static const int RESP_INDEX_READING_STATUS = 2;
static const int RESP_INDEX_RECORD_TYPE = 6;
static const int RESP_INDEX_EVENT_STATUS_THRESHOLD_BASED_2 = 3;

static const int CMD_INDEX_RECORD_ID_LSB = 2;
static const int CMD_INDEX_RECORD_ID_MSB = 3;
static const int CMD_INDEX_DATA_LENGTH = 5;
static const int CMD_INDEX_SENSOR_NUMBER = 0;

int CMD_OPTION_TEST_MODE = 0x00; // -t

constexpr std::chrono::microseconds DBUS_TIMEOUT(10000);

static const bool SEL_ASSERTED = true;
static const bool SEL_DEASSERTED = false;
static const uint8_t BMC_HEALTH_MASK_EMPTY_INVALID_FRU =
    static_cast<uint8_t>(0xA4);
static const uint8_t BMC_HEALTH_MASK_NO_MACADDRESS_PROGRAMMED =
    static_cast<uint8_t>(0xAC);


void do_SystemEventRecordSEL(std::shared_ptr<sdbusplus::asio::connection> conn,
                             const std::string& eventObjPath,
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

        // Write SEL method
        sdbusplus::message::message writeSEL =
            conn->new_method_call(service, path, interface, addsel);
        // IPMI log
        writeSEL.append(logMessage, eventObjPath, eventData, assert,
                        static_cast<uint16_t>(genID));
        conn->call(writeSEL);
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "exception:%s \n", e.what());
    }
}

void static check_FRU_devices_status(
    std::shared_ptr<sdbusplus::asio::connection> bus)
{
    std::vector<pathsOfServiceAndObject> serviceAndObjectList;

    GetSubTreeType resp;

    // If no FRU dbus object is found it is empty/invalid error
    std::vector<std::string> interfaces = {"xyz.openbmc_project.FruDevice"};
    auto method =
        bus->new_method_call("xyz.openbmc_project.ObjectMapper",
                             "/xyz/openbmc_project/object_mapper",
                             "xyz.openbmc_project.ObjectMapper", "GetSubTree");

    method.append("/", 0, interfaces);
    try
    {
        auto reply = bus->call(method);
        reply.read(resp);
    }
    catch (const sdbusplus::exception_t& e)
    {
        fprintf(stderr, "%s:%d exception:%s \n", __func__, __LINE__, e.what());
        return;
    }

    for (auto p1 : resp)
    {
        auto objPath = p1.first;
        auto vect1 = p1.second;
        for (auto p2 : vect1)
        {
            auto service = p2.first;
            pathsOfServiceAndObject val = {service, objPath};
            serviceAndObjectList.push_back(val);
        }
    }

    for (auto p : serviceAndObjectList)
    {
        // Check if the bus/address of the FRU is working by trying opening the
        // mapped file in /sys/bus/i2c/devices
        auto busProperty = ipmi::getDbusProperty(
            *bus, p.first, p.second, "xyz.openbmc_project.FruDevice", "BUS");
        auto addressProperty =
            ipmi::getDbusProperty(*bus, p.first, p.second,
                            "xyz.openbmc_project.FruDevice", "ADDRESS");
        std::stringstream output;
        output << "/sys/bus/i2c/devices/" << std::get<uint32_t>(busProperty)
               << "-" << std::right << std::setfill('0') << std::setw(4)
               << std::hex << std::get<uint32_t>(addressProperty) << "/eeprom";
        std::ifstream file(output.str(),
                           std::ios::in | std::ios::binary | std::ios::ate);
        if (file.is_open() == false)
        {
            std::cerr << "Unable to oepn file " << output.str() << std::endl;
            std::vector<uint8_t> eventData = {0xFF, 0xFF, 0xFF};
            eventData.at(0) = BMC_HEALTH_MASK_EMPTY_INVALID_FRU;
            eventData.at(1) = 0xFF;
            eventData.at(2) = 0xFF;

            do_SystemEventRecordSEL(bus, DBUS_OBJPATH_BMC_HEALTH_CHECK,
                                    eventData, std::string("BMC Health Check"),
                                    true, static_cast<uint8_t>(0x20));
            return;
        }
        else
        {
            file.close();
        }

        // checking if the data in the properties is empty

        ipmi::PropertyMap properties = ipmi::getAllDbusProperties(
            *bus, p.first, p.second, "xyz.openbmc_project.FruDevice");
        size_t zeroLengthCount = 0;
        for (auto map : properties)
        {
            try
            {
                // only get the std::string.
                std::string value = std::get<std::string>(map.second);
                if (value.length() == 0)
                    zeroLengthCount++;
            }
            catch (std::exception& e)
            {}
        }
        // ignore ADDRESS / BUS properties
        if (zeroLengthCount >= (properties.size() - 2))
        {
            std::vector<uint8_t> eventData = {0xFF, 0xFF, 0xFF};
            eventData.at(0) = BMC_HEALTH_MASK_EMPTY_INVALID_FRU;
            eventData.at(1) = 0xFF;
            eventData.at(2) = 0xFF;
            do_SystemEventRecordSEL(bus, DBUS_OBJPATH_BMC_HEALTH_CHECK,
                                    eventData, std::string("BMC Health Check"),
                                    true, static_cast<uint8_t>(0x20));
        }
    }
}

struct PSU{
    int bus;
    int address;
    int fh;
};


Json readJsonFile(const std::string &configFile)
{
    std::ifstream jsonFile(configFile);
    if (!jsonFile.good())
    {
        log<level::ERR>("JSON file not found",
                         entry("FILE_NAME=%s", configFile.c_str()));
        return nullptr;
    }

    Json data = nullptr;
    try
    {
        data = Json::parse(jsonFile, nullptr, false);
    }
    catch (const Json::parse_error &e)
    {
        log<level::ERR>("Corrupted config.",
                          entry("MSG=%s", e.what()));
        return nullptr;
    }

    return data;
}

const int I2CTOOL_FORCE_FLAG = 1;

//pmbus command mapping
//reference pmbus spec Table 31 Command Summary
//reference pmbus spec Figure 33 Summary of The status registers
const int CMD_STATUS_BYTE = 0x78;
const int CMD_STATUS_WORD = 0x79;
const int CMD_STATUS_VOUT = 0x7a;
const int CMD_STATUS_IOUT = 0x7b;
const int CMD_STATUS_INPUT = 0x7c;
const int CMD_STATUS_TEMPERATURE = 0x7d;
const int CMD_STATUS_CML = 0x7e;
const int CMD_STATUS_OTHER = 0x7f;
const int CMD_STATUS_MFR_SPECIFIC = 0x80;
const int CMD_STATUS_FANS_1_2 = 0x81;
const int CMD_STATUS_FANS_3_4 = 0x82;

const int BIT_SHIFT_NONE = (1<<0);
const int BIT_SHIFT_CML = (1<<1);
const int BIT_SHIFT_TEMPERATURE = (1<<2);
const int BIT_SHIFT_VIN_UV = (1 << 3);
const int BIT_SHIFT_IOUT_OC = (1<<4);
const int BIT_SHIFT_VOUT_OV = (1<<5);
const int BIT_SHIFT_OFF = (1<<6);
const int BIT_SHIFT_BUSY = (1<<7);
const int BIT_SHIFT_UNKNOWN = (1<<8);
const int BIT_SHIFT_OTHER = (1<<9);
const int BIT_SHIFT_FANS = (1<<10);
const int BIT_SHIFT_POWER_GOOD = (1<<11);
const int BIT_SHIFT_MFR = (1<<12);
const int BIT_SHIFT_INPUT = (1<<13);
const int BIT_SHIFT_IOUT_POUT = (1<<14);
const int BIT_SHIFT_VOUT = (1<<15);

const std::vector<int> BIT_NONE_QUERY_MORE;
const std::vector<int> BIT_CML_QUERY_MORE = {CMD_STATUS_CML};
const std::vector<int> BIT_TEMPERATURE_QUERY_MORE = {CMD_STATUS_TEMPERATURE};
const std::vector<int> BIT_VIN_UV_QUERY_MORE ;
const std::vector<int> BIT_IOUT_OC_QUERY_MORE = {CMD_STATUS_IOUT};
const std::vector<int> BIT_VOUT_OV_QUERY_MORE = {CMD_STATUS_VOUT};
const std::vector<int> BIT_OFF_QUERY_MORE;
const std::vector<int> BIT_BUSY_QUERY_MORE;
const std::vector<int> BIT_UNKNOWN_QUERY_MORE;
const std::vector<int> BIT_OTHER_QUERY_MORE = {CMD_STATUS_OTHER};
const std::vector<int> BIT_FANS_QUERY_MORE = {CMD_STATUS_FANS_1_2, CMD_STATUS_FANS_3_4};
const std::vector<int> BIT_POWER_GOOD_QUERY_MORE;
const std::vector<int> BIT_MFR_QUERY_MORE = {CMD_STATUS_MFR_SPECIFIC};
const std::vector<int> BIT_INPUT_QUERY_MODE = {CMD_STATUS_INPUT};
const std::vector<int> BIT_IOUT_POUT_QUERY_MORE = {CMD_STATUS_IOUT};
const std::vector<int> BIT_VOUT_QUERY_MORE = {CMD_STATUS_VOUT};

std::map<int, std::vector<int>> PMBUS_CMD_MAP = {
    {BIT_SHIFT_NONE, BIT_NONE_QUERY_MORE},
    {BIT_SHIFT_CML, BIT_CML_QUERY_MORE},
    {BIT_SHIFT_TEMPERATURE, BIT_TEMPERATURE_QUERY_MORE},
    {BIT_SHIFT_VIN_UV, BIT_VIN_UV_QUERY_MORE},
    {BIT_SHIFT_IOUT_OC, BIT_IOUT_OC_QUERY_MORE},
    {BIT_SHIFT_VOUT_OV, BIT_VOUT_OV_QUERY_MORE},
    {BIT_SHIFT_OFF, BIT_OFF_QUERY_MORE},
    {BIT_SHIFT_BUSY, BIT_BUSY_QUERY_MORE},
    {BIT_SHIFT_UNKNOWN, BIT_UNKNOWN_QUERY_MORE},
    {BIT_SHIFT_OTHER, BIT_OTHER_QUERY_MORE},
    {BIT_SHIFT_FANS, BIT_FANS_QUERY_MORE},
    {BIT_SHIFT_POWER_GOOD, BIT_POWER_GOOD_QUERY_MORE},
    {BIT_SHIFT_MFR, BIT_MFR_QUERY_MORE},
    {BIT_SHIFT_INPUT, BIT_INPUT_QUERY_MODE},
    {BIT_SHIFT_IOUT_POUT, BIT_IOUT_POUT_QUERY_MORE},
    {BIT_SHIFT_VOUT, BIT_VOUT_QUERY_MORE}};

const uint8_t MS_PW_SEL_FAULT_TYPE_STATUS_VOUT = 0x00;
const uint8_t MS_PW_SEL_FAULT_TYPE_STATUS_IOUT = 0x01;
const uint8_t MS_PW_SEL_FAULT_TYPE_STATUS_INPUT = 0x02;
const uint8_t MS_PW_SEL_FAULT_TYPE_STATUS_TEMP = 0x03;
const uint8_t MS_PW_SEL_FAULT_TYPE_STATUS_CML = 0x04;
const uint8_t MS_PW_SEL_FAULT_TYPE_STATUS_OTHER = 0x05;
const uint8_t MS_PW_SEL_FAULT_TYPE_STATUS_MFR = 0x06;
const uint8_t MS_PW_SEL_FAULT_TYPE_STATUS_FAN_1_2 = 0x07;
const uint8_t MS_PW_SEL_FAULT_TYPE_STATUS_FAN_3_4 = 0x08;

std::map<int, uint8_t> MS_PW_SEL_FAULTS= {
    {CMD_STATUS_VOUT, MS_PW_SEL_FAULT_TYPE_STATUS_VOUT},
    {CMD_STATUS_IOUT, MS_PW_SEL_FAULT_TYPE_STATUS_IOUT},
    {CMD_STATUS_INPUT, MS_PW_SEL_FAULT_TYPE_STATUS_INPUT},
    {CMD_STATUS_TEMPERATURE, MS_PW_SEL_FAULT_TYPE_STATUS_TEMP},
    {CMD_STATUS_CML, MS_PW_SEL_FAULT_TYPE_STATUS_CML},
    {CMD_STATUS_OTHER, MS_PW_SEL_FAULT_TYPE_STATUS_OTHER},
    {CMD_STATUS_MFR_SPECIFIC, MS_PW_SEL_FAULT_TYPE_STATUS_MFR},
    {CMD_STATUS_INPUT, MS_PW_SEL_FAULT_TYPE_STATUS_INPUT},
    {CMD_STATUS_FANS_1_2, MS_PW_SEL_FAULT_TYPE_STATUS_FAN_1_2},
    {CMD_STATUS_FANS_3_4, MS_PW_SEL_FAULT_TYPE_STATUS_FAN_3_4},
};

void static check_PSU_status(
    std::shared_ptr<sdbusplus::asio::connection> bus)
{

    const std::string psuConfigFileName = "/usr/share/ipmi-providers/psu_config.json";
    std::vector<PSU> PSUs;
    int resp_status_word;
    char filename[32];

    //load the PSU i2c information by readin psu_config.json
    Json root = readJsonFile(psuConfigFileName);
    if(root == nullptr){
        log<level::ERR>("load PSU config file failed, skipping checking PSU");
        dbg("load PSU config file failed, skipping checking PSU\n");
        return;
    }
    for(auto& p : root["psu"]){
        PSU psu;
        psu.bus = p["bus"].get<int>();
        psu.address = p["address"].get<int>();
        PSUs.emplace_back(psu);
    }

    unsigned char PSU_index=1; // index for PSU
    //query the PSU status_word (0x79) status
    for(auto& p : PSUs){
        DBUS_OBJPATH_PSU_STATE_CHECK.append(std::to_string(PSU_index)); // insert PSU index into dbus path
        PSU_index++;
        p.fh = open_i2c_dev(p.bus, filename, sizeof(filename), 0);
        if(p.fh <0){
            log<level::ERR>("open I2C bus failed",
                            entry("bus=%d", p.bus));
            dbg("open I2C bus failed bus=%d \n", p.bus);
        }
        int rt = set_slave_addr(p.fh, p.address, I2CTOOL_FORCE_FLAG);
        if(rt){
            log<level::ERR>("set I2C address failed",
                            entry("address=%d", p.address));
            dbg("set I2C address failed address=%d \n", p.address);
            close(p.fh);
            return ;
        }
        
        // query status_word and check the return value.
        resp_status_word = i2c_smbus_read_word_data_retry(p.fh, CMD_STATUS_WORD);
        if(resp_status_word<0){
            log<level::ERR>("read STATUS_WORD failed");
            dbg("read STATUS_WORD failed \n");
            close(p.fh);
            continue;
        }

        if(CMD_OPTION_TEST_MODE){
            resp_status_word = 0xFFFF;
        }
  
        //  if the status_word is not return 0x00 check each bit and related status register for advanced.
        if(resp_status_word){
            dbg("STATUS_WORD=%04X \n", resp_status_word);
            int MAX_SHIFT_COUNT = 16;
            uint8_t STATUS_WORD_LSB = static_cast<uint8_t>(resp_status_word & 0x00FF);
            uint8_t STATUS_WORD_MSB = static_cast<uint8_t>((resp_status_word & 0xFF00) >> 8);
            dbg("STATUS_WORD_LSB=%02X STATUS_WORD_MSB=%02X \n",
                    STATUS_WORD_LSB, STATUS_WORD_MSB);

            std::vector<uint8_t> eventData = {0x01, STATUS_WORD_LSB, STATUS_WORD_MSB};
            do_SystemEventRecordSEL(bus, DBUS_OBJPATH_PSU_STATE_CHECK,
                                    eventData, std::string("PSU STATUS_WORD error"),
                                    true, static_cast<uint8_t>(0x20));
            for(int i=0; i<MAX_SHIFT_COUNT; i++){
                int value = resp_status_word & (1<<i);                
                if(value){
                    for(int& cmd : PMBUS_CMD_MAP[1<<i]){
                        dbg("i=%d, value=%04X cmd=%02X\n", i, value, cmd);
                        uint8_t respv;
                        int retv;
                        // advanced query status_regisger which mapped to status_word bit.
                        retv = i2c_smbus_read_byte_data_retry(p.fh, cmd);
                        if(retv < 0){
                            log<level::ERR>("read STATUS register failed");
                            dbg("read STATUS register failed \n");
                            continue;
                        }

                        respv = static_cast<uint8_t>(retv);
                        if (CMD_OPTION_TEST_MODE){
                            respv = 0xFF;
                        }
                        
                        dbg("advanced query cmd=%02X respv=%02X \n", cmd, respv);
                        if(respv){
                            try{
                                std::vector<uint8_t> eventData = {
                                    0x02, 
                                    MS_PW_SEL_FAULTS.at(cmd),
                                    STATUS_WORD_LSB
                                };
                                do_SystemEventRecordSEL(bus, DBUS_OBJPATH_PSU_STATE_CHECK,
                                                        eventData, std::string("PSU STATUS_WORD error"),
                                                        true, static_cast<uint8_t>(0x20));
                            }
                            catch (std::exception &e){
                                //Not all the bits of status_word 
                                // mapped to the MS power supply fault type
                                //mapbe not found the status falut type, ignore the exception
                            }
                        }
                    }
                }
            }
        }
        DBUS_OBJPATH_PSU_STATE_CHECK.pop_back(); // remove psu index for dbus path
        close(p.fh);
    }

    
    
    
}


void static check_subsystem_sensor_status(
    std::shared_ptr<sdbusplus::asio::connection> bus)
{
    constexpr auto SENSOR_VALUE_INTERFACE = "xyz.openbmc_project.Sensor.Value";
    constexpr auto SERVICE_ROOT = "/";
    auto objTree = ipmi::getAllDbusObjects(*bus, SERVICE_ROOT, SENSOR_VALUE_INTERFACE, {});

    for( const auto& [objPath, objMap] : objTree)
    {
        for(const auto& [objService, objIntfs] : objMap){
            if(objService.find("EventSensor") != std::string::npos){
                break;
            }
            auto property = ipmi::getDbusProperty(*bus, objService, objPath, SENSOR_VALUE_INTERFACE, "Value");
            double value = std::get<double>(property);
            if(std::isnan(value)){
                // Getting value is NaN. do SEL
                constexpr auto IPMI_SENSOR_INTERFACE = "xyz.openbmc_project.Sensor.IpmiSensor";
                auto property = ipmi::getDbusProperty(*bus, objService, objPath, IPMI_SENSOR_INTERFACE, "sensorNumber");
                uint8_t snum = static_cast<uint8_t>(std::get<uint64_t>(property));
                dbg("objService=%s objPath=%s sensorNumber=%ld value is NaN \n", objService.c_str(), objPath.c_str(), snum);
                std::vector<uint8_t> eventData = {0xC0, snum, 0xFF};

                do_SystemEventRecordSEL(bus, DBUS_OBJPATH_SUBYSTEM_HEALTH_CHECK,
                                        eventData, std::string("Check Management Subsystem health event"),
                                        true, static_cast<uint8_t>(0x20));
            }
        }
    }
}


static void checkStatusWrapper1(
    const boost::system::error_code &ec,
    boost::asio::steady_timer &timer,
    std::shared_ptr<sdbusplus::asio::connection> systemBus)
{
    try
    {
        check_PSU_status(systemBus);
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "%s:%d exception%s \n", __FILE__, __LINE__, e.what());
    }

    timer.expires_after(std::chrono::seconds(1));
    timer.async_wait(boost::bind(&checkStatusWrapper1, boost::asio::placeholders::error, boost::ref(timer), systemBus));
}


static void checkStatusWrapper60(
    const boost::system::error_code &ec ,
    boost::asio::steady_timer &timer,
    std::shared_ptr<sdbusplus::asio::connection> systemBus)
{

    try
    {
        check_FRU_devices_status(systemBus);
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "%s:%d exception%s \n", __FILE__, __LINE__, e.what());
    }

    try
    {
        bool powerGood = false;
        constexpr const char *chassisStatePath =
            "/xyz/openbmc_project/state/chassis0";
        constexpr const char *chassisStateIntf =
            "xyz.openbmc_project.State.Chassis";
        auto service =
            ipmi::getService(*systemBus, chassisStateIntf, chassisStatePath);
        ipmi::Value powerState =
            ipmi::getDbusProperty(*systemBus, service, chassisStatePath,
                                  chassisStateIntf, "CurrentPowerState");
        powerGood = std::get<std::string>(powerState) ==
                    "xyz.openbmc_project.State.Chassis.PowerState.On";
        if ( (powerGood) || (CMD_OPTION_TEST_MODE==1) )
        {
            // if host power on, do checking of sub system sensor
            // In testing mode, power off the host, some sensor will return value with NaN
            check_subsystem_sensor_status(systemBus);
        }
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "%s:%d exception%s \n", __FILE__, __LINE__, e.what());
    }

    timer.expires_after(std::chrono::seconds(60));
    timer.async_wait(boost::bind(&checkStatusWrapper60, boost::asio::placeholders::error, boost::ref(timer), systemBus));
}


int main(int argc, char** argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "t")) != -1)
    {
        switch (opt)
        {
            case 't':
            {
                CMD_OPTION_TEST_MODE = 1;
            }break;
        }
    }

    boost::asio::io_service ioservice;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(ioservice);

    boost::asio::steady_timer timer1(ioservice, std::chrono::seconds(1));
    timer1.async_wait(boost::bind(&checkStatusWrapper1, boost::asio::placeholders::error, boost::ref(timer1), systemBus));

    boost::asio::steady_timer timer60(ioservice, std::chrono::seconds(5));
    timer60.async_wait(boost::bind(&checkStatusWrapper60, boost::asio::placeholders::error, boost::ref(timer60), systemBus));

    ioservice.run();
}
