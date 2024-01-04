cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(SOURCES
    src/main.cpp
    src/pm1006.cpp
    src/utils.cpp
    src/configure_serial.cpp
)

add_executable(MyExecutable ${SOURCES})