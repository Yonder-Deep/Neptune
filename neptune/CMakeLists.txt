cmake_minimum_required(VERSION 3.25)

project(neptune C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(src)
