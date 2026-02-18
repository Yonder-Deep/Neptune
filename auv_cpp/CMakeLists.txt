cmake_minimum_required(VERSION 3.16)
project(nautilus_auv VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_SIMULATION "Build simulation mode (no hardware)" OFF)
option(ENABLE_REALTIME "Enable real-time scheduling" ON)

# Nice default: on Windows, default to simulation unless user explicitly sets otherwise
if (WIN32 AND NOT DEFINED BUILD_SIMULATION)
  set(BUILD_SIMULATION ON CACHE BOOL "Build simulation mode (no hardware)" FORCE)
endif()

# Only define the *C++ macro* when simulation is enabled
if (BUILD_SIMULATION)
  add_compile_definitions(BUILD_SIMULATION)
endif()

# Only look for pigpio when NOT building simulation
if (NOT BUILD_SIMULATION)
  if (NOT UNIX OR APPLE)
    message(FATAL_ERROR
      "Hardware build requires pigpio and is supported on Raspberry Pi OS/Linux only. "
      "Reconfigure with -DBUILD_SIMULATION=ON."
    )
  endif()

  find_library(PIGPIO_LIB pigpio)
  if (NOT PIGPIO_LIB)
    message(FATAL_ERROR
      "pigpio library not found.\n"
      "On Raspberry Pi OS run: sudo apt install pigpio pigpio-dev\n"
      "Or build on Windows with: -DBUILD_SIMULATION=ON"
    )
  endif()
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)
find_package(Eigen3 REQUIRED CONFIG)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src/core)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
