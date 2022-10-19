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

#pragma once


#include <iostream>
#include <string>
#include <stdexcept>
#include <gpiod.hpp>
#include <nlohmann/json.hpp>

namespace invgpio {

using Json = nlohmann::json;

class gpioPinObj;

// the default path of GPIO config definition file
constexpr const char *defaultGPIOConfPath = "/etc/default/obmc/gpio/gpio_defs_inv.json";
// the default consumer name
const std::string defaultConsumer = "invgpiolib";

/** @brief To load the GPIO pins configuration
 *  @return a json object of the GPIO pins configuration
 */
Json loadGPIOConfigFile(const char* fileName = defaultGPIOConfPath);

/** @brief To get the GPIO pin config from the GPIO config file cache.
 *         This function searches the pin config according to the GPIO pin alias name (gpioObj.name),
 *         a caller must set the "gpioObj.name" before calling this function.
 *
 *  @param[in] gpioConfigJson - the GPIO pin config file cache which is retrieved from "loadGPIOConfigFile()".
 *  @param[in] gpioObj - to store the GPIO config info.
 *
 *  @return bool
 */
bool getGPIOPinObjConfig(invgpio::Json& gpioConfigJson, invgpio::gpioPinObj& gpioObj);

/*
Define a class gpioPinObj to wrap the GPIO line functions
and store a GPIO pin info.
*/
class gpioPinObj
{
public:
    enum direction
    {
        input = 1,
        output
    };

private:
    std::string name;
    std::string chipname;
    std::string consumer;
    uint32_t offset;
    bool activelow;
    gpioPinObj::direction dir;

public:
    //expose the gpio line member that a user can operation it just like using the library "gpiod"
    gpiod::line line;

    gpioPinObj(const std::string& pinName): name(pinName), chipname(""), offset(0),
                                        dir(input), activelow(false)
    {
    };
    gpioPinObj(): name(""), chipname(""), offset(0),
                    dir(input), activelow(false)
    {
    };
    ~gpioPinObj()
    {
        if (line)
        {
            line.reset();
        }
    };

    /** @brief To set the GPIO pin alias name
     *  @param[in] pinName - the GPIO pin alias name
     *  which can be used for parsing the GPIO config file.
     *
     */
    void setPinName(const std::string& pinName);

    /** @brief To get the GPIO pin alias name
     *  @return the GPIO pin alias name
     */
    std::string pinName();

    /** @brief To get the GPIO chip name
     *  @return the GPIO chip name
     */
    std::string chipName();

    /** @brief To get the GPIO pin active low flag
     *  @return bool
     */
    bool activeLow();

    /** @brief To set the GPIO pin config
     *         the GPIO line is retrieved based on the chipName and offset
     *  @param[in] chipName - the GPIO pin belongs to which gpiochip
     *  @param[in] lineOffset - the line offset of this gpiochip
     *  @param[in] direction - GPIO direction
     *  @param[in] activeLow - GPIO is active low or not
     */
    void setPinConfig(const std::string& chipName, uint32_t lineOffset,
        gpioPinObj::direction direction, bool activeLow);


    /** @brief To initialize the GPIO obj
     *          including getting GPIO line and setting GPIO dir and initial value
     *  @param[in] Consumer - the name of the caller
     *  @return bool
     */
    bool init(const std::string& Consumer = defaultConsumer);

    /** @brief To get the gpio line for the member line
     *  @return bool
     */
    bool getGPIOLine();

    /** @brief To set the GPIO pin direction input
     *  @return bool
     */
    bool setGPIOInput();

    /** @brief To set the GPIO pin direction output and its value
     *         but "this function doesn't convert the output value with activeLow/High".
     *  @param[in] value - Output value
     *  @return bool
     */
    bool setGPIOOutput(const int value);

    /** @brief To get the GPIO pin value which is converted with activeLow/High.
     *
     *  @return GPIO current value.
     *          If the gpioline is NULL then throw error.
     */
    int readGPIOValue();

    /** @brief To set the GPIO pin output value converted with activeLow/High.
     *
     *  @param[in] value - Output value
     *  @return bool
     */
    bool setGPIOValue(const int value);

    /** @brief To set the GPIO pin for evnet request.
     *
     *  @param[in] event - which event type to listen to
     *             The event type can be
     *                                  gpiod::line_request::EVENT_FALLING_EDGE
     *                                  gpiod::line_request::EVENT_RISING_EDGE
     *                                  gpiod::line_request::EVENT_BOTH_EDGES
     *
     *  @return success: the file descriptor of the gpio event request.
     *          fail: -1
     */
    int requestGPIOEvents(const int event);

    /** @brief To read the GPIO evnet.
     *
     *  @return success: the gpio event which will be
     *                                               gpiod::line_event::RISING_EDGE
     *                                               gpiod::line_event::FALLING_EDGE
     *          fail: throw error
     */
    gpiod::line_event readEvent();
};

} // namespace invgpio

