# SPDX-License-Identifier: GPL-3.0-only
# Copyright N.A. Moseley 2022

cmake_minimum_required(VERSION 3.20)
project(picmeup LANGUAGES CXX ASM VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
    set(PICMEUP_INSTALL_PREFIX /opt/picmeup-${CMAKE_PROJECT_VERSION})
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include(Packing)
endif(UNIX)

add_executable(picmeup 
    src/devices.S
    src/utils.cpp
    src/hexreader.cpp
    src/pgmfactory.cpp
    src/pic16a.cpp
    src/pic16b.cpp
    src/pic16pgm_a.cpp
    src/serial.cpp
    src/main.cpp
)

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/src/devices.S
    COMMAND touch ${PROJECT_SOURCE_DIR}/src/devices.S
    DEPENDS ${PROJECT_SOURCE_DIR}/src/devices.dat
)

install(TARGETS picmeup 
    RUNTIME 
    DESTINATION bin)
