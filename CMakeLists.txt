cmake_minimum_required(VERSION 3.14)
project(mudong-ev CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CMAKE_BUILD_TESTS "Enable testing of the mudong-json library." OFF)
option(CMAKE_BUILD_EXAMPLES "Enable examples of the mudong-json library." OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CXX_FLAGS
        -fno-omit-frame-pointer # linux perf
        -Wall
        -Wextra
        -Werror
        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
        -std=c++20
        -march=native
        -rdynamic)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

message("PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

message("PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
include_directories(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(src)

message("CMAKE_BUILD_TESTS: ${CMAKE_BUILD_TESTS}")
if(CMAKE_BUILD_TESTS)
    add_subdirectory(test)
endif()

message("CMAKE_BUILD_EXAMPLES: ${CMAKE_BUILD_EXAMPLES}")
if(CMAKE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()