#include "platform_configuration_commands.hpp"
#include <regex>

namespace ipmi::inv::configuration
{
    ipmi::RspType<std::vector<uint8_t>> ipmiOemGetServiceConfigInfo(ipmi::Context::ptr ctx, const uint8_t &serviceID, const uint8_t &functionID)
    {
        // Check system paths
        fs::path ipmiHostDirPath(std::string{CONF_IPMI_HOST_DIR});
        fs::path entityManagerDirPath(std::string{CONF_ENTITY_MANAGER_DIR});
        fs::path goldenConfigurationDirPath(std::string{CONF_GOLDEN_DIR});
        fs::path userConfigurationDirPath(std::string{CONF_USER_DIR});
        fs::path pendingConfigurationDirPath(std::string{CONF_PENDING_DIR});
        fs::path schemaFilePath(std::string{CONF_SCHEMA_FILE});
        if (!(fs::exists(ipmiHostDirPath) && fs::exists(entityManagerDirPath) && fs::exists(goldenConfigurationDirPath) &&
              fs::exists(userConfigurationDirPath) && fs::exists(pendingConfigurationDirPath) && fs::exists(schemaFilePath)))
        {
            std::cerr << "Error system configuration paths do not exist." << std::endl;
            phosphor::logging::log<phosphor::logging::level::ERR>("Error system configuration paths do not exist.");
            return ipmi::responseResponseError();
        }

        // Get the total number of the service count
        if (serviceID == 0 && functionID == 0)
        {
            // Get service name and id command
            std::vector<uint8_t> result = {0};

            uint8_t serviceCount = 0;
            std::vector<std::pair<json_t, fs::path>> configs = {};
            serviceCount += ConfigurationParser::getJsons(ipmiHostDirPath, schemaFilePath, configs);
            serviceCount += ConfigurationParser::getJsons(entityManagerDirPath, schemaFilePath, configs);

            // List service name and ID for Microsoft compliant configurations
            // Filling service count field
            result.emplace_back(serviceCount);
            for (const auto &[j, p] : configs)
            {
                // Filling service name field
                std::string serviceName = ConfigurationParser::getServiceName(j);
                if (serviceName.empty())
                {
                    continue;
                }

                int length = (serviceName.length() > 4) ? 4 : serviceName.length();
                result.insert(result.cend(), std::begin(serviceName), std::begin(serviceName) + length);

                // Filling service id field
                std::string id = ConfigurationParser::getServiceID(j);
                if (id.empty())
                {
                    continue;
                }
                result.emplace_back(std::stoi(id));
            }

            // Requested service not available
            if (serviceCount == 0)
            {
                return ipmi::responseSuccess(std::vector<uint8_t>{resultRequestedServiceNotAvailable});
            }

            // Success
            return ipmi::responseSuccess(result);
        }

        // Get active version
        if (serviceID != 0 && functionID == 1)
        {
            ServiceConfigActiveVersionRsp rsp{0};

            uint8_t serviceCount = 0;

            // Get work configuration version
            std::vector<std::pair<json_t, fs::path>> configs = {};
            serviceCount += ConfigurationParser::getJsons(ipmiHostDirPath, schemaFilePath, configs);
            serviceCount += ConfigurationParser::getJsons(entityManagerDirPath, schemaFilePath, configs);

            // Search for service ID
            bool foundID = false;
            for (const auto &[j, p] : configs)
            {
                std::string id = ConfigurationParser::getServiceID(j);
                if (id.empty())
                {
                    continue;
                }

                if (std::stoi(id) != serviceID)
                {
                    continue;
                }
                foundID |= true;

                // Filling service name field
                std::string name = ConfigurationParser::getServiceName(j);
                name.copy(rsp.service_name, sizeof(rsp.service_name), 0);

                // Filling active configuration version field
                std::string version = ConfigurationParser::getConfigVersion(j);
                auto [majorVersion, minorVersion, auxVersion] = ConfigurationParser::parseServiceVersions(version);
                rsp.active_config_major_version = majorVersion;
                rsp.active_config_minor_version = minorVersion;
                rsp.active_config_aux_version = auxVersion;
                break;
            }

            // Requested service not available
            if (serviceCount == 0 || foundID == false)
            {
                return ipmi::responseSuccess(std::vector<uint8_t>{resultRequestedServiceNotAvailable});
            }

            // Filling result
            auto pRsp = reinterpret_cast<uint8_t *>(&rsp);
            std::vector<uint8_t> result(pRsp, pRsp + sizeof(struct ServiceConfigActiveVersionRsp));

            // Success
            return ipmi::responseSuccess(result);
        }

        // Get all versions
        if (serviceID != 0 && functionID == 2)
        {
            ServiceConfigAllVersionsRsp rsp{0};

            uint8_t serviceCount = 0;

            // Get golden configuration version
            std::vector<std::pair<json_t, fs::path>> goldenConfigs = {};
            serviceCount += ConfigurationParser::getJsons(goldenConfigurationDirPath, schemaFilePath, goldenConfigs);

            bool foundID = false;

            // Search for service ID
            for (const auto &[j, p] : goldenConfigs)
            {
                std::string strId = ConfigurationParser::getServiceID(j);
                if (strId.empty())
                {
                    continue;
                }
                uint8_t id = std::stoi(strId);

                if (id != serviceID)
                {
                    continue;
                }
                foundID |= true;

                // Filling service name field
                std::string name = ConfigurationParser::getServiceName(j);
                name.copy(rsp.service_name, sizeof(rsp.service_name), 0);

                // Filling golden configuration version field
                std::string version = ConfigurationParser::getConfigVersion(j);
                auto [majorVersion, minorVersion, auxVersion] = ConfigurationParser::parseServiceVersions(version);
                rsp.golden_config_major_version = majorVersion;
                rsp.golden_config_minor_version = minorVersion;
                rsp.golden_config_aux_version = auxVersion;
                break;
            }

            // Get user configuration version
            std::vector<std::pair<json_t, fs::path>> userConfigs = {};
            serviceCount += ConfigurationParser::getJsons(userConfigurationDirPath, schemaFilePath, userConfigs);

            // Search for service ID
            for (const auto &[j, p] : userConfigs)
            {
                std::string strId = ConfigurationParser::getServiceID(j);
                if (strId.empty())
                {
                    continue;
                }
                uint8_t id = std::stoi(strId);

                if (id != serviceID)
                {
                    continue;
                }
                foundID |= true;

                // Filling service name field
                std::string name = ConfigurationParser::getServiceName(j);
                name.copy(rsp.service_name, sizeof(rsp.service_name), 0);

                // Filling golden configuration version field
                std::string version = ConfigurationParser::getConfigVersion(j);
                auto [majorVersion, minorVersion, auxVersion] = ConfigurationParser::parseServiceVersions(version);
                rsp.user_config_major_version = majorVersion;
                rsp.user_config_minor_version = minorVersion;
                rsp.user_config_aux_version = auxVersion;
                break;
            }

            // Get pending configuration version
            std::vector<std::pair<json_t, fs::path>> pendingConfigs = {};
            serviceCount += ConfigurationParser::getJsons(pendingConfigurationDirPath, schemaFilePath, pendingConfigs);

            // Search for service ID
            for (const auto &[j, p] : pendingConfigs)
            {
                std::string strId = ConfigurationParser::getServiceID(j);
                if (strId.empty())
                {
                    continue;
                }
                uint8_t id = std::stoi(strId);

                if (id != serviceID)
                {
                    continue;
                }
                foundID |= true;

                // Filling service name field
                std::string name = ConfigurationParser::getServiceName(j);
                name.copy(rsp.service_name, sizeof(rsp.service_name), 0);

                // Filling golden configuration version field
                std::string version = ConfigurationParser::getConfigVersion(j);
                auto [majorVersion, minorVersion, auxVersion] = ConfigurationParser::parseServiceVersions(version);
                rsp.pending_config_major_version = majorVersion;
                rsp.pending_config_minor_version = minorVersion;
                rsp.pending_config_aux_version = auxVersion;
                break;
            }

            // Requested service not available
            if (serviceCount == 0 || foundID == false)
            {
                return ipmi::responseSuccess(std::vector<uint8_t>{resultRequestedServiceNotAvailable});
            }

            // Filling result
            auto pRsp = reinterpret_cast<uint8_t *>(&rsp);
            std::vector<uint8_t> result(pRsp, pRsp + sizeof(struct ServiceConfigAllVersionsRsp));

            // Success
            return ipmi::responseSuccess(result);
        }

        // Requested service not available
        return ipmi::responseSuccess(std::vector<uint8_t>{resultRequestedServiceNotAvailable});
    }

    ipmi::RspType<uint8_t> ipmiOemRestoreGoldenConfigs(ipmi::Context::ptr ctx, const uint8_t &serviceID)
    {
        // Check system paths
        fs::path ipmiHostDirPath(std::string{CONF_IPMI_HOST_DIR});
        fs::path entityManagerDirPath(std::string{CONF_ENTITY_MANAGER_DIR});
        fs::path goldenConfigurationDirPath(std::string{CONF_GOLDEN_DIR});
        fs::path userConfigurationDirPath(std::string{CONF_USER_DIR});
        fs::path pendingConfigurationDirPath(std::string{CONF_PENDING_DIR});
        fs::path schemaFilePath(std::string{CONF_SCHEMA_FILE});
        if (!(fs::exists(ipmiHostDirPath) && fs::exists(entityManagerDirPath) && fs::exists(goldenConfigurationDirPath) &&
              fs::exists(userConfigurationDirPath) && fs::exists(pendingConfigurationDirPath) && fs::exists(schemaFilePath)))
        {
            std::cerr << "Error system configuration paths do not exist." << std::endl;
            phosphor::logging::log<phosphor::logging::level::ERR>("Error system configuration paths do not exist.");
            return ipmi::responseResponseError();
        }

        // Lambda for restore golden configs
        auto restoreGoldenConfig = [goldenConfigurationDirPath, schemaFilePath](const fs::path &workJsonFile, const uint8_t &id) -> uint8_t
        {
            std::string filename = workJsonFile.filename();

            // Create symbokic link from work folder to golden folder
            fs::path goldenJsonFile{goldenConfigurationDirPath};
            goldenJsonFile /= filename;
            if (!fs::exists(goldenJsonFile))
            {
                // Failed to restore to golden
                return resultFailedToRestoreToGolden;
            }

            // Validate json schema
            json_t jsonData;
            if (!ConfigurationParser::validate(schemaFilePath, goldenJsonFile, jsonData))
            {
                // Failed to restore to golden
                return resultFailedToRestoreToGolden;
            }

            // Validate configuration id
            std::string serviceID = ConfigurationParser::getServiceID(jsonData);
            if (serviceID.empty())
            {
                // Failed to restore to golden
                return resultFailedToRestoreToGolden;
            }

            if (std::stol(serviceID) != id)
            {
                // Failed to restore to golden
                return resultFailedToRestoreToGolden;
            }

            if (!fs::is_regular_file(goldenJsonFile))
            {
                // Failed to restore to golden
                return resultFailedToRestoreToGolden;
            }

            fs::remove(workJsonFile);
            fs::create_symlink(goldenJsonFile, workJsonFile);

            // Success
            return resultSuccess;
        };

        // Get work jsons
        std::vector<std::pair<json_t, fs::path>> configs = {};
        uint8_t serviceCount = 0;
        serviceCount += ConfigurationParser::getJsons(ipmiHostDirPath, schemaFilePath, configs);
        serviceCount += ConfigurationParser::getJsons(entityManagerDirPath, schemaFilePath, configs);

        // Search for service ID and its file path
        bool foundID = false, foundConfig = false;
        for (const auto &[j, p] : configs)
        {
            // Get id property
            std::string strId = ConfigurationParser::getServiceID(j);
            if (strId.empty())
            {
                continue;
            }
            uint8_t id = std::stoi(strId);

            // Exclude condition
            if (serviceID != 0)
            if (id != serviceID)
            {
                continue;
            }
            foundID |= true;

            // Restore configs
            if (restoreGoldenConfig(p, id) == resultSuccess)
            {
                foundConfig |= true;
            }

            // Exit condition
            if (serviceID != 0)
            if (foundConfig)
            {
                break;
            }
        }

        // Invalid configuration index
        if (!foundID)
        {
            return ipmi::responseSuccess(resultInvalidConfigurationIndex);
        }

        // Failed to restore to golden
        if (!foundConfig)
        {
            return ipmi::responseSuccess(resultFailedToRestoreToGolden);
        }

        // Success
        return ipmi::responseSuccess(resultSuccess);
    }

    ipmi::RspType<uint8_t> ipmiOemActivateUserConfigs(ipmi::Context::ptr ctx, const uint8_t &serviceID)
    {
        // Check system paths
        fs::path ipmiHostDirPath(std::string{CONF_IPMI_HOST_DIR});
        fs::path entityManagerDirPath(std::string{CONF_ENTITY_MANAGER_DIR});
        fs::path goldenConfigurationDirPath(std::string{CONF_GOLDEN_DIR});
        fs::path userConfigurationDirPath(std::string{CONF_USER_DIR});
        fs::path pendingConfigurationDirPath(std::string{CONF_PENDING_DIR});
        fs::path schemaFilePath(std::string{CONF_SCHEMA_FILE});
        if (!(fs::exists(ipmiHostDirPath) && fs::exists(entityManagerDirPath) && fs::exists(goldenConfigurationDirPath) &&
              fs::exists(userConfigurationDirPath) && fs::exists(pendingConfigurationDirPath) && fs::exists(schemaFilePath)))
        {
            std::cerr << "Error system configuration paths do not exist." << std::endl;
            phosphor::logging::log<phosphor::logging::level::ERR>("Error system configuration paths do not exist.");
            return ipmi::responseResponseError();
        }

        // Lambda for activate user configs
        auto activateUserConfig = [pendingConfigurationDirPath, userConfigurationDirPath, goldenConfigurationDirPath, schemaFilePath](const fs::path &workJsonFile, const uint8_t &id) -> uint8_t
        {
            std::string filename = workJsonFile.filename();

            // Check json file in pending folder
            fs::path pendingJsonFile{pendingConfigurationDirPath};
            pendingJsonFile /= filename;
            if (!fs::exists(pendingJsonFile))
            {
                // Configuration file does not exist at pending folder
                return resultConfigurationFileDoesNotExistAtPendingFolder;
            }

            // Validate json schema
            json_t jsonData;
            if (!ConfigurationParser::validate(schemaFilePath, pendingJsonFile, jsonData))
            {
                // Configuration file does not exist at pending folder
                return resultConfigurationFileDoesNotExistAtPendingFolder;
            }
            
            // Validate configuration id
            std::string serviceID = ConfigurationParser::getServiceID(jsonData);
            if (serviceID.empty())
            {
                // Configuration file does not exist at pending folder
                return resultConfigurationFileDoesNotExistAtPendingFolder;
            }

            if (std::stol(serviceID) != id)
            {
                // Configuration file does not exist at pending folder
                return resultConfigurationFileDoesNotExistAtPendingFolder;
            }

            // Move json file from pending folder to user folder
            fs::path userJsonFile{userConfigurationDirPath};
            userJsonFile /= filename;
            fs::copy_file(pendingJsonFile, userJsonFile, fs::copy_options::overwrite_existing);
            fs::remove(pendingJsonFile);

            // Move json file from work folder to golden folder
            if (!is_symlink(workJsonFile))
            {
                fs::path goldenJsonFile{goldenConfigurationDirPath};
                goldenJsonFile /= filename;
                fs::copy_file(workJsonFile, goldenJsonFile, fs::copy_options::overwrite_existing);
            }

            // Create a symbolic link to user directory for work configuration
            fs::remove(workJsonFile);
            fs::create_symlink(userJsonFile, workJsonFile);

            // Success
            return resultSuccess;
        };

        // Get work jsons
        std::vector<std::pair<json_t, fs::path>> configs = {};
        uint8_t serviceCount = 0;
        serviceCount += ConfigurationParser::getJsons(ipmiHostDirPath, schemaFilePath, configs);
        serviceCount += ConfigurationParser::getJsons(entityManagerDirPath, schemaFilePath, configs);

        // Search for service ID and its file path
        bool foundID = false, foundConfig = false;
        for (const auto &[j, p] : configs)
        {
            // Get id property
            std::string strId = ConfigurationParser::getServiceID(j);
            if (strId.empty())
            {
                continue;
            }
            uint8_t id = std::stoi(strId);

            // Exclude condition
            if (serviceID != 0)
            if (id != serviceID)
            {
                continue;
            }
            foundID |= true;

            // Activate configs
            if (activateUserConfig(p, id) == resultSuccess)
            {
                foundConfig |= true;
            }

            // Exit condition
            if (serviceID != 0)
            if (foundConfig)
            {
                break;
            }
        }

        // Invalid configuration index
        if (!foundID)
        {
            return ipmi::responseSuccess(resultInvalidConfigurationIndex);
        }

        // Configuration file does not exist at pending folder
        if (!foundConfig)
        {
            return ipmi::responseSuccess(resultConfigurationFileDoesNotExistAtPendingFolder);
        }

        // Success
        return ipmi::responseSuccess(resultSuccess);
    }
}