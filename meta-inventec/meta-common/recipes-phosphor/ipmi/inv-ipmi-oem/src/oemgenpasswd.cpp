#include <oemcommands.hpp>
#include <commandutils.hpp>

#include <ipmid/api.hpp>
#include <ipmid/utils.hpp>
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
#include <user_channel/user_layer.hpp>
#include <ipmid/sessiondef.hpp>
#include <ipmid/sessionhelper.hpp>

using namespace std;
using namespace ipmi::inv::cmdsNetFnInventec;

namespace ipmi
{
//cwsun : link need -luserlayer

/** @brief Sets username into ipmi user database.
 *  @param[in] username - the username that will create in the ipmi user database.
 *  @return uid of the user
 */
std::optional<uint8_t> invOemSetUsername(std::string username)
{
    uint8_t START_USER_ID = 15;
    uint8_t uid;
    for (uid = START_USER_ID; uid > 0 ; uid--)
    {
        //find an empty slot for store the username
        std::string tmpu;
        Cc ret = ipmiUserGetUserName(uid, tmpu);
        if (ret == ccSuccess)
        {
            if (tmpu.size() == 0)
            {
                //got empty slot
                break;
            }
            else
            {
                continue;
            }
        }
    }

    if (uid == 0)
    {
        //uid >=1 is good
        return std::nullopt;
    }

    Cc rc = ipmi::ipmiUserSetUserName(uid, username);
    if (rc != ccSuccess)
    {
        return std::nullopt;
    }
    return uid;
}

/** @brief generate random password for the specified user. It will stored in the ipmi user database
 *  @param[in] uid - the uid
 *  @return the generated password string
 */
std::optional<std::string> invOemSetPasswordByrandom(uint8_t uid)
{
    char password[MAX_PASSWORD_LENGTH + 1];
    bzero(password, MAX_PASSWORD_LENGTH + 1);
    std::srand(std::time(nullptr));
    for (int i = 0; i < MAX_PASSWORD_LENGTH; i++)
    {
        char ch;
        while (!std::isalnum(ch = static_cast<char>(std::rand())))
            ;
        password[i] = ch;
    }

    Cc ret = ipmi::ipmiUserSetUserPassword(uid, password);

    if (ret != ccSuccess)
    {
        return std::nullopt;
    }

    return std::string(password);
}

/** @brief delete user from the ipmi user database
 *  @param[in] username - the username that will create in the ipmi user database.
 *  @return true as success otherwise failed
 */
bool invOemDeleteUser(std::string username)
{
    std::string emptyUsername;
    uint8_t uid = ipmiUserGetUserId(username);
    if (0xFF == uid)
        return false;

    Cc cc = ipmiClearUserEntryPassword(username);
    cc += ipmi::ipmiUserSetUserName(uid, emptyUsername);
    return (cc == ccSuccess);
}

/** @brief If the watchdog is mark as Enabled, the HOST OS will be viewed as booted completely
 *  @return true as HOST OS booted completed
 */
bool ifHostOSBootedComplete(void)
{
    //get watchdog status to indicate the OS booted status
    //BUT the HOST OS may not set the Watchdog properties
    bool osBooted = true;
    try
    {
        sdbusplus::bus::bus bus(ipmid_get_sd_bus_connection());
        auto valEnabled = ipmi::getDbusProperty(bus,
                                                WATCHDOG_SERVICE,
                                                WATCHDOG_HOST0_OBJECT,
                                                WATCHDOG_STATE_INTERFACE,
                                                "Enabled");

        auto valTmrUse = ipmi::getDbusProperty(bus,
                                               WATCHDOG_SERVICE,
                                               WATCHDOG_HOST0_OBJECT,
                                               WATCHDOG_STATE_INTERFACE,
                                               "CurrentTimerUse");

        fprintf(stderr, "valTmrUse=%s \n", std::get<std::string>(valTmrUse).c_str());
        fprintf(stderr, "valEnabled=%d \n", static_cast<uint8_t>(std::get<bool>(valEnabled)));

        bool flagTmrUse = (std::get<std::string>(valTmrUse) == "xyz.openbmc_project.State.Watchdog.TimerUse.BIOSFRB2");
        osBooted = !(std::get<bool>(valEnabled) && flagTmrUse);
        return osBooted;
    }
    catch (std::exception &e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Fail ifHostOSBootedCompleted ",
            phosphor::logging::entry("ERROR=%s", e.what()));
        fprintf(stderr, "ifHostOSBootedCompleted exception %s \n", e.what());
    }
    return osBooted;
}

/** @brief check if the host interface (usb0) is ready for service
 *  @return If ready, retrun true.
 */
bool ifHostInterfaceReady(void)
{
    //Check if the usb0 network device is ready
    bool flag = false;
    try
    {
        sdbusplus::bus::bus bus(ipmid_get_sd_bus_connection());
        auto valNICEnabled = ipmi::getDbusProperty(bus,
                                                   NETWORK_SERVICE,
                                                   NETWORK_USB0_OBJECT,
                                                   NETWORK_ETH_INTERFACE,
                                                   "NICEnabled");
        flag = std::get<bool>(valNICEnabled);
        fprintf(stderr, "nicEnabled=%d \n", std::get<bool>(valNICEnabled));
        return flag;
    }
    catch (std::exception &e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Fail ifHostInterfaceReady ",
            phosphor::logging::entry("ERROR=%s", e.what()));
    }
    return flag;
}

/** @brief Get All IPMI Sessions
 *  @return The session object path in dbus object path format
 */
std::vector<std::string> getAllIpmiSessions(void)
{
    std::vector<std::string> objects;

    sdbusplus::bus::bus bus(ipmid_get_sd_bus_connection());
    try
    {
        ipmi::ObjectTree objectTree = ipmi::getAllDbusObjects(
            bus, session::sessionManagerRootPath, session::sessionIntf);
        fprintf(stderr, "root path=%s , intf path=%s \n", session::sessionManagerRootPath, session::sessionIntf);

        for (auto &objectTreeItr : objectTree)
        {
            //DbusObjectPath
            const std::string obj = objectTreeItr.first;
            fprintf(stderr, "SessionObj path=%s \n", objectTreeItr.first.c_str());
            objects.push_back(obj);
        }
    }
    catch (sdbusplus::exception::SdBusError &e)
    {
        fprintf(stderr, "exception %s\n", e.what());
    }
    return objects;
}

/** @brief Mark the specified ipmi session as "tear Down In progress" state. The system will do garbage collection.
 *  @param[in] username - the username that will create in the ipmi user database.
 *  @param[in] sessionObjects - All the ipmi sessions formated by dbus object path format.
 */
void deleteUserSession(const std::vector<std::string> sessionObjects, std::string username)
{
    try
    {
        sdbusplus::bus::bus bus(ipmid_get_sd_bus_connection());
        for (auto objPath : sessionObjects)
        {

            auto userID = ipmi::getDbusProperty(bus,
                                                IPMI_SESSION_SERVICE,
                                                objPath,
                                                IPMI_SESSION_SESSIONINFO_INTERFACE,
                                                "UserID");

            uint8_t uid = ipmiUserGetUserId(username);
            if (uid == std::get<uint8_t>(userID))
            {
                setDbusProperty(bus,
                                IPMI_SESSION_SERVICE,
                                objPath,
                                IPMI_SESSION_SESSIONINFO_INTERFACE,
                                "State",
                                static_cast<uint8_t>(session::State::tearDownInProgress));
                fprintf(stderr, "Session %s set as tearDownInProgress", objPath.c_str());
            }
        }
    }
    catch (sdbusplus::exception::SdBusError &e)
    {
        fprintf(stderr, "exception %s\n", e.what());
    }
}

/** @brief Get system HI Interface configuration
 *  @return the config bitbask as HOST INTERFACE ENABLED, KERNAL AUTH ENABLED, FIRMWARE AUTH ENABLED.
 */
uint32_t getHIInterfaceSupport(void)
{
    uint32_t flag = 0x00;
    bool ret = ifHostInterfaceReady();
    if (ret)
    {
        flag |= _HOST_INTERFACE_ENABLED;
    }

    //TODO: get kernal_auth and firmware_auth from dbus interface ?
    flag |= _KERNEL_AUTH_ENABLED;
    flag |= _FIRMWARE_AUTH_ENABLED;

    fprintf(stderr, "getHIInterfaceSupport flag=%08X \n", flag);

    return flag;
}

/** @brief Check if the parameter is one of BIOS USER, OS USER, DELETE FW USER, DELETE OS USER commands.
 *  @param[in] p the command id
 *  @return true as in the specified value range.
 */
bool checkParamIsOK(const uint8_t p)
{
    switch (p)
    {
    case _BIOS_USER:
    case _OS_USER:
    case _DELETE_FW_USER:
    case _DELETE_OS_USER:
        return true;
    }
    return false;
}

ipmi::RspType<message::Payload> ipmiOemGenerateRandomPassword(const uint8_t paramSelector, const uint8_t bmcInst)
{
    message::Payload ret;
    std::string username;

    if (checkParamIsOK(paramSelector) == false)
    {
        return responseParmOutOfRange();
    }

    uint32_t interfaceSupport = getHIInterfaceSupport();
    if (!(interfaceSupport & _HOST_INTERFACE_ENABLED))
    {

        std::vector<std::string> sessionObjects = getAllIpmiSessions();
        deleteUserSession(sessionObjects, FWUsername);
        deleteUserSession(sessionObjects, FWUsername);
        invOemDeleteUser(FWUsername);
        invOemDeleteUser(OSUsername);
        phosphor::logging::log<phosphor::logging::level::ERR>("The HOST Interface is not ready, remove both account");
        return responseHostInterfaceNotReady(); //0x85
    }

    auto flagHostOSBootedCompleted = ifHostOSBootedComplete();
    if (((paramSelector == _BIOS_USER) || (paramSelector == _OS_USER)) &&
        flagHostOSBootedCompleted)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>("The HOST OS Boot complete, command not available");
        return responseCommandNotAvailable(); //0xD5
    }


    if ((_BIOS_USER == paramSelector) && (interfaceSupport & _FIRMWARE_AUTH_ENABLED))
    {
        username = FWUsername;
        std::vector<std::string> sessionObjects = getAllIpmiSessions();
        deleteUserSession(sessionObjects, OSUsername);
        invOemDeleteUser(OSUsername);
    }
    else if ((paramSelector == _OS_USER) && (interfaceSupport & _KERNEL_AUTH_ENABLED))
    {
        username = OSUsername;
    }
    else if (_DELETE_FW_USER == paramSelector)
    {
        std::vector<std::string> sessionObjects = getAllIpmiSessions();
        deleteUserSession(sessionObjects, FWUsername);
        bool ret = invOemDeleteUser(FWUsername);
        if (ret)
        {
            return ipmi::responseSuccess();
        }
        else
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(
                "The username delete failed",
                phosphor::logging::entry("username=%s", FWUsername.c_str()));
            return ipmi::responseUnspecifiedError();
        }
    }
    else if (_DELETE_OS_USER == paramSelector)
    {
        std::vector<std::string> sessionObjects = getAllIpmiSessions();
        deleteUserSession(sessionObjects, OSUsername);
        bool ret = invOemDeleteUser(OSUsername);
        if (ret)
        {
            return ipmi::responseSuccess();
        }
        else
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(
                "The username delete failed",
                phosphor::logging::entry("username=%s", OSUsername.c_str()));
            return ipmi::responseUnspecifiedError();
        }
    }
    else
    {
        std::vector<std::string> sessionObjects = getAllIpmiSessions();
        deleteUserSession(sessionObjects, FWUsername);
        deleteUserSession(sessionObjects, OSUsername);
        bool ret1 = invOemDeleteUser(OSUsername);
        bool ret2 = invOemDeleteUser(FWUsername);
        if (ret1 && ret2)
        {
            return ipmi::responseSuccess();
        }
        else
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(
                "Deleteing BOTH user failed",
                phosphor::logging::entry("username=%s", username.c_str()));
            return ipmi::responseUnspecifiedError();
        }
    }

    //Check if the username is already existing in the ipmi user database
    auto uid1 = ipmiUserGetUserId(username);
    if (0xFF != uid1)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "The username already exists",
            phosphor::logging::entry("username=%s", username.c_str()));
        return ipmi::responseUnspecifiedError();
    }

    //set the username into the ipmi user database and get the assigned user id
    auto _opt1 = invOemSetUsername(username);
    if (std::nullopt == _opt1)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "The username adding failed",
            phosphor::logging::entry("username=%s", username.c_str()));
        return ipmi::responseUnspecifiedError();
    }
    uint8_t uid = _opt1.value();
    std::vector<uint8_t> retUsername(username.begin(), username.end());

    //enable the user id
    bool enableState = true;
    ipmiUserUpdateEnabledState(uid, enableState);

    //set the pvivilege
    ipmi::PrivAccess pa;
    pa.privilege = 0x04; //Administrator
    pa.ipmiEnabled = 0x01;
    pa.linkAuthEnabled = 0x01;
    pa.accessCallback = 0x01;
    bool otherPrivUpdates = true;
    ipmiUserSetPrivilegeAccess(uid, 0x01, pa, otherPrivUpdates);

    //generate a random password for the specified userid
    auto _opt2 = invOemSetPasswordByrandom(uid);
    if (!_opt2)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "The user password creating failed",
            phosphor::logging::entry("username=%s", username.c_str()));
        return ipmi::responseUnspecifiedError();
    }
    std::string password = _opt2.value();
    std::vector<uint8_t> retPassword(password.begin(), password.end());

    //pack the username length and pack the password length
    ret.pack(static_cast<uint8_t>(retUsername.size()));
    ret.pack(static_cast<uint8_t>(retPassword.size()));
    ret.pack(retUsername);
    ret.pack(retPassword);

    return ipmi::responseSuccess(std::move(ret));
}


}//end of namespace ipmi
