cmake_minimum_required (VERSION 2.8.10 FATAL_ERROR)
set (BUILD_SHARED_LIBRARIES OFF)
include (ExternalProject)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
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

project (ms-subsystem-check CXX C)

set (MSAPP_SRC_FILES src/ms-subsystem-check.cpp src/Utils.cpp src/i2cbusses.c)

set (MSAPP_LINK_LIBS systemd stdc++fs sdbusplus i2c ipmid phosphor_dbus boost_coroutine boost_context)

link_directories (${EXTERNAL_INSTALL_LOCATION}/lib )

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable (ms-subsystem-check ${MSAPP_SRC_FILES})

target_link_libraries(ms-subsystem-check ${MSAPP_LINK_LIBS})

install (TARGETS ms-subsystem-check DESTINATION bin)

