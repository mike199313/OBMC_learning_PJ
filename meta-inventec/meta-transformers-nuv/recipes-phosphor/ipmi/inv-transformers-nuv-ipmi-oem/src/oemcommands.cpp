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

#include <ipmid/api.hpp>
#include <ipmid/utils.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message/types.hpp>
#include <phosphor-logging/log.hpp>
#include <nlohmann/json.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <variant>
#include <vector>
#include <peci.h>
#include <fstream>
#include <sstream>

using namespace std;

namespace ipmi
{
static void registerOEMFunctions() __attribute__((constructor));

using Json = nlohmann::json;

static vector<string> getDirFiles(string dirPath, string regexStr)
{
    vector<string> result;

    for (const auto& entry : filesystem::directory_iterator(dirPath))
    {
        // If filename matched the regular expression put it in result.
        if (regex_match(entry.path().filename().string(), regex(regexStr)))
        {
            result.emplace_back(move(entry.path().string()));
        }
    }

    return result;
}

ipmi::RspType<uint8_t> ipmiOemSetFanPwm(uint8_t fanId, uint8_t pwm)
{
    uint8_t rc = 0;
    uint32_t newFanId = 0;
    uint32_t scaledPwm = 0;

    if (fanId < FAN_ID_MIN ||
        fanId > FAN_ID_MAX) {
        phosphor::logging::log<phosphor::logging::level::INFO>(
        "fanId out of range");
        return ipmi::responseUnspecifiedError();
    }
    if (pwm > PWM_MAX) {
        phosphor::logging::log<phosphor::logging::level::INFO>(
        "pwm out of range");
        return ipmi::responseUnspecifiedError();
    }

    newFanId = fanId - FAN_ID_BASE;
    scaledPwm = static_cast<uint32_t>(pwm) * PWM_REG_RANGE / PWM_RANGE;
    auto pwmDirVec = getDirFiles(parentPwmDir, "hwmon[0-9]+");
    if (pwmDirVec.size() != 1)
    {
        phosphor::logging::log<phosphor::logging::level::INFO>(
        "didnt find unique hwmon path");
        return ipmi::responseUnspecifiedError();
    }
    auto pwmFilePath = pwmDirVec[0] + "/pwm" + std::to_string(newFanId);
    std::ofstream ofs;
    ofs.open(pwmFilePath);
    if (!ofs.is_open()) {
        phosphor::logging::log<phosphor::logging::level::INFO>(
        "fail to open the file");
        return ipmi::responseUnspecifiedError();
    }
    ofs << static_cast<int64_t>(scaledPwm);
    ofs.close();
    phosphor::logging::log<phosphor::logging::level::INFO>(
    "set fan pwm ok");
    return ipmi::responseSuccess(rc);
}

const std::string gpioMapConfig = "/usr/share/ipmi-providers/gpiomap.json";

const std::string GPIO_SYS_BASE_PATH = "/sys/class/gpio";

struct GPIOBank {
    std::string chip;
    uint32_t base;
    uint32_t ngpio;
    std::string gpiotype;
};

std::vector<ipmi::GPIOBank> gpioBanks;

static bool setupGpioMap(const std::string &configFile, std::vector<GPIOBank> &banks)
{
    std::ifstream jsonFile(configFile);
    if (!jsonFile.good())
    {
        fprintf(stderr, "gpio map file %s is not found \n", configFile.c_str());
        return false;
    }

    try
    {
        auto root = Json::parse(jsonFile, nullptr, false);
        auto jbanks = root["banks"];
        for (auto &bank : jbanks)
        {
            GPIOBank b = {
                bank["chip"],
                bank["base"],
                bank["ngpio"],
                bank["type"]
            };
            banks.push_back(b);
        }
    }
    catch (Json::parse_error &e)
    {
        fprintf(stderr, "Corrupted gpio map file %s \n", configFile.c_str());
        return false;
    }

    return true;

}

/**
 * @brief The inputed pin num should be mapped into system gpio number. The system gpio map include
 * gpio bank and gpio line information.
 *
 * @param pinNum pin number to translate to valid mapped platform gpio number
 * @return std::tuple<int, GPIOBnak> successfully mapped. return the valid system gpio num and bank to be exported
 */
static std::optional<std::tuple<int, GPIOBank>> mapToSystemGpioLine(int _pinNum)
{
    int pinNum = _pinNum;
    int bankIndex = 0;
    int ngpioTotal = 0;
    for(auto &bank : gpioBanks){
        ngpioTotal += bank.ngpio;
        if ((ngpioTotal - pinNum) > 0 ){
            std::tuple<int, GPIOBank> t;
            t = std::make_tuple(pinNum, bank);
            return std::make_optional(t);
        }
        else{
            pinNum -= ngpioTotal;
        }
    }
    return std::nullopt;
}

static int initOEMGpioAgent(void)
{
    if (gpioBanks.empty())
    {
        setupGpioMap(gpioMapConfig, gpioBanks);
        if (gpioBanks.empty())
        {
            fprintf(stderr, "setup gpio map failed\n");
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Get the Gpio Line object
 * 
 * @param gpio 
 * @param bank 
 * @return std::optional<std::tuple<uint8_t, uint8_t>> value and direction
 * value : 1 for high, 0 for low
 * direction : 1 for output , 0 for input
 */
static std::optional<std::tuple<uint8_t, uint8_t>> getGpioLine(int& gpio, GPIOBank& bank)
{
    bool ifFail = false;
    int exportnum = gpio + bank.base;
    std::stringstream ss;

    ss << GPIO_SYS_BASE_PATH << "/export";
    std::ofstream opengpiofs(ss.str());
    opengpiofs << exportnum; // create gpioline XXX of gpiobank as /sys/class/gpio/gpioXXX
    opengpiofs.close();

    //Check if the gpioXXX line is created
    //It maybe already existing by other process
    struct stat s;
    ss.str("");
    ss << GPIO_SYS_BASE_PATH << "/gpio" << exportnum;
    if( stat(ss.str().c_str(), &s) != 0){
        fprintf(stderr, "%s is NOT existing \n", ss.str().c_str());
        return std::nullopt;
    }

    ss.str("");
    ss << GPIO_SYS_BASE_PATH << "/gpio" << exportnum << "/value";
    std::ifstream valuefs(ss.str());
    stringstream value;
    value << valuefs.rdbuf();
    
    ss.str("");
    ss << GPIO_SYS_BASE_PATH << "/gpio" << exportnum << "/direction";
    std::ifstream directionfs(ss.str());
    stringstream direction;
    direction << directionfs.rdbuf();
    
    ss.str("");
    ss << GPIO_SYS_BASE_PATH << "/unexport";
    std::ofstream closegpiofs(ss.str());
    closegpiofs << exportnum; // create gpioline XXX of gpiobank as /sys/class/gpio/gpioXXX

    if (valuefs.fail() || directionfs.fail()){
        ifFail = true;
    }

    valuefs.close();
    directionfs.close();
    closegpiofs.close();

    if( ifFail == false){
        return make_tuple( (value.str().compare(0, 1, "0") == 0 ? 0 : 1), (direction.str().compare(0, 2, "in") == 0 ? 0 : 1) );
    }else{
        fprintf(stderr, "%s get value or direction failed \n", __func__);
        return std::nullopt;
    }
}

/**
 * @brief Set the Gpio Line object. Only direction be output can write value.
 *
 * @param gpio
 * @param bank
 * @param direction 0 is input, 1 is output
 * @param value 0 is low, 1 is high
 * @return bool true as successful otherwise error. 
 */
static bool setGpioLine(int &gpio, GPIOBank &bank, uint8_t &direction, uint8_t &value)
{
    int ifFail = 0;
    int exportnum = gpio + bank.base;
    std::stringstream ss;

    ss << GPIO_SYS_BASE_PATH << "/export";
    std::ofstream opengpiofs(ss.str());
    opengpiofs << exportnum; // create gpioline XXX of gpiobank as /sys/class/gpio/gpioXXX
    opengpiofs.close();

    // Check if the gpioXXX line is created
    // It maybe already existing by other process
    struct stat s;
    ss.str("");
    ss << GPIO_SYS_BASE_PATH << "/gpio" << exportnum;
    if (stat(ss.str().c_str(), &s) != 0)
    {
        fprintf(stderr, "%s is NOT existing \n", ss.str().c_str());
        return false;
    }


    ss.str("");
    ss << GPIO_SYS_BASE_PATH << "/gpio" << exportnum << "/direction";
    std::ofstream directionfs(ss.str());
    stringstream dss;
    if (direction == 0){
        dss << "in";
    }
    else{
        dss << "out";
    }
    directionfs << dss.str();
    if(directionfs.fail()){
        ifFail++;
    }
    directionfs.close();

    //only gpio direction be output will write value into
    if(direction == 1)
    {
        ss.str("");
        ss << GPIO_SYS_BASE_PATH << "/gpio" << exportnum << "/value";
        std::ofstream valuefs(ss.str());
        stringstream vss;
        if(value == 0){
            vss << "0";
        }else{
            vss << "1";
        }
        valuefs << vss.str();
        if(valuefs.fail()){
            ifFail++;
        }
        valuefs.close();
    }
    

    ss.str("");
    ss << GPIO_SYS_BASE_PATH << "/unexport";
    std::ofstream closegpiofs(ss.str());
    closegpiofs << exportnum; // create gpioline XXX of gpiobank as /sys/class/gpio/gpioXXX
    closegpiofs.close();

    return (ifFail > 0) ;
}

ipmi::RspType<uint8_t, uint8_t> ipmiOemGetGpio(uint8_t pinNum)
{
    GPIOBank bank;
    int gpio;

    int ret = initOEMGpioAgent();
    if(ret){
        return ipmi::responseCmdFailInitAgent();
    }


    try{
        uint8_t value = 0xFF;
        uint8_t direction = 0xFF;

        auto o = mapToSystemGpioLine(pinNum);
        if(o == nullopt){
            fprintf(stderr, "mapToSystemGpioLine with %d failed \n", pinNum);
            return ipmi::responseParmOutOfRange();
        }
        std::tie(gpio, bank) = *o;
        
        auto o2 = getGpioLine(gpio, bank);
        if(o2 == nullopt){
            fprintf(stderr, "getGpioLine failed %d maybe in used\n", pinNum);
            return ipmi::responseUnspecifiedError();
        }
        std::tie(value, direction) = *o2;
        return ipmi::responseSuccess(direction, value);
    }catch(const std::exception &e){
        fprintf(stderr, "Exception %s \n", e.what());
        return ipmi::responseUnspecifiedError();
    }
}

ipmi::RspType<> ipmiOemSetGpio(uint8_t pinNum, uint8_t pinDirection, uint8_t pinValue)
{
    GPIOBank bank;
    int gpio;

    int ret = initOEMGpioAgent();
    if(ret){
        return ipmi::responseCmdFailInitAgent();
    }

    try
    {
        auto o = mapToSystemGpioLine(pinNum);
        if (o == nullopt)
        {
            fprintf(stderr, "mapToSystemGpioLine with %d failed \n", pinNum);
            return ipmi::responseParmOutOfRange();
        }
        std::tie(gpio, bank) = *o;

        auto ifFail = setGpioLine(gpio, bank, pinDirection, pinValue);
        if (ifFail == true)
        {
            fprintf(stderr, "setGpioLine failed %d maybe in used\n", pinNum);
            return ipmi::responseUnspecifiedError();
        }
        
        return ipmi::responseSuccess();
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "Exception %s \n", e.what());
        return ipmi::responseUnspecifiedError();
    }
}

static void registerOEMFunctions(void)
{
    phosphor::logging::log<phosphor::logging::level::INFO>(
        "Registering INV TRANSFORMERS-NUV OEM commands");
    
    registerOemCmdHandler(inv::netFnOem30, inv::cmdsNetFnOem30::cmdGetGpio,
                          Privilege::Admin, ipmiOemGetGpio);

    registerOemCmdHandler(inv::netFnOem30, inv::cmdsNetFnOem30::cmdSetGpio,
                          Privilege::Admin, ipmiOemSetGpio);

    registerOemCmdHandler(inv::netFnOem30, inv::cmdsNetFnOem30::cmdSetFanPwm,
                            Privilege::Admin, ipmiOemSetFanPwm);
}

} // namespace ipmi
