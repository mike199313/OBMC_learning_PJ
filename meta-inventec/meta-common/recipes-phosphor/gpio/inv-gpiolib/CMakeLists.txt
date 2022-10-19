cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

project (invgpiolib CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set (INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set (INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)
set (INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)

set (LIB_NAME "${PROJECT_NAME}")
set (CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
    -Werror \
    -Wtype-limits \
    -Wnull-dereference \
    -fno-rtti \
    "
)
set (INSTALL_INCLUDE_FILES
    ${PROJECT_SOURCE_DIR}/include/invgpiolib.hpp
)
set (INSTALL_GPIO_CONFIGS
    ${PROJECT_SOURCE_DIR}/Configuration/gpio_defs_inv.json
)
set (GPIO_CONFIG_DIR /etc/default/obmc/gpio/)

find_package (nlohmann_json REQUIRED)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library (${LIB_NAME}
             SHARED src/invgpiolib.cpp
             src/gpioconf.cpp)

set_target_properties (${LIB_NAME} PROPERTIES VERSION "0.1.0")
set_target_properties (${LIB_NAME} PROPERTIES SOVERSION "0")

target_link_libraries (${LIB_NAME} stdc++fs)
target_link_libraries (${LIB_NAME} gpiodcxx)
target_link_libraries (${LIB_NAME} nlohmann_json::nlohmann_json)

install (TARGETS ${LIB_NAME}
  EXPORT "${LIB_NAME}EXPORT"
  RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT stlib
  COMPONENT dev)

install (FILES ${INSTALL_INCLUDE_FILES}
  DESTINATION "${INSTALL_INCLUDE_DIR}/")

install (FILES ${INSTALL_GPIO_CONFIGS}
  DESTINATION "${GPIO_CONFIG_DIR}")

