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
    src/serial.cpp
    src/main.cpp
)

add_custom_target(
    generate-devices-file
    DEPENDS ${PROJECT_SOURCE_DIR}/src/devices.dat
    )

add_dependencies(picmeup generate-devices-file)

install(TARGETS picmeup 
    RUNTIME 
    DESTINATION bin)
