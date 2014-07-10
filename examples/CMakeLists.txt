cmake_minimum_required(VERSION 2.8)
project(cfsm_examples CXX)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

set(BASIC_SOURCES basic.cpp)
add_executable("basic" ${BASIC_SOURCES})

set(ATM_SOURCES atm.cpp)
add_executable("atm" ${ATM_SOURCES})

