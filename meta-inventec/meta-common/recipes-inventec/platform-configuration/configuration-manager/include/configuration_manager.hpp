extern "C"
{
#include <sys/fcntl.h>
#include <sys/mman.h>
}

#include <filesystem>
#include <sdbusplus/server.hpp>

#include "nlohmann/json.hpp"

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

const constexpr char *CONF_UPLOAD_DIR = "/var/wcs/home/configs";
const constexpr char *CONF_GOLDEN_DIR = "/etc/defconfig/configs";
const constexpr char *CONF_USER_DIR = "/etc/conf/configs/user";
const constexpr char *CONF_PENDING_DIR = "/etc/conf/configs/pending";
const constexpr char *CONF_SCHEMA_FILE = "/etc/conf/configs/schema.json";
const constexpr char *PUBLICKEY_FILE = "/etc/activationdata/OpenBMC/publickey";
const constexpr char *HASH_FUNCTION_FILE = "/etc/activationdata/OpenBMC/hashfunc";
const constexpr auto hashFunctionTag = "HashType";

namespace fs = std::filesystem;
using json_t = nlohmann::json;

// RAII support for openSSL functions.
using BIO_MEM_Ptr = std::unique_ptr<BIO, decltype(&::BIO_free)>;
using EVP_PKEY_Ptr = std::unique_ptr<EVP_PKEY, decltype(&::EVP_PKEY_free)>;
using EVP_MD_CTX_Ptr = std::unique_ptr<EVP_MD_CTX, decltype(&::EVP_MD_CTX_free)>;

struct RemovablePath
{
    fs::path path;

    RemovablePath(const fs::path &path) : path(path)
    {
    }

    ~RemovablePath()
    {
        if (!path.empty())
        {
            std::error_code ec;
            fs::remove_all(path, ec);
        }
    }
};

/** @struct CustomFd
 *
 *  RAII wrapper for file descriptor.
 */
struct CustomFd
{
public:
    CustomFd() = delete;
    CustomFd(const CustomFd &) = delete;
    CustomFd &operator=(const CustomFd &) = delete;
    CustomFd(CustomFd &&) = default;
    CustomFd &operator=(CustomFd &&) = default;
    /** @brief Saves File descriptor and uses it to do file operation
     *
     *  @param[in] fd - File descriptor
     */
    CustomFd(int fd) : fd(fd)
    {
    }

    ~CustomFd()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }

    int operator()() const
    {
        return fd;
    }

private:
    /** @brief File descriptor */
    int fd = -1;
};

/** @struct CustomMap
 *
 *  RAII wrapper for mmap.
 */
struct CustomMap
{
private:
    /** @brief starting address of the map   */
    void *addr;

    /** @brief length of the mapping   */
    size_t length;

public:
    CustomMap() = delete;
    CustomMap(const CustomMap &) = delete;
    CustomMap &operator=(const CustomMap &) = delete;
    CustomMap(CustomMap &&) = default;
    CustomMap &operator=(CustomMap &&) = default;

    /** @brief Saves starting address of the map and
     *         and length of the file.
     *  @param[in]  addr - Starting address of the map
     *  @param[in]  length - length of the map
     */
    CustomMap(void *addr, size_t length) : addr(addr), length(length)
    {
    }

    ~CustomMap()
    {
        munmap(addr, length);
    }

    void *operator()() const
    {
        return addr;
    }
};

namespace ipmi::inv::configuration
{
    class Manager
    {
    public:
        Manager(sdbusplus::bus::bus &bus) : bus(bus){};
        int processConfigurations(const std::string &signFilePath);

    private:
        sdbusplus::bus::bus &bus;
        static int unTar(const std::string &tarballFilePath, const std::string &extractDirPath);
        bool findSignPair(const fs::path &dirPath, const std::string &matchString, std::vector<std::pair<fs::path, fs::path>> &foundPaths);
        CustomMap mapFile(const fs::path &path, size_t size);
        inline RSA *createPublicRSA(const fs::path &publicKey);
        std::string getValue(const std::string &manifestFilePath, std::string key);
        bool verifyFile(const fs::path &file, const fs::path &sigFile, const fs::path &publicKey, const fs::path &hashFunc);
        bool validateJson(const json_t &i_schema, const json_t &i_json);
        bool validateJsonWithSchema(const fs::path &jsonPath, const fs::path &schemaPath);
        int virtualFRUWrite(const fs::path &configTempPath);
    };
}
