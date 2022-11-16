#include <ipmid/api.hpp>
#include <ipmid/utils.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message/types.hpp>
#include <phosphor-logging/log.hpp>
#include <systemd/sd-bus.h>

#include "nlohmann/json.hpp"

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

#include <vector>
#include <filesystem>
#include <fstream>

const constexpr char *CONF_IPMI_HOST_DIR = "/usr/share/ipmi-providers/";
const constexpr char *CONF_ENTITY_MANAGER_DIR = "/usr/share/entity-manager/configurations/";
const constexpr char *CONF_IPMI_PROVIDERS_DIR = "/usr/share/ipmi-providers/";
const constexpr char *CONF_GOLDEN_DIR = "/etc/defconfig/configs";
const constexpr char *CONF_USER_DIR = "/etc/conf/configs/user";
const constexpr char *CONF_PENDING_DIR = "/etc/conf/configs/pending";
const constexpr char *CONF_SCHEMA_FILE = "/etc/conf/configs/schema.json";

namespace fs = std::filesystem;
using json_t = nlohmann::json;

struct ServiceConfigActiveVersionRsp
{
    uint8_t completion_code;
    char service_name[4];
    uint8_t active_code_major_version;
    uint8_t active_code_minor_version;
    uint8_t active_code_aux_version;

    uint8_t active_config_major_version;
    uint8_t active_config_minor_version;
    uint8_t active_config_aux_version;
} __attribute__((packed));

struct ServiceConfigAllVersionsRsp
{
    uint8_t completion_code;
    char service_name[4];
    uint8_t golden_code_major_version;
    uint8_t golden_code_minor_version;
    uint8_t golden_code_aux_version;

    uint8_t user_code_major_version;
    uint8_t user_code_minor_version;
    uint8_t user_code_aux_version;

    uint8_t pending_code_major_version;
    uint8_t pending_code_minor_version;
    uint8_t pending_code_aux_version;

    uint8_t golden_config_major_version;
    uint8_t golden_config_minor_version;
    uint8_t golden_config_aux_version;

    uint8_t user_config_major_version;
    uint8_t user_config_minor_version;
    uint8_t user_config_aux_version;

    uint8_t pending_config_major_version;
    uint8_t pending_config_minor_version;
    uint8_t pending_config_aux_version;
} __attribute__((packed));

namespace ipmi::inv::configuration
{
    static constexpr uint8_t netFnPlatformConfiguration = 0x38;

    static constexpr uint8_t cmdOemGetServiceConfigInfo = 0xD2;
    static constexpr uint8_t cmdOemRestoreGoldenConfigs = 0xD1;
    static constexpr uint8_t cmdOemActivateUserConfigs = 0xD0;

    static constexpr uint8_t resultSuccess = 0x00;
    static constexpr uint8_t resultRequestedServiceNotAvailable = 0x80;
    static constexpr uint8_t resultConfigurationFileDoesNotExistAtPendingFolder = 0x80;
    static constexpr uint8_t resultInvalidConfigurationIndex = 0x81;
    static constexpr uint8_t resultFailedToRestoreToGolden = 0x82;

    ipmi::RspType<std::vector<uint8_t>> ipmiOemGetServiceConfigInfo(ipmi::Context::ptr ctx, const uint8_t &serviceID, const uint8_t &functionID);
    ipmi::RspType<uint8_t> ipmiOemRestoreGoldenConfigs(ipmi::Context::ptr ctx, const uint8_t &serviceID);
    ipmi::RspType<uint8_t> ipmiOemActivateUserConfigs(ipmi::Context::ptr ctx, const uint8_t &serviceID);

    class ConfigurationParser
    {
    public:
        ConfigurationParser() = delete;
        ~ConfigurationParser() = default;
        ConfigurationParser(const fs::path &jsonPath) : configPath{jsonPath}, json{nullptr}
        {
            if (!fs::exists(jsonPath))
            {
                return;
            }

            std::ifstream jsonStream(jsonPath.string().c_str());
            if (!jsonStream.good())
            {
                return;
            }

            json = json_t::parse(jsonStream, nullptr, false);
            if (json.is_discarded())
            {
                return;
            }
        }

        bool is_valid(void)
        {
            std::cerr << __FUNCTION__ << std::endl;
            if (json == nullptr)
            {
                return false;
            }
            return true;
        }

        bool validate(const fs::path &schemeFile)
        {
            if (json.is_discarded())
            {
                return false;
            }

            if (!fs::exists(schemeFile))
            {
                return false;
            }

            std::ifstream schemaStream(schemeFile.string().c_str());
            if (!schemaStream.good())
            {
                return false;
            }

            json_t schema = json_t::parse(schemaStream, nullptr, false);
            if (schema.is_discarded())
            {
                return false;
            }

            if (!validateJson(schema, json))
            {
                return false;
            }
            return true;
        }

        static bool validate(const fs::path &schemeFile, const json_t &jsonData)
        {
            if (jsonData.is_discarded())
            {
                return false;
            }

            if (!fs::exists(schemeFile))
            {
                return false;
            }

            std::ifstream schemaStream(schemeFile.string().c_str());
            if (!schemaStream.good())
            {
                return false;
            }

            json_t schema = json_t::parse(schemaStream, nullptr, false);
            if (schema.is_discarded())
            {
                return false;
            }

            if (!validateJson(schema, jsonData))
            {
                return false;
            }
            return true;
        }

        static bool validate(const fs::path &schemaFile, const fs::path &jsonFile, json_t &jsonData)
        {
            if (!fs::exists(jsonFile))
            {
                return false;
            }

            std::ifstream jsonStream(jsonFile.string().c_str());
            if (!jsonStream.good())
            {
                return false;
            }

            jsonData = json_t::parse(jsonStream, nullptr, false);
            if (jsonData.is_discarded())
            {
                return false;
            }
            return validate(schemaFile, jsonData);
        }

        static std::string getConfigurationProperty(const json_t &jsonData, const std::string &name)
        {
            if (jsonData.is_discarded())
            {
                return std::string();
            }

            if (jsonData.find(name) != jsonData.end())
            {
                if (!jsonData[name].is_null())
                {
                    return jsonData[name];
                }
            }
            return std::string();
        }

        std::string getConfigurationProperty(const std::string &name)
        {
            return getConfigurationProperty(json, name);
        }

        static std::string getServiceName(const json_t &jsonData)
        {
            return getConfigurationProperty(jsonData, "ServiceName");
        }

        inline std::string getServiceName(void)
        {
            return getConfigurationProperty("ServiceName");
        }

        inline std::string getServiceID(void)
        {
            return getConfigurationProperty("ServiceID");
        }

        static std::string getServiceID(const json_t &jsonData)
        {
            return getConfigurationProperty(jsonData, "ServiceID");
        }

        inline std::string getConfigVersion(void)
        {
            return getConfigurationProperty("ConfigVersion");
        }

        static std::string getConfigVersion(const json_t &jsonData)
        {
            return getConfigurationProperty(jsonData, "ConfigVersion");
        }

        std::string getConfigVersion(const fs::path &schemaPath, const int &serviceID)
        {
            for (const auto &p : fs::directory_iterator(configPath))
            {
                std::string path = p.path().string();
                fs::path jsonFile(path);

                // Ignore files that are not json
                if (jsonFile.extension() != ".json")
                {
                    continue;
                }

                // Validate json with schema to filter out Microsoft compliant configurations
                if (!validate(schemaPath))
                {
                    continue;
                }

                // Verify service id and then return its configuration version
                std::string strID = getServiceID();
                if (!strID.empty())
                {
                    int id = std::stoi(strID);
                    if (id == serviceID)
                    {
                        std::string configVersion = getConfigVersion();
                        return configVersion;
                    }
                }
            }
            return std::string();
        }

        std::string getServiceName(const fs::path &schemaPath, const size_t &nameLength, const int &serviceID)
        {
            if (serviceID == 0)
            {
                return std::string();
            }

            for (const auto &p : fs::directory_iterator(configPath))
            {
                std::string path = p.path().string();
                fs::path jsonFile(path);

                // Ignore files that are not json
                if (jsonFile.extension() != ".json")
                {
                    continue;
                }

                // Validate json with schema to filter out Microsoft compliant configurations
                if (!validate(schemaPath))
                {
                    continue;
                }

                // Verify service id and then return its configuration version
                std::string strID = getServiceID();
                if (!strID.empty())
                {
                    int id = std::stoi(strID);
                    if (id == serviceID)
                    {
                        std::string serviceName = getServiceName();
                        if (nameLength == 0)
                        {
                            return serviceName;
                        }
                        int length = (serviceName.length() > nameLength) ? nameLength : serviceName.length();
                        std::string name = serviceName.substr(0, length);
                        return name;
                    }
                }
            }
            return std::string();
        }

        static int getJsons(const fs::path &configDirPath, const fs::path &schemaFile, std::vector<std::pair<json_t, fs::path>> &jsons)
        {
            int serviceCount = 0;

            for (const auto &p : fs::directory_iterator(configDirPath))
            {
                std::string path = p.path().string();
                fs::path jsonFile(path);

                // Ignore files that are not json
                if (jsonFile.extension() != ".json")
                {
                    continue;
                }

                // Check json
                std::ifstream jsonStream(jsonFile.string().c_str());
                if (!jsonStream.good())
                {
                    continue;
                }

                auto json_data = json_t::parse(jsonStream, nullptr, false);
                if (json_data.is_discarded())
                {
                    continue;
                }

                // Validate json with schema to filter out Microsoft compliant configurations
                if (!validate(schemaFile, json_data))
                {
                    continue;
                }

                serviceCount++;
                jsons.emplace_back(std::make_pair(json_data, p));
            }
            return serviceCount;
        }

        static auto parseServiceVersions(const std::string &version) -> std::tuple<uint8_t, uint8_t, uint8_t>
        {
            std::regex pattern{R"(\d+.\d+.\d+)"};
            std::smatch match;
            if (std::regex_match(version, match, pattern))
            {
                std::vector<std::string> versions = split(version, '.');
                return std::make_tuple(std::stoi(versions[0]), std::stoi(versions[1]), std::stoi(versions[2]));
            }
            return std::tuple<uint8_t, uint8_t, uint8_t>();
        }

    private:
        fs::path configPath;
        json_t json;

        static bool validateJson(const json_t &i_schema, const json_t &i_json)
        {
            valijson::Schema schema;
            valijson::SchemaParser parser;
            valijson::adapters::NlohmannJsonAdapter schemaAdapter(i_schema);
            parser.populateSchema(schemaAdapter, schema);
            valijson::Validator validator;
            valijson::adapters::NlohmannJsonAdapter targetAdapter(i_json);
            return validator.validate(schema, targetAdapter, nullptr);
        }

        static std::vector<std::string> split(const std::string &article, const char &delimiter)
        {
            std::vector<std::string> result;
            std::stringstream ss(article);
            std::string token;

            while (std::getline(ss, token, delimiter))
            {
                result.push_back(token);
            }
            return result;
        }
    };
}