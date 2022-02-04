project(picmeup)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(picmeup 
    src/utils.cpp
    src/hexreader.cpp
    src/pic16a.cpp
    src/serial.cpp
    src/main.cpp
)
