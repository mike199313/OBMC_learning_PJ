#include <regex>
#include <iostream>
#include <fstream>

#include "configuration_manager.hpp"
#include <phosphor-logging/log.hpp>

namespace ipmi::inv::configuration
{
    int Manager::processConfigurations(const std::string &tarFilePath)
    {
        // Check file exist
        if (!fs::is_regular_file(tarFilePath))
        {
            std::cerr << "Error tarball does not exist." << std::endl;
            phosphor::logging::log<phosphor::logging::level::ERR>("Error tarball does not exist.");
            return -1;
        }

        // Paths to remove
        RemovablePath tarPathToRemove(tarFilePath);
        fs::path tmpDirPath(std::string{CONF_UPLOAD_DIR});
        tmpDirPath /= "tmp";
        auto tmpDir = tmpDirPath.string();
        tmpDirPath = tmpDir;
        RemovablePath tmpDirToRemove(tmpDirPath);

        // Create a tmp dir to extract tarball.
        fs::create_directory(tmpDir);

        // Untar tarball into the tmp dir
        auto rc = unTar(tarFilePath, tmpDirPath.string());
        if (rc < 0)
        {
            std::cerr << "Error occurred during untar." << std::endl;
            phosphor::logging::log<phosphor::logging::level::ERR>("Error occurred during untar.");
            return -1;
        }

        // Virtual FRU write API
        virtualFRUWrite(tmpDirPath);

        return 0;
    }

    int Manager::unTar(const std::string &tarFilePath, const std::string &extractDirPath)
    {
        if (tarFilePath.empty())
        {
            std::cerr << "Error TarFilePath is empty." << std::endl;
            return -1;
        }
        
        if (extractDirPath.empty())
        {
            std::cerr << "Error TarFilePath is empty." << std::endl;
            return -1;
        }

        int status = 0;
        pid_t pid = fork();

        if (pid == 0)
        {
            // child process
            execl("/bin/tar", "tar", "-xf", tarFilePath.c_str(), "-C", extractDirPath.c_str(), (char *)0);
            // execl only returns on fail
            std::cerr << "Failed to execute untar file." << std::endl;
            return -1;
        }
        else if (pid > 0)
        {
            waitpid(pid, &status, 0);
            if (WEXITSTATUS(status))
            {
                std::cerr << "Failed to untar file." << std::endl;
                return -1;
            }
        }
        else
        {
            std::cerr << "fork() failed." << std::endl;
            return -1;
        }

        return 0;
    }

    bool Manager::findSignPair(const fs::path &dirPath, const std::string &matchString, std::vector<std::pair<fs::path, fs::path>> &foundPaths)
    {
        if (!fs::exists(dirPath))
        {
            std::cerr << "Error search path does not exist." << std::endl;
            return false;
        }

        std::regex search(matchString);
        std::smatch match;
        for (const auto &p : fs::directory_iterator(dirPath))
        {
            std::string path = p.path().string();

            if (std::regex_search(path, match, search))
            {
                fs::path signPath(path);
                signPath.replace_extension(".sig");
                if (!fs::exists(signPath))
                {
                    continue;
                }
                foundPaths.emplace_back(std::make_pair(signPath, p.path()));
            }
        }
        return true;
    }

    CustomMap Manager::mapFile(const fs::path &path, size_t size)
    {
        CustomFd fd(open(path.c_str(), O_RDONLY));
        return CustomMap(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd(), 0), size);
    }

    inline RSA* Manager::createPublicRSA(const fs::path &publicKey)
    {
        RSA *rsa = nullptr;
        auto size = fs::file_size(publicKey);

        // Read public key file
        auto data = mapFile(publicKey, size);

        BIO_MEM_Ptr keyBio(BIO_new_mem_buf(data(), -1), &::BIO_free);
        if (keyBio.get() == nullptr)
        {
            std::cerr << "Failed to create new BIO Memory buffer." << std::endl;
            return nullptr;
        }

        rsa = PEM_read_bio_RSA_PUBKEY(keyBio.get(), &rsa, nullptr, nullptr);
        return rsa;
    }

    std::string Manager::getValue(const std::string &manifestFilePath, std::string key)
    {
        key = key + "=";
        auto keySize = key.length();

        if (manifestFilePath.empty())
        {
            std::cerr << "Error MANIFESTFilePath is empty" << std::endl;
        }

        std::string value{};
        std::ifstream efile;
        std::string line;
        efile.exceptions(std::ifstream::failbit | std::ifstream::badbit |
                         std::ifstream::eofbit);

        // Too many GCC bugs (53984, 66145) to do this the right way...
        try
        {
            efile.open(manifestFilePath);
            while (getline(efile, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    // If the manifest has CRLF line terminators, e.g. is created on
                    // Windows, the line will contain \r at the end, remove it.
                    line.pop_back();
                }
                if (line.compare(0, keySize, key) == 0)
                {
                    value = line.substr(keySize);
                    break;
                }
            }
            efile.close();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in reading MANIFEST file" << std::endl;
        }

        return value;
    }

    bool Manager::verifyFile(const fs::path &file, const fs::path &sigFile, const fs::path &publicKey, const fs::path &hashFunc)
    {
        // Check existence of the files in the system.
        if (!(fs::exists(file) && fs::exists(sigFile)))
        {
            std::cerr << "Failed to find the Data or signature file." << std::endl;
            return false;
        }

        if (!(fs::exists(publicKey) && fs::exists(hashFunc)))
        {
            std::cerr << "Failed to find the public key file or hash function file." << std::endl;
            return false;
        }

        // Create RSA.
        auto publicRSA = createPublicRSA(publicKey);
        if (publicRSA == nullptr)
        {
            std::cerr << "Failed to create RSA." << std::endl;
            return false;
        }

        // Assign key to RSA.
        EVP_PKEY_Ptr pKeyPtr(EVP_PKEY_new(), ::EVP_PKEY_free);
        EVP_PKEY_assign_RSA(pKeyPtr.get(), publicRSA);

        // Initializes a digest context.
        EVP_MD_CTX_Ptr rsaVerifyCtx(EVP_MD_CTX_new(), ::EVP_MD_CTX_free);

        // Adds all digest algorithms to the internal table
        OpenSSL_add_all_digests();

        // Create Hash structure.
        auto hashFuncName = getValue(hashFunc, hashFunctionTag);
        auto hashStruct = EVP_get_digestbyname(hashFuncName.c_str());
        if (!hashStruct)
        {
            std::cerr << "EVP_get_digestbynam: Unknown message digest." << std::endl;
            return false;
        }

        auto result = EVP_DigestVerifyInit(rsaVerifyCtx.get(), nullptr, hashStruct, nullptr, pKeyPtr.get());
        if (result <= 0)
        {
            std::cerr << "Error occurred during EVP_DigestVerifyInit." << std::endl;
            return false;
        }

        // Hash the data file and update the verification context
        auto size = fs::file_size(file);
        auto dataPtr = mapFile(file, size);

        result = EVP_DigestVerifyUpdate(rsaVerifyCtx.get(), dataPtr(), size);
        if (result <= 0)
        {
            std::cerr << "Error occurred during EVP_DigestVerifyUpdate." << std::endl;
            return false;
        }

        // Verify the data with signature.
        size = fs::file_size(sigFile);
        auto signature = mapFile(sigFile, size);

        result = EVP_DigestVerifyFinal(rsaVerifyCtx.get(), reinterpret_cast<unsigned char *>(signature()), size);

        // Check the verification result.
        if (result < 0)
        {
            std::cerr << "Error occurred during EVP_DigestVerifyFinal." << std::endl;
            return false;
        }

        if (result == 0)
        {
            std::cerr << "EVP_DigestVerifyFinal:Signature validation failed." << std::endl;
            return false;
        }
        return true;
    }

    bool Manager::validateJson(const json_t &i_schema, const json_t &i_json)
    {
        valijson::Schema schema;
        valijson::SchemaParser parser;
        valijson::adapters::NlohmannJsonAdapter schemaAdapter(i_schema);
        parser.populateSchema(schemaAdapter, schema);
        valijson::Validator validator;
        valijson::adapters::NlohmannJsonAdapter targetAdapter(i_json);
        return validator.validate(schema, targetAdapter, nullptr);
    }

    bool Manager::validateJsonWithSchema(const fs::path &jsonPath, const fs::path &schemaPath)
    {
        if (!(fs::exists(jsonPath) && fs::exists(schemaPath)))
        {
            return false;
        }

        std::ifstream schemaStream(schemaPath.c_str());
        if (!schemaStream.good())
        {
            std::cerr << "Cannot open schema file,  cannot validate JSON." << std::endl;
            return false;
        }

        json_t schema = json_t::parse(schemaStream, nullptr, false);
        if (schema.is_discarded())
        {
            std::cerr << "Illegal schema file detected, cannot validate JSON." << std::endl;
            return false;
        }

        std::ifstream jsonStream(jsonPath.c_str());
        if (!jsonStream.good())
        {
            std::cerr << "unable to open JSON." << jsonPath.string() << std::endl;
            return false;
        }

        auto data = json_t::parse(jsonStream, nullptr, false);
        if (data.is_discarded())
        {
            std::cerr << "syntax error in " << jsonPath.string() << "." << std::endl;
            return false;
        }

        if (!validateJson(schema, data))
        {
            std::cerr << "Error validating " << jsonPath.string() << "." << std::endl;
            return false;
        }
        return true;
    }

    int Manager::virtualFRUWrite(const fs::path &configTempPath)
    {
        int count = 0;

        // Search signed files
        std::vector<std::pair<fs::path, fs::path>> signPairPaths = {};
        if (!findSignPair(configTempPath, R"(.*\.json)", signPairPaths))
        {
            return 0;
        }

        for (const auto &[sigFile, file] : signPairPaths)
        {
            // Authenticate configuration
            fs::path jsonFile(file);
            fs::path publicKeyFile(PUBLICKEY_FILE);
            fs::path hashFunctionFile(HASH_FUNCTION_FILE);
            
            if (!verifyFile(jsonFile, sigFile, publicKeyFile, hashFunctionFile))
            {
                continue;
            }

            // Validate json with schema
            fs::path schemaFile(CONF_SCHEMA_FILE);
            if (!validateJsonWithSchema(jsonFile, schemaFile))
            {
               continue;
            }

            // Move to the pending directory
            fs::copy_file(jsonFile, CONF_PENDING_DIR / jsonFile.filename(), fs::copy_options::overwrite_existing);
            count++;
        }
        return count;
    }
}
