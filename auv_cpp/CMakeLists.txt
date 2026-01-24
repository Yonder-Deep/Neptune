cmake_minimum_required(VERSION 3.16)
project(nautilus_auv VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_SIMULATION "Build simulation mode (no hardware)" OFF)
option(ENABLE_REALTIME "Enable real-time scheduling" ON)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

#if we make core subdirectory uncomment this
#add_subdirectory(src/core)