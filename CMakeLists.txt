cmake_minimum_required(VERSION 3.0)
project(hd6309sim)

find_package (Threads)

set (SRC
    ${PROJECT_SOURCE_DIR}/contrib/usim/mc6809.cc
    ${PROJECT_SOURCE_DIR}/contrib/usim/mc6809in.cc
    ${PROJECT_SOURCE_DIR}/contrib/usim/usim.cc
    ${PROJECT_SOURCE_DIR}/contrib/usim/misc.cc
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/uart.cpp
    ${PROJECT_SOURCE_DIR}/src/diskio.cpp
    ${PROJECT_SOURCE_DIR}/src/machine.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/contrib/usim
    ${PROJECT_SOURCE_DIR}/contrib/cxxopts/include
)

add_executable(machdep ${PROJECT_SOURCE_DIR}/contrib/usim/machdep.c)
add_executable(hd6309sim ${SRC})
target_link_libraries (hd6309sim ${CMAKE_THREAD_LIBS_INIT})
