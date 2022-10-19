cmake_minimum_required (VERSION 2.8.10 FATAL_ERROR)
set (BUILD_SHARED_LIBRARIES OFF)
include (ExternalProject)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -lstdc++fs \
    -Werror \
    -Wall \
    -Wextra \
    -Wnon-virtual-dtor \
    -Wold-style-cast \
    -Wcast-align \
    -Wunused \
    -Woverloaded-virtual \
    -Wpedantic \
    -Wmisleading-indentation \
    -Wduplicated-cond \
    -Wduplicated-branches \
    -Wlogical-op \
    -Wnull-dereference \
    -Wuseless-cast \
    -Wdouble-promotion \
    -Wformat=2 \
    -Wno-sign-compare \
    -Wno-reorder \
"
)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

project (configuration_manager CXX)
set(APP_NAME "configuration_manager")

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)
set (CONF_MANAGER_SRC_FILES src/watch.cpp src/configuration_manager.cpp src/configuration_manager_main.cpp)
set (PKG_CONFIG_EXECUTABLE "/usr/bin/pkg-config")

#link_directories (${DBUSINTERFACE_LIBRARIES})

find_package (PkgConfig REQUIRED)

# import systemd
pkg_check_modules (SYSTEMD libsystemd REQUIRED)
#include_directories (${SYSTEMD_INCLUDE_DIRS})
#link_directories (${SYSTEMD_LIBRARY_DIRS})

# import sdbusplus
pkg_check_modules (SDBUSPLUSPLUS sdbusplus REQUIRED)
#include_directories (${SDBUSPLUSPLUS_INCLUDE_DIRS})
#link_directories (${SDBUSPLUSPLUS_LIBRARY_DIRS})
find_program (SDBUSPLUSPLUS sdbus++)

# import phosphor-dbus-interfaces
pkg_check_modules (DBUSINTERFACE phosphor-dbus-interfaces REQUIRED)
#include_directories (${DBUSINTERFACE_INCLUDE_DIRS})
#link_directories (${DBUSINTERFACE_LIBRARIES})

# import phosphor-logging
pkg_check_modules (LOGGING phosphor-logging REQUIRED)
#include_directories (SYSTEM ${LOGGING_INCLUDE_DIRS})
#link_directories (${LOGGING_LIBRARY_DIRS})

# boost
#include_directories (SYSTEM ${CMAKE_BINARY_DIR}/boost-src)
#set (CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/boost-src ${CMAKE_PREFIX_PATH})
#add_definitions (-DBOOST_ERROR_CODE_HEADER_ONLY)
#add_definitions (-DBOOST_SYSTEM_NO_DEPRECATED)
#add_definitions (-DBOOST_ALL_NO_LIB)
#add_definitions (-DBOOST_NO_RTTI)
#add_definitions (-DBOOST_NO_TYPEID)
#add_definitions (-DBOOST_ASIO_DISABLE_THREADS)
#add_definitions (-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)
#add_definitions (-Wno-psabi)

# import nlohmann_json
find_package (nlohmann_json REQUIRED)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package (OpenSSL REQUIRED)

#link_directories (${EXTERNAL_INSTALL_LOCATION}/lib)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable (${APP_NAME} ${CONF_MANAGER_SRC_FILES})
target_link_libraries (${APP_NAME} ${SYSTEMD_LIBRARIES})
#target_link_libraries (${APP_NAME} systemd)
target_link_libraries (${APP_NAME} ${DBUSINTERFACE_LIBRARIES})
target_link_libraries (${APP_NAME} ${SDBUSPLUSPLUS_LIBRARIES})
#target_link_libraries (${APP_NAME} sdbusplus)
target_link_libraries (${APP_NAME} stdc++fs)
target_link_libraries (${APP_NAME} nlohmann_json::nlohmann_json)
#target_link_libraries (${APP_NAME} OpenSSL::Crypto)
target_link_libraries (${APP_NAME} ${OPENSSL_LIBRARIES})
target_link_libraries (${APP_NAME} ${LOGGING_LIBRARIES})
#target_link_libraries (${APP_NAME} phosphor_logging)

install (TARGETS ${APP_NAME} DESTINATION bin)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

print_all_variables()

