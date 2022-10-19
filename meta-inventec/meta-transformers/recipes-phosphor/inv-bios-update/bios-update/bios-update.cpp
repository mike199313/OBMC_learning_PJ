#include "bios-update.hpp"
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <gpiod.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/message/types.hpp>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>


static constexpr uint8_t meAddress = 1;
static constexpr uint8_t lun = 0;
static constexpr uint8_t ME_COMMAND_ADDR = 1;
static constexpr uint8_t ME_NET_FN_2E = 0x2e;
static constexpr uint8_t ME_NET_FN_06 = 0x6;
static constexpr uint8_t FORCE_ME_RECOVERY_CMD = 0xdf;
static constexpr uint8_t GET_SELF_TESTS_RESULT_CMD = 0x4;
static constexpr uint8_t ME_RESET_CMD = 0x2;

constexpr auto SYSTEMD_SERVICE = "org.freedesktop.systemd1";
constexpr auto SYSTEMD_ROOT = "/org/freedesktop/systemd1";
constexpr auto SYSTEMD_INTERFACE = "org.freedesktop.systemd1.Manager";

constexpr auto MONITOR_PROGRESS_SERVICE = "monitor-bios-update@";

constexpr auto MTD_NUM_MAX = 15;
constexpr auto FLASHCP_CNT = 500;
int8_t BiosUpdateManager::setMeToRecoveryMode()
{
    uint8_t commandAddress = ME_COMMAND_ADDR;
    uint8_t netfn = ME_NET_FN_2E;
    uint8_t command = FORCE_ME_RECOVERY_CMD;

    //Byte 1:3 = Intel Manufacturer ID – 000157h, LS byte first.
    //Byte 4 – Command = 01h Restart using Recovery Firmware
    std::vector<uint8_t> commandData = {0x57, 0x01, 0x00, 0x1};
    
    std::tuple<int, uint8_t, uint8_t, uint8_t, uint8_t, std::vector<uint8_t>>
        response_data;

    auto bus = sdbusplus::bus::new_default_system();
    auto method = bus.new_method_call(IPMB_BRIDGE_OBJ, IPMB_BRIDGE_PATH,
                                      IPMB_BRIDGE_INTERFACE, "sendRequest");

    try
    {
        method.append(commandAddress, netfn, lun, command, commandData);
        auto ret = bus.call(method);
        ret.read(response_data);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        printf("%s sdbus error\n", __func__);
        return -1;
    }
    
    std::vector<uint8_t> data_received(3,0);
    int status = -1;
    uint8_t netFn = 0, lun = 0, cmd = 0, cc = 0;
        	
    std::tie(status, netFn, lun, cmd, cc, data_received) = response_data;
    if (data_received.size() < 2) {
        printf("%s receive fail st:%d\n", __func__, status);
	return -1;
    }
    if (data_received.at(0) == 0x57 && data_received.at(1) == 0x01)
    {
        BIOS_UPDATE_DEBUG("%s set ME recovery mode fail\n", __func__);
    } else {
        printf("%s set ME recovery mode fail\n", __func__);
	return -1;
    }
    return 0;
}

int8_t BiosUpdateManager::checkMeToRecoveryMode()
{
    uint8_t commandAddress = ME_COMMAND_ADDR;
    uint8_t netfn = ME_NET_FN_06;
    uint8_t command = GET_SELF_TESTS_RESULT_CMD;

    std::vector<uint8_t> commandData = {};

    std::tuple<int, uint8_t, uint8_t, uint8_t, uint8_t, std::vector<uint8_t>>
        response_data;

    isRecoveryMode = false;

    auto bus = sdbusplus::bus::new_default_system();
    try
    {
        auto method = bus.new_method_call(IPMB_BRIDGE_OBJ, IPMB_BRIDGE_PATH,
                                          IPMB_BRIDGE_INTERFACE, "sendRequest");
        method.append(commandAddress, netfn, lun, command, commandData);
        auto ret = bus.call(method);
        ret.read(response_data);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        printf("%s sdbus error\n", __func__);
        return -1;
    }

    std::vector<uint8_t> data_received(2,0);
    int status = -1;
    uint8_t netFn = 0, lun = 0, cmd = 0, cc = 0;

    std::tie(status, netFn, lun, cmd, cc, data_received) = response_data;

    if (data_received.size() < 2) {
        printf("%s response fail st:%d\n", __func__, status);
        return -1;
    }
    // Byte 1 : 81h = Firmware entered recovery boot-loader mode
    // Byte 2 : 02h = Recovery mode entered by IPMI command "Force ME Recovery"
    if (data_received.at(0) == 0x81 && data_received.at(1) == 0x02)
    {
        isRecoveryMode = true;
    }
    return 0;
}

int8_t BiosUpdateManager::resetMeToBoot()
{
    uint8_t commandAddress = ME_COMMAND_ADDR;
    uint8_t netfn = ME_NET_FN_06;
    uint8_t command = ME_RESET_CMD;
    std::vector<uint8_t> commandData = {};

    auto bus = sdbusplus::bus::new_default_system();
    auto method = bus.new_method_call(IPMB_BRIDGE_OBJ, IPMB_BRIDGE_PATH,
                                      IPMB_BRIDGE_INTERFACE, "sendRequest");

    method.append(commandAddress, netfn, lun, command, commandData);
    try
    {
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        return -1;
    }

    return 0;
}

int8_t BiosUpdateManager::setBiosMtdDevice(uint8_t state)
{
    int ret = 0;
    std::string spi = BIOS_SPI;
    std::string path = BIOS_DRIVER_PATH;
    int fd;

    switch (state)
    {
    case unbind:
        path = path + "unbind";
        break;
    case bind:
        path = path + "bind";
        break;
    default:
        std::cerr << "Fail to get state failed\n";
        return -1;
    }

    fd = open(path.c_str(), O_WRONLY);
    if (fd < 0)
    {
        std::cerr << "Fail in " << __func__ << "\n";
        return -1;
    }

    write(fd, spi.c_str(), spi.size());
    close(fd);

    return 0;
}

int8_t BiosUpdateManager::biosUpdatePrepare()
{
    int ret = 0;
    int retry = 0;

    std::cout << "Set ME to recovery mode\n";

    sleep(1);
    while (retry < MAX_RETRY_RECOVERY_MODE)
    {
        ret = setMeToRecoveryMode();
        if (ret < 0)
        {
            std::cerr << "Fail to set ME to recovery mode\n";
        }

        sleep(2);

        // check ME status using get self-test result command.
        ret = checkMeToRecoveryMode();
        if (ret == 0 && isRecoveryMode)
        {
            std::cout << "ME is in recovery mode\n";
            break;
        }

        std::cout << "Failed to set ME to recovery mode, Retry!!\n";
        retry++;
        sleep(5);
    }

    if (retry == MAX_RETRY_RECOVERY_MODE)
    {
        std::cerr << "Force to Update\n";
    }

    sleep(1);
    // Set BIOS SPI MUX path to BMC (H) 
    gpiod_ctxless_set_value(GIPO_CHIPNAME,          // Label of the gpiochip.
                            BIOS_SPI_MUX_CTRL,     // Number of GPIO pin.
                            1,                    // GPIO set value.
                            false,                // The active state of this line - true if low.
                            "bios-update",       // Name of comsumer.
                            NULL,                 // Callback function.
                            NULL);                // value passed to callback function.
    sleep(1);
    ret = setBiosMtdDevice(bind);
    if (ret < 0)
    {
        std::cerr << "Failed in bind mtd partition\n";
        return -1;
    }
    sleep(1);
    return 0;
}
/*the function start report-update-progress service , only can invoke while object is found*/
int8_t BiosUpdateManager::setMonitorProgress(std::string object, enum service_action action)
{
    if (object == "") {
        return -1;		
    }
    std::string serviceAction="StopUnit";
    /*format of object : "object:/xyz/openbmc_project/software/verId"*/
    std::string match = "software/";
    std::size_t pos =match.size() + object.find(match);
    std::string verId = object.substr(pos);
    BIOS_UPDATE_DEBUG("verId:%s\n", verId.c_str());
    if (action == SERVICE_START) {
	serviceAction="StartUnit";
    } else {
	serviceAction="StoptUnit";
    }    
    auto bus = sdbusplus::bus::new_default();
    auto method = bus.new_method_call(SYSTEMD_SERVICE, SYSTEMD_ROOT,
                                      SYSTEMD_INTERFACE, serviceAction.c_str());
    auto reportService = MONITOR_PROGRESS_SERVICE + verId + ".service";
    method.append(reportService, "replace");
    try
    {
        auto reply = bus.call(method);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        printf("sd bus call fail\n");
        return -1;
    }
    return 0;
}

int8_t BiosUpdateManager::getMtdNum()
{
    int8_t num;
    int8_t i;
    char path[32]; 
    char buf[10]; 
    int fd = 0; 
    int len = 0;

    for (i = 0; i < MTD_NUM_MAX; i++) {    
        memset(path, '\0', sizeof(path));
        memset(buf, 0, sizeof(buf));
        
        snprintf(path, sizeof(path), "/sys/class/mtd/mtd%d/name", i); 
        if ((fd = open(path, O_RDONLY, S_IRWXU)) < 0)  {
            BIOS_UPDATE_DEBUG("fail fd:%d\n", fd);  
	    continue;
	}
        len = read(fd, buf, sizeof(buf));
        if (len > 0) {
	    buf[len-1] = '\0';
            std::string name(buf);
            if (name.compare(BIOS_MTD_NAME) == 0) {
                BIOS_UPDATE_DEBUG("find name %s\n", name.c_str());  
                return i;
            } 	
        } 
    }

    printf("could not find mtd device!\n");  
    return -1;    
} 

int8_t BiosUpdateManager::biosUpdate(const char* image_str)
{
    int ret = 0;
    char buf[128];  
    FILE *pp;  
    int8_t num = getMtdNum();
    uint32_t cnt = 0;
    if (num < 0) {
        return -1;
    }

    std::string mtdNum = std::to_string(num);

    std::string flashcp_str = "/usr/sbin/flashcp -v ";
    /*mtd6: 04000000 00010000 "pnor"*/
    flashcp_str = flashcp_str + image_str + " /dev/mtd" + mtdNum;
     
    printf("start bios-update\n %s\n", flashcp_str.c_str());  
    if ((pp = popen(flashcp_str.c_str(), "r")) == NULL) {  
       printf("popen() error!\n");  
       return -1;
    }  
    while (fgets(buf, sizeof(buf), pp)) {  
        if (cnt++ <= FLASHCP_CNT) {
            cnt = 0;
	    //printf("%s", buf); 
	    printf("flashcp is running...\n"); 
	} 
    } 
    
    ret = WEXITSTATUS(pclose(pp)); 
    if (ret == 0) {
        printf("flashcp pass!\n"); 
    } else {
        printf("flashcp fail!\n"); 
    }
    BIOS_UPDATE_DEBUG("Exit code: %d\n", ret);
  
    return ret;
}
#if 0
<reserved>
int8_t BiosUpdateManager::verify(const char* image_str, uint32_t imageSize)
{
    int fd = 0;
    int file_fd = 0;
    mtd_info_t mtd_info;           // the MTD structure
    erase_info_t ei;               // the erase block structure
    int len = 0;
    int i = 0;
    unsigned char read_buf[32] = {0x00};                // empty array for reading

    
    file_fd = open(image_str, O_RDWR);


    len = read(file_fd, read_buf, sizeof(read_buf)); // read 20 bytes
    if (len < 0) {
        printf("len fail\n");
        close(file_fd);
	return -1;	
    }	
    // sanity check, should be all 0xFF if erase worked
    for(i = 16; i < 32; i++) {
        printf("file_fd buf[%d] = 0x%02x\n", i, (unsigned int)read_buf[i]);
    }
    close(file_fd);

    memset(read_buf, 0, 32);

    fd = open("/dev/mtd6", O_RDWR); // open the mtd device for reading and 
                                        // writing. Note you want mtd0 not mtdblock0
                                        // also you probably need to open permissions
                                        // to the dev (sudo chmod 777 /dev/mtd0)
    if (fd < 0) {
        printf("fd open fail\n");
	return -1;	
    }
    ioctl(fd, MEMGETINFO, &mtd_info);   // get the device info

    // dump it for a sanity check, should match what's in /proc/mtd
    printf("MTD Type: %x\nMTD total size: %x bytes\nMTD erase size: %x bytes\n",
           mtd_info.type, mtd_info.size, mtd_info.erasesize);
    lseek(fd, 0, SEEK_SET);               // go to the first block
    len = read(fd, read_buf, sizeof(read_buf)); // read 20 bytes
    if (len < 0) {
        printf("len fail\n");
        close(fd);
	return -1;	
    }	
    // sanity check, should be all 0xFF if erase worked
    for(i = 16; i < 32; i++) {
        printf("buf[%d] = 0x%02x\n", i, (unsigned int)read_buf[i]);
    }
    close(fd);
    return 0;
}
#endif
int8_t BiosUpdateManager::biosUpdateFinished(const char* image_str)
{
    int ret = 0;

    sleep(1);

    ret = setBiosMtdDevice(unbind);
    if (ret < 0)
    {
        std::cerr << "Failed in unbind mtd partition\n";
        return -1;
    }
    sleep(2);
    
    // Set BIOS SPI MUX path to Host (L)
    gpiod_ctxless_set_value(GIPO_CHIPNAME,          // Label of the gpiochip.
                            BIOS_SPI_MUX_CTRL,     // Number of GPIO pin.
                            0,                    // GPIO set value.
                            false,                // The active state of this line - true if low.
                            "bios-update",       // Name of comsumer.
                            NULL,                 // Callback function.
                            NULL);                // value passed to callback function.
    sleep(1);

    std::cout << "Reset ME to boot\n";
    ret = resetMeToBoot();
    if (ret < 0)
    {
        std::cerr << "Fail in reset ME to boot\n";
        return -1;
    }
    //alang <TBD> do we need to remove image?
    //remove(image_str);

    return 0;
}
