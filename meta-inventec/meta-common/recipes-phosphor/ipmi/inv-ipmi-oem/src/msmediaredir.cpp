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
#include <sstream>
#include <iomanip>

#include <sys/mount.h>
#include <sys/stat.h>
#include <dirent.h>

#define loginfo(...)                                                      \
    do                                                                  \
    {                                                                   \
        {                                                         \
            loginfo_impl_v1(__FILE__, __LINE__, __func__, __VA_ARGS__); \
        }                                                               \
    } while (0)

[[maybe_unused]] inline static void loginfo_impl_v1(const char *srcname, int linenum, const char *funcname, const char *fmt, ...)
{
    va_list ap;
    char *filename = basename(strdup(srcname));

    fprintf(stderr, "{%s:%d:%s}:", filename, linenum, funcname);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fflush(stderr);
}

const uint8_t RIS_TYPE_RIS = 0x08;
const uint8_t RIS_TYPE_HD = 0x04;

using namespace std;
using namespace ipmi::inv::cmdsNetFnMsMediaRedirect;
using Json = nlohmann::json;

namespace ipmi
{
    const std::string MEDIA_REDIRECT_CONFIG_FILE = "/usr/share/ipmi-providers/ms_mediaredirect.json";

    static int writeJsonFile(const std::string &configFile,
                                     const Json &jsonData)
    {
        const std::string tmpFile = configFile + "_tmp";
        int fd = open(tmpFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC | O_SYNC,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd < 0)
        {
           fprintf(stderr, "Error createing %s file \n", MEDIA_REDIRECT_CONFIG_FILE.c_str());
            return -EIO;
        }
        const auto &writeData = jsonData.dump(4);
        if (write(fd, writeData.c_str(), writeData.size()) !=
            static_cast<ssize_t>(writeData.size()))
        {
            close(fd);
            fprintf(stderr, "Error writing %s file \n", MEDIA_REDIRECT_CONFIG_FILE.c_str());
            return -EIO;
        }
        close(fd);

        if (std::rename(tmpFile.c_str(), configFile.c_str()) != 0)
        {
            fprintf(stderr, "Error renaming %s file \n", MEDIA_REDIRECT_CONFIG_FILE.c_str());
            return -EIO;
        }

        return 0;
    }

    static Json readJsonFile(const std::string &configFile)
    {
        std::ifstream jsonFile(configFile);
        if (!jsonFile.good())
        {
            fprintf(stderr, "Not found %s file \n", MEDIA_REDIRECT_CONFIG_FILE.c_str());
            return nullptr;
        }

        Json data = nullptr;
        try
        {
            data = Json::parse(jsonFile, nullptr, false);
        }
        catch (const std::exception &e)
        {
            fprintf(stderr, "JSON parsing error %s file \n", MEDIA_REDIRECT_CONFIG_FILE.c_str());
            return nullptr;
        }

        return data;
    }

    template <typename T>
    static int updateRisProperty(std::string serviceType, std::string pName, T value)
    {
        try
        {
            Json outData = readJsonFile(MEDIA_REDIRECT_CONFIG_FILE);
            Json jobj;
            jobj[pName] = value;
            outData["RIS"][serviceType].update(jobj);
            if (writeJsonFile(MEDIA_REDIRECT_CONFIG_FILE, outData) != 0)
            {
                fprintf(stderr, "Error writing %s:%d %s \n", __func__, __LINE__, MEDIA_REDIRECT_CONFIG_FILE.c_str());
                return -1;
            }
        }
        catch (std::exception &e)
        {
            fprintf(stderr, ":%s:%d Exception :%s\n", __func__, __LINE__, e.what());
            return -2;
        }
        return 0;
    }

    template <typename T>
    static int updateMediaRedirectProperty(std::string mediaType,
                                                    std::string imageType,
                                                    std::string imageIndex, 
                                                    std::string pName, T value)
    {
        try
        {
            Json outData = readJsonFile(MEDIA_REDIRECT_CONFIG_FILE);
            int index = 0;
            for(auto& obj : outData["MEDIARedirect"]){
                if(
                    (obj["Mediatype"] == mediaType) 
                    && (obj["Imagetype"] == imageType)
                    && (obj["Imageindex"] == imageIndex)
                ){
                    obj[pName] = value;
                    outData["MEDIARedirect"].at(index) = obj;
                }
                index++;
            }
            
            if (writeJsonFile(MEDIA_REDIRECT_CONFIG_FILE, outData) != 0)
            {
                fprintf(stderr, "Error writing %s:%d %s \n", __func__, __LINE__, MEDIA_REDIRECT_CONFIG_FILE.c_str());
                return -1;
            }
        }
        catch (std::exception &e)
        {
            fprintf(stderr, ":%s:%d Exception :%s\n", __func__, __LINE__, e.what());
            return -2;
        }
        return 0;
    }

    static bool isRISMounted(std::string serviceType)
    {
        //MS SPEC : mounted is 0, unmounted is 1
        try{
            Json root = readJsonFile(MEDIA_REDIRECT_CONFIG_FILE);
            return (root["RIS"][serviceType]["Mountstatus"].get<uint32_t>() == 0)  ? true : false;
        }catch(std::exception& e){
            return false;
        }
    }

    ipmi::RspType<message::Payload> ipmiOemGetRis( uint8_t st, uint8_t paramSelector)
    {
        fprintf(stderr, "%s:%d \n", __func__, __LINE__);
        message::Payload ret;
        Json root = readJsonFile(MEDIA_REDIRECT_CONFIG_FILE);
        if(root == nullptr){
            return ipmi::responseUnspecifiedError();
            fprintf(stderr, "readJsonFile() return nullptr \n");
        }

        std::stringstream ss;
        ss << static_cast<int>(st);
        std::string serviceType = ss.str();
        loginfo("serviceType=%s \n", serviceType.c_str());

        switch(paramSelector)
        {
            case 0:{ //Get Image name
                try{
                    Json param = root["RIS"][serviceType]["Imagename"];
                    ret.pack(st);
                    ret.pack(paramSelector);
                    for(auto& c : param.get<std::string>()){
                        ret.pack(c);
                    }
                    return ipmi::responseSuccess(std::move(ret));
                }catch(std::exception& e){
                    fprintf(stderr, "%s:%d exception : %s \n", __func__, __LINE__, e.what());
                }
            }break;

            case 1:{ // Get the Source path
                Json param = root["RIS"][serviceType]["Sourcepath"];
                ret.pack(st);
                ret.pack(paramSelector);
                for(auto& c : param.get<std::string>()){
                    ret.pack(c);
                }
                return ipmi::responseSuccess(std::move(ret));
            }break;

            case 2:{ //Get the IPaddress
                Json param = root["RIS"][serviceType]["IPaddress"];
                ret.pack(st);
                ret.pack(paramSelector);
                for(auto& c : param.get<std::string>()){
                    ret.pack(c);
                }
                return ipmi::responseSuccess(std::move(ret));
            }break;

            case 5:{ //Share Type
                Json param = root["RIS"][serviceType]["Sharetype"];
                ret.pack(st);
                ret.pack(paramSelector);
                for(auto& c : param.get<std::string>()){
                    ret.pack(c);
                }
                return ipmi::responseSuccess(std::move(ret));
            }break;

            case 7:{ //Start mount , read only
                Json param = root["RIS"][serviceType]["Startmount"];
                ret.pack(st);
                ret.pack(paramSelector);
                ret.pack(param.get<uint8_t>() );
                return ipmi::responseSuccess(std::move(ret));
            }break;

            case 8:{//mount status
                Json param = root["RIS"][serviceType]["Mountstatus"];
                ret.pack(st);
                ret.pack(paramSelector);
                ret.pack(param.get<uint8_t>());
                return ipmi::responseSuccess(std::move(ret));
            }break;

            case 10:{// RIS state
                Json param = root["RIS"][serviceType]["RISstate"];
                ret.pack(st);
                ret.pack(paramSelector);
                ret.pack(param.get<uint8_t>() );
                return ipmi::responseSuccess(std::move(ret));
            }break;
        }

        return ipmi::responseInvalidFieldRequest();
    }




    ipmi::RspType<message::Payload> ipmiOemSetRis(uint8_t st, 
                                                                uint8_t paramSelector, 
                                                                uint8_t blockSelector,  
                                                                message::Payload &req)
    {
        fprintf(stderr, "%s:%d \n", __func__, __LINE__);
        message::Payload ret;

        std::vector<uint8_t> vec;
        if (req.unpack(vec) != 0 || !req.fullyUnpacked())
        {
            return ipmi::responseUnspecifiedError();
        }

        std::stringstream ss;
        ss << static_cast<int>(st);
        std::string serviceType = ss.str();
        loginfo("serviceType=%s \n", serviceType.c_str());

        switch (paramSelector)
        {
            case 0:{ //Imagename
                std::string value = std::string(vec.begin(), vec.end());
                int rt = updateRisProperty(serviceType,"Imagename", value);
                if(rt) return ipmi::responseUnspecifiedError();
                return ipmi::responseSuccess();
            }break;

            case 1:{ //Sourcepath
                std::string value = std::string(vec.begin(), vec.end());
                int rt = updateRisProperty(serviceType,"Sourcepath", value);
                if(rt) return ipmi::responseUnspecifiedError();
                return ipmi::responseSuccess();
            }break;

            case 2:{ //IPaddress
                std::string value = std::string(vec.begin(), vec.end());
                int rt = updateRisProperty(serviceType,"IPaddress", value);
                if(rt) return ipmi::responseUnspecifiedError();
                return ipmi::responseSuccess();
            }break;

            case 10:{ //RISstate
                uint8_t value = vec.at(0);
                loginfo("RISstate set to %d \n", value);
                int rt = updateRisProperty(serviceType,"RISstate", value);
                rt += updateRisProperty(serviceType,"Startmount", value);
                if(rt) return ipmi::responseUnspecifiedError();
                return ipmi::responseSuccess();
            }break;
        }

        return ipmi::responseInvalidFieldRequest();
    }

    ipmi::RspType<message::Payload> ipmiOemStartStopRis(uint8_t st,  uint8_t startStop)
    {
        fprintf(stderr, "%s:%d \n", __func__, __LINE__);
        message::Payload ret;

        std::stringstream ss;
        ss << static_cast<int>(st);
        std::string serviceType = ss.str();
        loginfo("serviceType=%s \n", serviceType.c_str());

        Json root = readJsonFile(MEDIA_REDIRECT_CONFIG_FILE);
        if(root == nullptr){
            return ipmi::responseUnspecifiedError();
            fprintf(stderr, "readJsonFile() return nullptr \n");
        }

        uint32_t risStateEnabled = root["RIS"][serviceType]["RISstate"].get<uint32_t>();
        if((risStateEnabled == 0) && (st == RIS_TYPE_RIS)){
            std::string stype = root["RIS"][serviceType]["Servicetype"].get<std::string>();
            fprintf(stderr, "%s:%d %s is disabled \n", __func__, __LINE__, stype.c_str());
            return ipmi::response(ccServiceNotEnabled);
        }

        bool isMounted = isRISMounted(serviceType);
        if(isMounted && (startStop==1)){
            fprintf(stderr, "%s:%d isRISMounted=%s is mounted \n", __func__, __LINE__, 
                        root["RIS"][serviceType]["Servicetype"].get<std::string>().c_str());
            return ipmi::responseSuccess();
        }


        std::string sourcePath = root["RIS"][serviceType]["Sourcepath"].get<std::string>();
        std::string destPath = root["RIS"][serviceType]["Destpath"].get<std::string>();
        std::string ipAddress = root["RIS"][serviceType]["IPaddress"].get<std::string>();
        std::string nbdDevice = root["RIS"][serviceType]["Nbdclient"].get<std::string>();
        std::string imageName = root["RIS"][serviceType]["Imagename"].get<std::string>();
        uint32_t port = root["RIS"][serviceType]["Port"].get<uint32_t>();
        int rt = 0;
        char buff[512];

        switch (startStop)
        {
            case 0:{ //STOP
                //1. umount NBD client
                //2. stop the NBD-server if serviceType=8
                //2. umount NFS if serviceType is Remote Image
                bzero(buff, sizeof(buff));
                sprintf(buff, "/usr/sbin/nbd-client -d %s ", nbdDevice.c_str());
                rt = system(buff);
                loginfo("RUN rt=%d %s \n", rt, buff);
                if (rt){
                    // something error
                    fprintf(stderr, "%s:%d nbd-client  unmount failed\n", __func__, __LINE__);
                }
                if(serviceType == "8"){
                    // It is remote image that shared by NFS,  
                    // umount NBD server of the bmcbox
                    bzero(buff, sizeof(buff));
                    sprintf(buff, "/usr/bin/killall nbd-server");
                    rt = system(buff);
                    loginfo("RUN rt=%d %s \n", rt, buff);
                    
                    //It is remote image,  umount NFS, too.
                    bzero(buff, sizeof(buff));
                    sprintf(buff, "/bin/umount -f %s ", destPath.c_str());
                    rt = system(buff);
                    loginfo("RUN rt=%d %s \n", rt, buff);
                    if (rt){
                        // something error
                        fprintf(stderr, "%s:%d NFS  unmount failed\n", __func__, __LINE__);
                    }
                }
                updateRisProperty(serviceType, "Mountstatus", 1);
                return ipmi::responseSuccess();
            }break;

            case 1:{ //START
                if(serviceType == "8"){
                    //image file, mount NFS first

                    //Checking if the mount point is existing. Creating it if not existing.
                    DIR *dir = opendir(destPath.c_str());
                    if(dir){
                        closedir(dir);
                    }else if(errno == ENOTDIR){
                        fprintf(stderr, "%s:%d %s existing but not DIR \n", __func__, __LINE__, destPath.c_str());
                        return ipmi::responseUnspecifiedError();
                    }else{
                        bzero(buff, sizeof(buff));
                        sprintf(buff, "/bin/mkdir -p %s ", destPath.c_str());
                        rt = system(buff);
                        loginfo("RUN rt=%d %s \n", rt, buff);
                    }

                    // mount nfs
                    bzero(buff, sizeof(buff));
                    sprintf(buff, "/bin/mount -t nfs %s:%s %s ", ipAddress.c_str(), sourcePath.c_str(), destPath.c_str());
                    rt = system(buff);
                    loginfo("RUN rt=%d %s \n", rt, buff);
                    if(rt){
                        //something error
                        fprintf(stderr, "%s:%d NFS mount failed\n", __func__, __LINE__);
                        return ipmi::responseUnspecifiedError();
                    }else{
                        //NFS mount ok, do nbd-server sharing
                        bzero(buff, sizeof(buff));
                        sprintf(buff, "/usr/bin/nbd-server %d %s/%s ", port, destPath.c_str(), imageName.c_str());
                        rt = system(buff);
                        loginfo("RUN rt=%d %s \n", rt, buff);
                        if (rt){
                            // something error
                            fprintf(stderr, "%s:%d nbd-server mount failed\n", __func__, __LINE__);
                            return ipmi::responseUnspecifiedError();
                        }
                    }

                }

                //NBD client mounting
                std::string remoteip;
                if(serviceType=="8"){
                    //mount from 127.0.0.1
                    remoteip = "127.0.0.1";
                }else{
                    remoteip = ipAddress;
                }

                bzero(buff, sizeof(buff));
                sprintf(buff, "/usr/sbin/nbd-client -p -t 30 %s %d %s ", remoteip.c_str(), port, nbdDevice.c_str());
                rt = system(buff);
                loginfo("RUN rt=%d %s \n", rt, buff);
                if (rt){
                    // something error
                    fprintf(stderr, "%s:%d nbd-client mount failed\n", __func__, __LINE__);
                    updateRisProperty(serviceType, "Mountstatus", 1);
                    return ipmi::responseUnspecifiedError();
                }

                //Checking if the nbd mounting is OK
                bzero(buff, sizeof(buff));
                sprintf(buff, "/usr/sbin/nbd-client -c %s", nbdDevice.c_str());
                rt = system(buff);
                loginfo("RUN rt=%d %s \n", rt, buff);
                if (rt){
                    // something error
                    fprintf(stderr, "%s:%d nbd-client mount failed\n", __func__, __LINE__);
                    updateRisProperty(serviceType, "Mountstatus", 1);
                    return ipmi::responseUnspecifiedError();
                }

                updateRisProperty(serviceType, "Mountstatus", 0);
                return ipmi::responseSuccess();
            }break;
        }

        return ipmi::responseInvalidFieldRequest();
    }

    ipmi::RspType<message::Payload> ipmiOemStartStopMediaRedirect(uint8_t paramSelector, message::Payload &req)
    {
        fprintf(stderr, "%s:%d \n", __func__, __LINE__);
        message::Payload ret;
        char buff[512];
        int rt;


        Json root;
        Json mrList;

        std::vector<uint8_t> vec;
        if (req.unpack(vec) != 0 || !req.fullyUnpacked()){
            return ipmi::responseUnspecifiedError();
        }

        int mediaType;
        int imageType;
        int startStop;
        int imageIndex;
        int gadgetNum;
        int isCdrom;
        try
        {
            mediaType = static_cast<int>(vec.at(0));
            imageType = static_cast<int>(vec.at(1));
            startStop = static_cast<int>(vec.at(2));
            imageIndex = static_cast<int>(vec.at(3));

            if( (imageType !=1)
                    && (imageType !=2)
                    && (imageType !=4) ){
                fprintf(stderr, "Invalid Image Type %d. Remote Media Type(1,2,4) only supported \n", imageType);
                return ipmi::responseInvalidFieldRequest();
            }
            
            //1:CD, otherwise act as HD
            isCdrom = (imageType == 1) ? 1 : 0 ;

            root = readJsonFile(MEDIA_REDIRECT_CONFIG_FILE);
            if (root == nullptr)
            {
                return ipmi::responseUnspecifiedError();
                fprintf(stderr, "readJsonFile() return nullptr \n");
            }
            mrList = root["MEDIARedirect"];

        }catch (std::exception &e){
            fprintf(stderr, "%s:%d Exception:%s \n", __func__, __LINE__, e.what());
            return ipmi::responseUnspecifiedError();
        }

        if(paramSelector != 0){
            fprintf(stderr, "Invalid parameterSelector. Only 0 is supported %d\n", paramSelector);
            return ipmi::responseInvalidFieldRequest();
        }

        try{
            switch (paramSelector)
            {
                case 0:
                { // By Index
                    for (auto &o : mrList)
                    {
                        stringstream ss;
                        ss << imageIndex;
                        std::string imageIndexString = ss.str();

                        ss.str("");
                        ss.clear();
                        ss << mediaType;
                        std::string mediaTypeString = ss.str();

                        ss.str("");
                        ss.clear();
                        ss << imageType;
                        std::string imageTypeString = ss.str();
                        
                        if ( (imageIndexString == o["Imageindex"].get<std::string>())
                            && (mediaTypeString == o["Mediatype"].get<std::string>()) 
                            && (imageTypeString == o["Imagetype"].get<std::string>())) 
                        {
                            bzero(buff, sizeof(buff));
                            Json target = root["RIS"][imageIndexString];
                            //  <start|stop> <nbd device> <gadget number> <media type>
                            std::string startstopcmd = (startStop == 1) ? "start" : "stop";
                            sprintf(buff, "/usr/bin/ms_mediaredirect.sh %s %s %d %d ",
                                    startstopcmd.c_str(),
                                    target["Nbdclient"].get<std::string>().c_str(),
                                    o["Gadgetnum"].get<int>(),
                                    isCdrom);
                            rt = system(buff);
                            loginfo("RUN rt=%d %s \n", rt, buff);
                            if(rt){
                                fprintf(stderr, "rt=%d %s failed \n", rt, buff);
                                updateMediaRedirectProperty(mediaTypeString, imageTypeString, imageIndexString, "Redirectionstate", -1);
                                return ipmi::response(inv::cmdsNetFnMsMediaRedirect::ccInvalidMediaRedirectStartStopCommand);
                            }
                            updateMediaRedirectProperty(mediaTypeString, imageTypeString, imageIndexString, "Redirectionstate", startStop);
                            return ipmi::responseSuccess();
                        }
                    }
                    fprintf(stderr, "Start/Stop Media redirect failed. mediaType=%d imageType=%d imageIndex=%d \n", 
                                        mediaType, imageType, imageIndex);
                    return ipmi::responseUnspecifiedError();
                }
                break;
            }
        }catch(std::exception& e){
            fprintf(stderr, "%s:%d Exception %s", __func__, __LINE__, e.what());
            return ipmi::responseUnspecifiedError();
        }

        return ipmi::responseInvalidFieldRequest();
    }



    ipmi::RspType<message::Payload> ipmiOemGetMediaImageInfo(uint8_t paramSelector, message::Payload &req)
    {
        fprintf(stderr, "%s:%d \n", __func__, __LINE__);
        message::Payload ret;
        Json root;

        std::vector<uint8_t> vec;
        if (req.unpack(vec) != 0 || !req.fullyUnpacked())
        {
            return ipmi::responseUnspecifiedError();
        }

        root = readJsonFile(MEDIA_REDIRECT_CONFIG_FILE);
        if (root == nullptr)
        {
            fprintf(stderr, "readJsonFile() return nullptr \n");
            return ipmi::responseUnspecifiedError();
        }

        switch (paramSelector)
        {
            case 0:{
                // Maximum number of possible redirection images count
                // in either the SD or SPI card
                uint8_t count = 0;
                ret.pack(count);
                return responseSuccess(std::move(ret));
            }break; 
           

            case 1: // Configured CD Instances
            case 2: // Configured FD Instances
            case 3: // Configured HD Instances
            {
                int mediaType = static_cast<int>(vec.at(0));
                stringstream ss;
                ss << mediaType;
                std::string mediaTypeString = ss.str();
                std::string imageTypeString;
                if(paramSelector == 1) imageTypeString = std::string("1"); //CD
                if(paramSelector == 2) imageTypeString = std::string("2"); //FD
                if(paramSelector == 3) imageTypeString = std::string("4"); //HD
                uint8_t count = 0;
                for(auto& obj : root["MEDIARedirect"]){
                    if( (obj["Mediatype"] == mediaTypeString)
                        && (obj["Imagetype"]) == imageTypeString ){
                        count++;
                    }
                }
                ret.pack(count);
                return responseSuccess(std::move(ret));
            }break;

             
            case 4:{ //Total number of Available images
                int mediaType = static_cast<int>(vec.at(0));
                stringstream ss;
                ss << mediaType;
                std::string mediaTypeString = ss.str();
                uint8_t count = 0;
                for(auto& obj : root["MEDIARedirect"]){
                    if( (obj["Mediatype"] == mediaTypeString) ){
                        count++;
                    }
                }
                ret.pack(count);
                return ipmi::responseSuccess(std::move(ret));
            }break;

            case 8:{ // Clear the image information

                int mediaType;
                int imageType;
                int imageIndex;
                try
                {
                    stringstream ss;
                    mediaType = static_cast<int>(vec.at(0));
                    ss << mediaType;
                    std::string mediaTypeString = ss.str();
                    
                    ss.str("");
                    ss.clear();
                    imageType = static_cast<int>(vec.at(1));
                    ss << imageType;
                    std::string imageTypeString = ss.str();

                    ss.str("");
                    ss.clear();
                    imageIndex = static_cast<int>(vec.at(2));
                    ss << imageIndex;
                    std::string imageIndexString = ss.str();

                    updateMediaRedirectProperty(mediaTypeString, imageTypeString, imageIndexString, "Redirectionstate", 0);

                    uint8_t retMediaType = vec.at(0);
                    uint8_t retImageType = vec.at(1);
                    uint8_t retImageIndex = vec.at(2);

                    ret.pack(retMediaType);
                    ret.pack(retImageType);
                    ret.pack(retImageIndex);
                    return ipmi::responseSuccess(std::move(ret));

                }catch(std::exception& e){
                    fprintf(stderr, "%s:%d Exception:%s \n", __func__, __LINE__, e.what());
                    return ipmi::responseUnspecifiedError();
                }
            }break;
        }

        return ipmi::responseInvalidFieldRequest();
    }

    ipmi::RspType<message::Payload> ipmiOemSetMediaImageInfo(uint8_t paramSelector, message::Payload &req)
    {
        fprintf(stderr, "%s:%d \n", __func__, __LINE__);
        message::Payload ret;
        Json root;

        std::vector<uint8_t> vec;
        if (req.unpack(vec) != 0 || !req.fullyUnpacked())
        {
            return ipmi::responseUnspecifiedError();
        }

        root = readJsonFile(MEDIA_REDIRECT_CONFIG_FILE);
        if (root == nullptr)
        {
            fprintf(stderr, "readJsonFile() return nullptr \n");
            return ipmi::responseUnspecifiedError();
        }

        switch(paramSelector)
        {
            case 0:{ // Add Image
                return responseParameterNotSupported();
            }
            break;

            case 3:{ // Clear Media Image Info
                int mediaType;
                int imageType;
                int imageIndex;
                try
                {
                    stringstream ss;
                    mediaType = static_cast<int>(vec.at(0));
                    ss << mediaType;
                    std::string mediaTypeString = ss.str();
                    
                    ss.str("");
                    ss.clear();
                    imageType = static_cast<int>(vec.at(1));
                    ss << imageType;
                    std::string imageTypeString = ss.str();

                    ss.str("");
                    ss.clear();
                    imageIndex = static_cast<int>(vec.at(2));
                    ss << imageIndex;
                    std::string imageIndexString = ss.str();

                    updateMediaRedirectProperty(mediaTypeString, imageTypeString, imageIndexString, "Redirectionstate", 0);

                    return ipmi::responseSuccess();

                }catch(std::exception& e){
                    fprintf(stderr, "%s:%d Exception:%s \n", __func__, __LINE__, e.what());
                    return ipmi::responseUnspecifiedError();
                }

            }
        }

        return ipmi::responseInvalidFieldRequest();
    }
}
