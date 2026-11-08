cmake_minimum_required(VERSION 3.25)
project(Neptune
    VERSION 0.1.0
    HOMEPAGE_URL "https://github.com/Yonder-Deep/Neptune"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Allows clangd language server to navigate

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cache/cpm" CACHE PATH "CPM Source Cache")

message(STATUS "Loading CPM.cmake...")
set(CPM_DOWNLOAD_VERSION 0.43.2)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    message(STATUS "Downloading CPM.cmake to ${CMAKE_BINARY_DIR}/cmake")
    message(STATUS "Downloading https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake"
    )
else ()
    message(STATUS "CPM.cmake already downloaded")
endif()

include("${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

add_subdirectory(src)
