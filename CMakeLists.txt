cmake_minimum_required(VERSION 3.20)
project(picmeup CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
