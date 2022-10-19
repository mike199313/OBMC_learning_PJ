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


namespace invgpio {

/** @brief To set the GPIO pin alias name
 *  @param[in] pinName - the GPIO pin alias name
 *  which can be used for parsing the GPIO config file.
 *
 */
void gpioPinObj::setPinName(const std::string& pinName)
{
    this->name = pinName;
}

/** @brief To get the GPIO pin alias name
 *  @return the GPIO pin alias name
 */
std::string gpioPinObj::pinName()
{
    return this->name;
}

/** @brief To get the GPIO chip name
 *  @return the GPIO chip name
 */
std::string gpioPinObj::chipName()
{
    return this->chipname;
}

/** @brief To get the GPIO pin active low flag
 *  @return bool
 */
bool gpioPinObj::activeLow()
{
    return this->activelow;
}

/** @brief To set the GPIO pin config
 *         the GPIO line is retrieved based on the chipName and offset
 *  @param[in] chipName - the GPIO pin belongs to which gpiochip
 *  @param[in] lineOffset - the line offset of this gpiochip
 *  @param[in] direction - GPIO direction
 *  @param[in] activeLow - GPIO is active low or not
 */
void gpioPinObj::setPinConfig(const std::string& chipName, uint32_t lineOffset,
                                gpioPinObj::direction direction, bool activeLow)
{
    this->chipname = chipName;
    this->offset = lineOffset;
    this->dir = direction;
    this->activelow = activeLow;
};

/** @brief To initialize the GPIO obj
 *          including getting GPIO line and setting GPIO dir and initial value
 *  @param[in] Consumer - the name of the caller
 *  @return bool
 */
bool gpioPinObj::init(const std::string& Consumer)
{
    this->consumer = Consumer;
    if (this->dir == output)
    {
        return this->setGPIOValue(0);
    }
    return this->setGPIOInput();
}

/** @brief To get the gpio line for the member line
 *  @return bool
 */
bool gpioPinObj::getGPIOLine()
{
    if (this->line) {
        // if previous gpio resource exist, relese it
        this->line.reset();
    }
    gpiod::chip chip(this->chipname);
    try
    {
        this->line = chip.get_line(this->offset);
    }
    catch (std::exception&)
    {
        std::cerr << this->pinName()<< ", Failed to get GPIO line, offset "
                << this->offset << " of " << this->chipName() << '\n';
        return false;
    }
    chip.reset();
    // Invalid GPIO line
    if (!this->line)
    {
        std::cerr << this->pinName()<< ", Failed to get GPIO line, offset "
                << this->offset << " of " << this->chipName() << '\n';
        return false;
    }
    return true;
}

/** @brief To set the GPIO pin direction input
 *  @return bool
 */
bool gpioPinObj::setGPIOInput()
{
    // if the gpio line is empty then get it
    if (!this->line && this->getGPIOLine() == false)
    {
        return false;
    }

    // release previous request for next one
    this->line.release();

    // Request GPIO to set direction to input
    try
    {
        this->line.request({this->consumer, gpiod::line_request::DIRECTION_INPUT}, 0);
    }
    catch (std::exception&)
    {
        std::cerr << "Failed to set GPIO " << this->pinName()<< " direction to input\n";
        // reset this gpio line for next try
        this->line.reset();
        return false;
    }
    std::cerr << "Set GPIO " << this->pinName()<< " direction to input\n";

    return true;
}

/** @brief To set the GPIO pin direction output and its value
 *         but "this function doesn't convert the output value with activeLow/High".
 *  @param[in] value - Output value
 *  @return bool
 */
bool gpioPinObj::setGPIOOutput(const int value)
{
    // if the gpio line is empty then get it
    if (!this->line && this->getGPIOLine() == false)
    {
        return false;
    }

    // release previous request for next one
    this->line.release();

    // Request GPIO to set direction to out and the value
    try
    {
        this->line.request({this->consumer, gpiod::line_request::DIRECTION_OUTPUT}, value);
    }
    catch (std::exception&)
    {
        std::cerr << "Failed to set GPIO " << this->pinName()<< " direction to output\n";
        // reset this gpio line for next try
        this->line.reset();
        return false;
    }
    std::cerr << "Set GPIO " << this->pinName()
        << " direction to output, value " << std::to_string(value) << '\n';

    return true;
}

/** @brief To get the GPIO pin value which is converted with activeLow/High.
 *
 *  @return GPIO current value.
 *          If the gpioline is NULL then throw error.
 */
int gpioPinObj::readGPIOValue()
{
    if (!this->line)
    {
        throw std::logic_error("GPIO line is uninitialized.");
    }
    return (this->line.get_value() ^ this->activelow);
}

/** @brief To set the GPIO pin output value converted with activeLow/High.
 *
 *  @param[in] value - Output value
 *  @return bool
 */
bool gpioPinObj::setGPIOValue(const int value)
{
    // if the gpio line is empty then request it
    if (!this->line)
    {
        return this->setGPIOOutput(value ^ this->activelow);
    }
    // if the gpio line is available then just call the set_value()
    try
    {
        this->line.set_value(value ^ this->activelow);
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return this->setGPIOOutput(value ^ this->activelow);
}

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
int gpioPinObj::requestGPIOEvents(const int event)
{
    // if the gpio line is empty then get it
    if (!this->line && this->getGPIOLine() == false)
    {
        return -1;
    }

    // release previous request for next one
    this->line.release();

    try
    {
        this->line.request({this->consumer, event});
    }
    catch (std::exception&)
    {
        std::cerr << "Failed to request events for " << this->pinName() << '\n';
        // reset this gpio line for next try
        this->line.reset();
        return -1;
    }

    return this->line.event_get_fd();
}

/** @brief To read the GPIO evnet.
 *
 *  @return success: the gpio event which will be
 *                                               gpiod::line_event::RISING_EDGE
 *                                               gpiod::line_event::FALLING_EDGE
 *          fail: throw error
 */
gpiod::line_event gpioPinObj::readEvent()
{
    if (!this->line)
    {
        throw std::logic_error("GPIO line is uninitialized.");
    }
    return this->line.event_read();
}

} // namespace invgpio

