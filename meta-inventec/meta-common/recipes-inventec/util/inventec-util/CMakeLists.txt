cmake_minimum_required (VERSION 3.5 FATAL_ERROR)
set (BUILD_SHARED_LIBRARIES OFF)
include (ExternalProject)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
    -Werror \
    -Wtype-limits \
    -Wnull-dereference \
"
)
# todo: get rid of nos, add the below:
#  -Wshadow \
#  -Wconversion \

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set (PKG_CONFIG_EXECUTABLE "/usr/bin/pkg-config")

project (inventec_util CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDBUSPLUSPLUS sdbusplus REQUIRED)
include_directories(${SDBUSPLUSPLUS_INCLUDE_DIRS})
link_directories(${SDBUSPLUSPLUS_LIBRARY_DIRS})

set (MAC_UTIL_SRC_FILES src/mac_util.cpp )

set (MAC_UTIL_LINK_LIBS -lsystemd stdc++fs sdbusplus)

link_directories (${EXTERNAL_INSTALL_LOCATION}/lib)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable (mac_util ${MAC_UTIL_SRC_FILES})

target_link_libraries(mac_util ${MAC_UTIL_LINK_LIBS})

install (TARGETS mac_util DESTINATION bin)

install (PROGRAMS mac_util_config.sh DESTINATION bin)

set (EEPROM_UTIL_SRC_FILES src/eeprom_util.cpp )

set (EEPROM_UTIL_LINK_LIBS -lsystemd stdc++fs sdbusplus)

add_executable (eeprom_util ${EEPROM_UTIL_SRC_FILES})

target_link_libraries(eeprom_util ${EEPROM_UTIL_LINK_LIBS})

install (TARGETS eeprom_util DESTINATION bin)

install (PROGRAMS post_bootup.sh DESTINATION bin)

install (PROGRAMS pre_shutdown.sh DESTINATION bin)
