cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

cmake_policy (SET CMP0054 NEW)

project (inv-ipmi-oem CXX)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(LIB_NAME "invoemcmds")
set (
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
    -Wtype-limits \
    -Wnull-dereference \
    "
)

option(WITH_BIOS_OEM_CMD "Enable Support BIOS OEM Cammand" OFF)

add_definitions (-DBOOST_ERROR_CODE_HEADER_ONLY)
add_definitions (-DBOOST_SYSTEM_NO_DEPRECATED)
add_definitions (-DBOOST_ALL_NO_LIB)
add_definitions (-DBOOST_NO_RTTI)
add_definitions (-DBOOST_NO_TYPEID)
add_definitions (-DBOOST_ASIO_DISABLE_THREADS)
add_definitions (-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)
add_definitions (-Wno-psabi)

if(WITH_BIOS_OEM_CMD)
    add_definitions (-DSUPPORT_BIOS_OEM_CMD)
endif()

if(WITH_AST_BMC)
    add_definitions (-DBOARD_AST)
endif()

if(WITH_NUV_BMC)
    add_definitions (-DBOARD_NUV)
endif()

find_package (PkgConfig REQUIRED)
pkg_check_modules (LOGGING phosphor-logging REQUIRED)
include_directories (SYSTEM ${LOGGING_INCLUDE_DIRS})
link_directories (${LOGGING_LIBRARY_DIRS})

pkg_check_modules (LIBIPMID libipmid REQUIRED)
include_directories (SYSTEM ${LIBIPMID_INCLUDE_DIRS})
link_directories (${LIBIPMID_LIBRARY_DIRS})


include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library (${LIB_NAME}
             SHARED src/oemcommands.cpp src/oemgenpasswd.cpp src/platform_configuration_commands.cpp src/msmediaredir.cpp src/configure_firmware_commands.cpp
	         )

set_target_properties (${LIB_NAME} PROPERTIES VERSION "0.1.0")
set_target_properties (${LIB_NAME} PROPERTIES SOVERSION "0")
target_link_libraries (${LIB_NAME} stdc++fs)
target_link_libraries (${LIB_NAME} ipmid)
target_link_libraries (${LIB_NAME} sdbusplus)
target_link_libraries (${LIB_NAME} phosphor_logging)
target_link_libraries (${LIB_NAME} peci)
target_link_libraries (${LIB_NAME} userlayer)



install (TARGETS ${LIB_NAME} DESTINATION lib/ipmid-providers)
