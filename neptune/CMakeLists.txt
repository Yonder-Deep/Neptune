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

# Bootstrap xrepo.cmake, this just downloads the cmake file not xrepo & cmake
# When we include() it below, it automatically ensures installation of xrepo & xmake
if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/xrepo.cmake")
    message(STATUS "Downloading xrepo.cmake wrapper...")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/xmake-io/xrepo-cmake/main/xrepo.cmake"
        "${CMAKE_BINARY_DIR}/cmake/xrepo.cmake"
        TLS_VERIFY ON
    )
endif()

set(ENV{XMAKE_PKG_CACHEDIR} "${CMAKE_BINARY_DIR}/.xrepo/cache")
set(ENV{XMAKE_PKG_INSTALLDIR} "${CMAKE_BINARY_DIR}/.xrepo/packages")
include(${CMAKE_BINARY_DIR}/cmake/xrepo.cmake)

add_subdirectory(src)
