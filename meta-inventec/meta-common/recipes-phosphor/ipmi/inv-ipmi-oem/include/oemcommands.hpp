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
#include <ipmid/api.hpp>
#include <stdexcept>
#include <map>

#define EXAMPLE 0

namespace ipmi
{
namespace inv
{

static constexpr NetFn netFnOem30 = netFnOemOne;
static constexpr NetFn netFnOem32 = netFnOemTwo;
static constexpr NetFn netFnOem3e = netFnOemEight;

static constexpr NetFn netFnInventec = netFnOemSix;

//MS MediaRedirection netfn
static constexpr NetFn netFnMsMediaRedirect = 0x34;

static constexpr NetFn netFnOem38 = netFnOemFive;
#ifdef SUPPORT_BIOS_OEM_CMD
static constexpr NetFn netFnBios = netFnOemTwo;
#endif //SUPPORT_BIOS_OEM_CMD


/*based on system reconfiguration SEL log definition in MS spec*/
static constexpr int COMPONENT_TYPE_OFFSET = 4;
enum DeviceType{
    cpu    = 0x0 << COMPONENT_TYPE_OFFSET,
    memory = 0x1 << COMPONENT_TYPE_OFFSET,
    pcie   = 0x2 << COMPONENT_TYPE_OFFSET,
    nic    = 0x3 << COMPONENT_TYPE_OFFSET,
};
enum SystemReconfigEventData{
    Device_removed = 0x0,
    Device_addedd  = 0x1,
};
enum CpuStatus{
    present     = 0x1,
    not_present = 0xff,
};
enum DimmStatus{
    unknown_dimm     = 0x1,
    ok               = 0x2,
    not_preseent     = 0x3,
    single_bit_err   = 0x5,
    multi_bir_err    = 0x7,
};
static std::map<int, std::string> TypeToName = {
    {DeviceType::cpu,    "cpu"},
    {DeviceType::memory, "dimm"},
    {DeviceType::pcie,   "pcie"},
    {DeviceType::nic,    "nic"},
};
/*Based on table in ms spec*/
enum CpuType{
    CPUTYPE_SANDYBRIDGE     = 0x15,
    CPUTYPE_iVYBRIDGE       = 0x16,
    CPUTYPE_CENTERTON       = 0x17,
    CPUTYPE_HASWELL         = 0x18,
    CPUTYPE_BROADWELL       = 0x19,
    CPUTYPE_SKYLAKE         = 0x20,
    CPUTYPE_COFFEELAKE      = 0x21,
    CPUTYPE_CASCAKDELAKE    = 0x22,
    CPUTYPE_ICELAKE         = 0x23,
    CPUTYPE_COOPERLAKE      = 0x24,
    CPUTYPE_SP3             = 0x70,
    CPUTYPE_THUNDERX2       = 0x71,
    CPUTYPE_ROME            = 0x72,
    CPUTYPE_MILAN_ROME      = 0x73,
    CPUTYPE_NONE            = 0xFF,
};
static std::map<uint8_t, std::string> ToCpuType = {
    {CPUTYPE_SANDYBRIDGE,   "Sandy Bridge"},
    {CPUTYPE_iVYBRIDGE,     "Ivy Bridge"},
    {CPUTYPE_CENTERTON,     "Centerton"},
    {CPUTYPE_HASWELL,       "Haswell"},
    {CPUTYPE_BROADWELL,     "Broadwell"},
    {CPUTYPE_SKYLAKE,       "Skylake"},
    {CPUTYPE_COFFEELAKE,    "CoffeLake"},
    {CPUTYPE_CASCAKDELAKE,  "CascakdeLake"},
    {CPUTYPE_ICELAKE,       "IceLake"},
    {CPUTYPE_COOPERLAKE,    "CooperLake"},
    {CPUTYPE_SP3,           "SP3"},
    {CPUTYPE_THUNDERX2,     "ThunderX2"},
    {CPUTYPE_ROME,          "Rome"},
    {CPUTYPE_MILAN_ROME,    "Milan/Rome"},
    {CPUTYPE_NONE,          "No CPU present"},
};
enum DimmType{
    DIMMTYPE_SDRAM                  = 0x0,
    DIMMTYPE_DDR1_RAM               = 0x1,
    DIMMTYPE_RAMBUS                 = 0x2,
    DIMMTYPE_DDR2_RAM               = 0x3,
    DIMMTYPE_FBDIMM                 = 0x4,
    DIMMTYPE_DDR3_RAM               = 0x5,
    DIMMTYPE_DDR4_RAM               = 0x6,
    DIMMTYPE_DDR3_NVDIMM            = 0x7,
    DIMMTYPE_DDR4_NVDIMM            = 0x8,
    DIMMTYPE_DDR3_NVDIMM_W_SUPERCAP = 0x9,
    DIMMTYPE_DDR4_NVDIMM_W_SUPERCAP = 0xA,
    DIMMTYPE_NONE                   = 0x3F,
};
static std::map<uint6_t, std::string> ToDimmType = {
    {DIMMTYPE_SDRAM,                    "SDRAM"},
    {DIMMTYPE_DDR1_RAM,                 "DDR-1 RAM"},
    {DIMMTYPE_RAMBUS,                   "Rambus"},
    {DIMMTYPE_DDR2_RAM,                 "DDR-2 RAM"},
    {DIMMTYPE_FBDIMM,                   "FBDIMM"},
    {DIMMTYPE_DDR3_RAM,                 "DDR-3 RAM"},
    {DIMMTYPE_DDR4_RAM,                 "DDR-4 RAM"},
    {DIMMTYPE_DDR3_NVDIMM,              "DDR-3 NVDIMM"},
    {DIMMTYPE_DDR4_NVDIMM,              "DDR-4 NVDIMM"},
    {DIMMTYPE_DDR3_NVDIMM_W_SUPERCAP,   "DDR-3 NVDIMM with Supercap"},
    {DIMMTYPE_DDR4_NVDIMM_W_SUPERCAP,   "DDR-4 NVDIMM with Supercap"},
    {DIMMTYPE_NONE,                     "No DIMM present"},
};


namespace cmdsNetFnOem30
{
    static constexpr Cmd cmdSendRawPeci = 0xE6;

} // namespace cmdsNetFnOem30

namespace cmdsNetFnOem3e
{
//An example of IPMI OEM command registration
#if EXAMPLE
static constexpr Cmd cmdExample = 0xff;
#endif

} // namespace cmdsNetFnOem3e

namespace cmdsNetFnOem32
{
    static constexpr Cmd cmdGetBIOSCode = 0x73;
    static constexpr int CURRENT_POST_CODE = 0;
    static constexpr int PREVIOUS_POST_CODE = 1;
    static constexpr char* POSTCODE_SERVICE = "xyz.openbmc_project.State.Boot.PostCode0";
    static constexpr char* POSTCODE_OBJ = "/xyz/openbmc_project/State/Boot/PostCode0";
    static constexpr char* POSTCODE_INTF = "xyz.openbmc_project.State.Boot.PostCode";
    #define REQUEST_TO_INDEX(request_id) request_id+1

} // namespace cmdsNetFnOem32

namespace cmdsNetFnMsMediaRedirect{

    //ms media redirection

    constexpr Cc ccParameterNotSupported = 0x80;
    constexpr Cc ccRedirectionRunning = 0x83;

    constexpr Cc ccServiceNotEnabled = 0x90;
    constexpr Cc ccInvalidMediaType = 0x91;
    constexpr Cc ccInvalidRISStartStopCommand = 0x94;
    constexpr Cc ccInvalidMediaRedirectStartStopCommand = 0x9A;
    constexpr Cc ccInsufficientConfiguration = 0x97;

    constexpr Cmd cmdOemGetRis = 0x9E;
    constexpr Cmd cmdOemSetRis = 0x9F;
    constexpr Cmd cmdOemStartStopRis = 0xA0;
    constexpr Cmd cmdOemStartStopMediaRedirect = 0xD7;
    constexpr Cmd cmdOemGetMediaImageInfo = 0xD8;
    constexpr Cmd cmdOemSetMediaImageInfo = 0xD9;

    static inline auto responseParameterNotSupported()
    {
        return response(ccParameterNotSupported);
    }

}

namespace cmdsNetFnInventec
{
    constexpr Cmd cmdGetBmcInterfaceStatus = 0x50;

    static constexpr uint8_t _BIOS_USER = 0x1;
    static constexpr uint8_t _OS_USER = 0x2;
    static constexpr uint8_t _DELETE_FW_USER = 0x4;
    static constexpr uint8_t _DELETE_OS_USER = 0x8;
    static constexpr uint8_t MAX_PASSWORD_LENGTH = 20;

    static constexpr uint8_t _HOST_INTERFACE_ENABLED = 0x01;
    static constexpr uint8_t _KERNEL_AUTH_ENABLED = 0x02;
    static constexpr uint8_t _FIRMWARE_AUTH_ENABLED = 0x04;

    static constexpr auto NETWORK_SERVICE = "xyz.openbmc_project.Network";
    static constexpr auto NETWORK_USB0_OBJECT = "/xyz/openbmc_project/network/usb0";
    static constexpr auto NETWORK_ETH_INTERFACE = "xyz.openbmc_project.Network.EthernetInterface";

    static constexpr auto WATCHDOG_SERVICE = "xyz.openbmc_project.Watchdog";
    static constexpr auto WATCHDOG_HOST0_OBJECT = "/xyz/openbmc_project/watchdog/host0";
    static constexpr auto WATCHDOG_STATE_INTERFACE = "xyz.openbmc_project.State.Watchdog";

    static constexpr auto IPMI_SESSION_SERVICE = "xyz.openbmc_project.Ipmi.Channel.usb0";
    static constexpr auto IPMI_SESSION_SESSIONINFO_INTERFACE = "xyz.openbmc_project.Ipmi.SessionInfo";

    static const std::string OSUsername = "HostAutoOS";
    static const std::string FWUsername = "HostAutoFW";

    constexpr Cmd cmdOemGenerateRandomPassword = 0x5D;

    static inline auto responseHostInterfaceNotReady()
    {
        return response(0x85);
    }

} // namespace cmdsNetFnInventec

namespace cmdsEEpromInventec
{
	static constexpr Cmd cmdOemGetNICInfo = 0x19;
    static constexpr Cmd cmdOemSetNICInfo = 0xF3;
    static constexpr Cmd cmdOemGetMemInfo = 0x1D;
    static constexpr Cmd cmdOemSetMemInfo = 0xF1;
    static constexpr Cmd cmdOemGetCpuInfo = 0x1B;
    static constexpr Cmd cmdOemSetCpuInfo = 0xF0;
    static constexpr Cmd cmdOemGetPcieInfo = 0x1A;
    static constexpr Cmd cmdOemSetPcieInfo = 0xF2;
    
	static constexpr Cmd cmdOemGetBiosConfig = 0x75;
    static constexpr Cmd cmdOemSetBiosConfig = 0x74;
    static constexpr Cmd cmdOemSetBiosConfigInfo = 0x76;

    static constexpr auto EEPROM_SERVICE = "com.inventec.EepromDevice";
    static constexpr auto EEPROM_OBJECT = "/com/inventec/EepromDevice";
    static constexpr auto EEPROM_INTERFACE = "com.inventec.MsEepromManager";
}// namespace cmdsEEpromInventec
constexpr uint8_t BMC_INTF_NONE = 0x0;
constexpr uint8_t BMC_INTF_SUPPORTED = 0x1;
constexpr uint8_t BMC_INTF_ACTIVE = 0x2;

#ifdef SUPPORT_BIOS_OEM_CMD
namespace cmdsNetFnBios
{
    constexpr Cmd cmdGetBmcInfStatus = 0x3d;
    constexpr Cmd cmdEnableVHub = 0xaa;
    constexpr Cmd cmdGetVHubStatus = 0xab;
} // namespace cmdsNetFnBios

constexpr uint8_t BIOS_INTF_NONE = 0x0;
constexpr uint8_t BIOS_INTF_STARTED = 0x2;
constexpr uint8_t BIOS_INTF_ERROR = 0x4;
constexpr uint8_t BIOS_INTF_READY = 0x6;

constexpr uint8_t BIOS_IPMI_USB_INTF = 0x1;
constexpr uint8_t BIOS_LAN_USB_INTF = 0x2;
constexpr uint8_t BIOS_REDFISH_INTF = 0x4;
#endif //SUPPORT_BIOS_OEM_CMD

} // namespace inv
} // namespace ipmi
