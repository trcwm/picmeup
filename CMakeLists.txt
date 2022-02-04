project(picmeup)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(picmeup 
    src/utils.cpp
    src/hexreader.cpp
    src/pgmfactory.cpp
    src/pic16a.cpp
    src/pic16b.cpp
    src/serial.cpp
    src/main.cpp
)
