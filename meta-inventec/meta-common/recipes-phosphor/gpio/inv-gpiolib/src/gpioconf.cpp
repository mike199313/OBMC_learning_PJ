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

#include <invgpiolib.hpp>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <fstream>


namespace fs = std::filesystem;

namespace invgpio {

using Json = nlohmann::json;

// Key words of the GPIO configutration file
static constexpr const char *Key_gpiochips = "GPIOCHIPS";
static constexpr const char *Key_BaseRegAddr = "BaseRegAddr";
static constexpr const char *Key_SOC = "SOC";
static constexpr const char *Key_GPIOdefins = "GPIO_definitions";
static constexpr const char *Key_GPIO = "GPIO";
static constexpr const char *Key_AliasName = "Name";
static constexpr const char *Key_direction = "Direction";
static constexpr const char *Key_activelow = "ActiveLow";


// regular expression rule
#define REGX_AST(num) ("\\s*[aA][sS][tT]\\s*\\-*"#num"\\s*")


// convert string to lower case
static inline void tolowerStr(std::string &str)
{
    std::for_each(str.begin(), str.end(),
        [](char &c) {
            c = ::tolower(c);
        });
}
// convert string to upper case
static inline void toupperStr(std::string &str)
{
    std::for_each(str.begin(), str.end(),
        [](char &c) {
            c = ::toupper(c);
        });
}

static uint32_t gpioPinOffset_ast2600(std::string &pinName)
{
    std::regex reg_gpio("\\s*[A-Z][0-7]\\s*");
    std::regex reg_gpio18("\\s*18[A-E][0-7]\\s*");
    std::smatch m;
    uint32_t offset = 0;

    toupperStr(pinName);

    // first search for GPIO group A-Z
    if (regex_match(pinName, m, reg_gpio))
    {
        std::string str = *m.begin();
        // find the position of non-space character
        auto it = std::find_if(str.begin(), str.end(),
                [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });

        // extract the first alphabet and the following number for calculating the offest
        char alpha = *it, num = *(it + 1);
        offset = static_cast<uint32_t>(alpha - 'A') * 8 + (num - '0');
    }
    else if (regex_match(pinName, m, reg_gpio18))
    {
        std::string str = *m.begin();
        // find the position of non-space character
        auto it = std::find_if(str.begin(), str.end(),
                [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });

        // extract the first alphabet and the following number for calculating the offest
        char alpha = *(it + 2), num = *(it + 3);
        offset = static_cast<uint32_t>(alpha - 'A') * 8 + (num - '0');
    }
    else
    {
        throw std::logic_error(("Cannot find pin offset of ") + pinName);
    }

    return offset;
}

static uint32_t gpioPinOffset_ast2500(std::string &pinName)
{
    std::regex reg_gpio("\\s*([A-Z][0-7])|(A[A-C][0-7])\\s*");
    std::smatch m;
    uint32_t offset = 0;

    toupperStr(pinName);

    if (regex_match(pinName, m, reg_gpio))
    {
        std::string str = *m.begin();
        // find the position of non-space character
        auto it = std::find_if(str.begin(), str.end(),
                [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });

        // extract the first alphabet and the following word for calculating the offest
        char alpha = *it, secWord = *(it + 1);
        std::string pin = std::to_string(alpha);
        pin += std::to_string(secWord);

        // check if the GPIO is group AA ~ AC then adjust the cal argument
        if (secWord >= 'A')
        {
            alpha += secWord - 'A' + 26;
            secWord = *(it + 2);
            pin += std::to_string(secWord);
        }
        offset = static_cast<uint32_t>(alpha - 'A') * 8 + (secWord - '0');
    }
    else
    {
        throw std::logic_error(("Cannot find pin offset of ") + pinName);
    }

    return offset;
}

static std::function<uint32_t(std::string &)> getPinOffsetHandler(std::string &socName)
{
    std::regex soc_ast2600(REGX_AST(2600));
    std::regex soc_ast2500(REGX_AST(2500));
    std::smatch m;

    // if the soc is ast2600
    if (regex_match(socName, m, soc_ast2600))
    {
        return gpioPinOffset_ast2600;
    }
    else if (regex_match(socName, m, soc_ast2500))
    {
        return gpioPinOffset_ast2500;
    }
    return nullptr;
}

static bool getGPIOchipName(std::string &baseAddr/*in*/, std::string &chipName/*out*/)
{
    fs::path sysGPIO("/sys/class/gpio/");

    if (!fs::exists(sysGPIO))
    {
        return false;
    }

    tolowerStr(baseAddr);

    // 1. search /sys/class/gpio/gpiochipxxx/label and read it
    for (const auto& p : fs::directory_iterator(sysGPIO))
    {
        std::string path = p.path().string();

        if (path.find("gpiochip") != std::string::npos)
        {
            std::ifstream labelStream(std::string(path) + "/label");
            if (labelStream.good())
            {
                std::string labelstr;
                labelStream >> labelstr;

                // 2. if the content match then get the gpiochipx dir name
                if (labelstr.find(baseAddr) != std::string::npos)
                {
                    for (const auto& subp : fs::directory_iterator(fs::path(path + "/device/")))
                    {
                        std::string gpioChipPath = subp.path().string();
                        auto posStr = gpioChipPath.rfind("device/gpiochip");
                        if (posStr != std::string::npos)
                        {
                            chipName.clear();
                            chipName = gpioChipPath.substr(posStr + 7);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}


/** @brief To load the GPIO pins configuration
 *  @return a json object of the GPIO pins configuration
 */
Json loadGPIOConfigFile(const char* fileName)
{
    std::ifstream jsonFile(fileName);
    Json newJsonObj = nullptr;

    if (!jsonFile.good())
    {
        std::cerr << fileName << " file not found" << std::endl;
        return newJsonObj;
    }

    try
    {
        newJsonObj = Json::parse(jsonFile, nullptr, false);
    }
    catch (Json::parse_error& e)
    {
        std::cerr << "Failed to parse "<< fileName << ", Error: " << e.what() << std::endl;
    }

    return newJsonObj;
}

/** @brief To get the GPIO pin config from the GPIO config file cache.
 *         This function searches the pin config according to the GPIO pin alias name (gpioObj.name),
 *         a caller must set the "gpioObj.name" before calling this function.
 *
 *  @param[in] gpioConfigJson - the GPIO pin config file cache which is retrieved from "loadGPIOConfigFile()".
 *  @param[in] gpioObj - to store the GPIO config info.
 *
 *  @return bool
 */
bool getGPIOPinObjConfig(invgpio::Json &gpioConfigJson, invgpio::gpioPinObj &gpioObj)
{
    auto gpiochips = gpioConfigJson.find(Key_gpiochips);

    if (gpiochips->is_null() && !gpiochips->is_array())
    {
        std::cerr << "Invalid GPIO configuration file\n";
        return false;
    }

    for (auto itr = gpiochips->begin(); itr != gpiochips->end(); itr++)
    {
        std::string soc;
        std::string baseAddr;

        try
        {
            soc = itr->at(Key_SOC).get<std::string>();
            baseAddr = itr->at(Key_BaseRegAddr).get<std::string>();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            // try next gpiochip
            continue;
        }

        auto gpioDefs = itr->find(Key_GPIOdefins);
        if (gpioDefs->is_null())
        {
            std::cerr << "Cannot find GPIO definitions\n";
            // try next gpiochip
            continue;
        }

        // search this json array for the corresponding alias name
        for (auto itr_pin = gpioDefs->begin(); itr_pin != gpioDefs->end(); itr_pin++)
        {
            auto itr_name = itr_pin->find(Key_AliasName);

            // currently just compare the string naively which means case sensitive
            if (!itr_name->is_null() && !gpioObj.pinName().compare(*itr_name))
            {
                std::string gpioName, direction, chipname;
                uint32_t offset = 0;
                bool activeLow = false;
                std::function<uint32_t(std::string &)> handler = getPinOffsetHandler(soc);

                try
                {
                    gpioName = itr_pin->at(Key_GPIO).get<std::string>();
                    direction = itr_pin->at(Key_direction).get<std::string>();
                    activeLow = itr_pin->at(Key_activelow).get<bool>();
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                    return false;
                }

                try
                {
                    offset = handler(gpioName);
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                    // try next gpiochip
                    continue;
                }

                if (getGPIOchipName(baseAddr, chipname))
                {
                    gpioObj.setPinConfig(chipname, offset,
                        direction.compare("out") ? gpioPinObj::input : gpioPinObj::output,
                        activeLow);

                    return true;
                }
            }
        }
    }

    return false;
}

} // namespace invgpio

