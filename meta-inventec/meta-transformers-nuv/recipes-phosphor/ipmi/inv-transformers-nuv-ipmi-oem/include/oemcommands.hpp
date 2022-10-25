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

#include <ipmid/api-types.hpp>
#include <stdexcept>
#include <cstring>


namespace ipmi
{
/*fan control definition*/
constexpr uint32_t FAN_ID_BASE = 0;
constexpr uint32_t FAN_ID_MIN = 1;
constexpr uint32_t FAN_ID_MAX = 8;
constexpr uint32_t PWM_REG_RANGE = 255;
constexpr uint32_t PWM_MAX = 100;
constexpr uint32_t PWM_RANGE = 100;
constexpr auto parentPwmDir = "/sys/devices/platform/ahb/ahb:apb/f0103000.pwm-fan-controller/hwmon/";

namespace inv
{

static constexpr NetFn netFnOem30 = netFnOemOne;
static constexpr NetFn netFnOem3e = netFnOemEight;


namespace cmdsNetFnOem30
{
    static constexpr Cmd cmdGetGpio = 0xb0;
    static constexpr Cmd cmdSetGpio = 0xb1;
    static constexpr Cmd cmdSetFanPwm = 0xb6;
} // namespace cmdsNetFnOem30

} // namespace inv
} // namespace ipmi
