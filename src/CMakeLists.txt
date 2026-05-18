cmake_minimum_required(VERSION 3.25)

project(neptune C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
find_package(Threads REQUIRED)

#required for pthread_setaffinity_np / CPU_SET (GNU extensions)
add_compile_definitions(_GNU_SOURCE)

add_executable(neptune main.cpp)
add_executable(neptune_sim sim_main.cpp)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#paths to ST driver repos(relative to src/CMakeLists.txt)
set(LSM6DSOX_PID_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/st/lsm6dsox-pid)
set(LIS3MDL_PID_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/st/lis3mdl-pid)

#compiles lsm6dsox_reg.c into a static library called lsm6dsox_driver
add_library(lsm6dsox_driver STATIC ${LSM6DSOX_PID_DIR}/lsm6dsox_reg.c) 
#compiles lis3mdl_reg.c into a static library called lis3mdl_driver
add_library(lis3mdl_driver STATIC ${LIS3MDL_PID_DIR}/lis3mdl_reg.c)

target_include_directories(lsm6dsox_driver PUBLIC ${LSM6DSOX_PID_DIR})
target_include_directories(lis3mdl_driver PUBLIC ${LIS3MDL_PID_DIR})

target_link_libraries(lsm6dsox_driver lis3mdl_driver)

#BUILD_SIMULATION option
#this isn't a physics engine its to mimic motor.py to simulate the motor
option(BUILD_SIMULATION "Build in simulation mode" OFF)

#find lgpio library(only if not in simulation mode)
if(NOT BUILD_SIMULATION)
    find_library(LGPIO_LIB lgpio)
    find_path(LGPIO_INCLUDE_DIR lgpio.h)

    if(NOT LGPIO_LIB OR NOT LGPIO_INCLUDE_DIR)
        message(FATAL_ERROR "lgpio library not found")
    endif()

    set(LGPIO_LINK_LIBS ${LGPIO_LIB})
    message(STATUS "Found lgpio library: ${LGPIO_LIB}")
    message(STATUS "Found lgpio include directory: ${LGPIO_INCLUDE_DIR}")

    #neptune links lgpio and has lgpio include directory if not in simulation
    target_link_libraries(neptune ${LGPIO_LINK_LIBS})
    target_include_directories(neptune PRIVATE ${LGPIO_INCLUDE_DIR})
else()
    #lgpio library isn't required in simulation mode
    #not sure how simulations will work yet if we do any
    message(STATUS "Building in simulation mode. lgpio library not used.")
    set(LGPIO_LINK_LIBS "")
endif()




link_directories(drivers)
add_subdirectory(drivers)
add_subdirectory(types)


target_link_libraries(motors ${LGPIO_LINK_LIBS})

#main target
target_include_directories(motors PRIVATE ${LGPIO_INCLUDE_DIR})

target_link_libraries(neptune_sim motors)
target_link_libraries(neptune motors)

target_link_libraries(neptune_sim gps_drivers)
target_link_libraries(neptune gps_drivers)


# Everything below here is not part of the main target

#Testing
enable_testing()

add_executable(
  thruster_test
  tests/thruster.cc
)

target_link_libraries(thruster_test
  GTest::gtest_main
  motors
)

include(GoogleTest)
gtest_discover_tests(thruster_test)

add_library(thruster STATIC
  drivers/motors/thruster.cpp
  drivers/motors/motor.cpp
)
target_include_directories(thruster PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers
)
if(NOT BUILD_SIMULATION)
  target_link_libraries(thruster PUBLIC ${LGPIO_LINK_LIBS})
  target_include_directories(thruster PUBLIC ${LGPIO_INCLUDE_DIR})
endif()

add_library(sim_motor STATIC
  drivers/motors/sim_motor.cpp
  drivers/motors/motor.cpp
)
target_include_directories(sim_motor PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers
)

add_executable(motor_server
  drivers/server/motor_server.cpp
)

target_include_directories(motor_server PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/comms
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers
)


target_link_libraries(motor_server PRIVATE Threads::Threads)

if(NOT BUILD_SIMULATION)
  target_link_libraries(motor_server PRIVATE thruster)
  target_include_directories(motor_server PRIVATE ${LGPIO_INCLUDE_DIR})
else()
  target_link_libraries(motor_server PRIVATE sim_motor)
  target_compile_definitions(motor_server PRIVATE BUILD_SIMULATION)
endif()


#linter
find_program(PIPX NAMES "pipx")
if(PIPX)
 add_custom_target(
        lint
        COMMAND ${PIPX} run pre-commit run

        COMMENT "Runing precommit checks"
    )
    add_custom_target(
        lint_all
        COMMAND ${PIPX} run pre-commit run --all-files

        COMMENT "Runing precommit checks"
    )
else()
message(WARNING "Failed to find pipx for precommit checks")
endif()
