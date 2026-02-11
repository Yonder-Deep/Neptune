cmake_minimum_required(VERSION 3.25)

project(neptune C CXX)

add_executable(neptune main.cpp)


#paths to ST driver repos(relative to src/CMakeLists.txt)
set(LSM6DSOX_PID_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lsm6dsox-pid)
set(LIS3MDL_PID_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lis3mdl-pid)


#do like 'if(NOT BUILD_SIMULATION)' for testing this thing
add_library(lsm6dsox_driver STATIC ${LSM6DSOX_PID_DIR}/lsm6dsox_reg.c) 
#compiles lsm6dsox_reg.c into a static library called lsm6dsox_reg.h
target_include_directories(lsm6dsox_driver PUBLIC ${LSM6DSOX_PID_DIR})
#target can link to it with #include "lsm6dsox_reg.h"


#BUILD_SIMULATION option
#this isn't a physics engine its to mimic motor.py to simulate the motor
option(BUILD_SIMULATION "Build in simulation mode" OFF)

#find pigpio library(only if not in simulation mode)
if(NOT BUILD_SIMULATION)
    find_library(PIGPIO_LIB pigpio)
    find_path(PIGPIO_INCLUDE_DIR pigpio.h)

    if(NOT PIGPIO_LIB OR NOT PIGPIO_INCLUDE_DIR)
        message(FATAL_ERROR "pigpio library not found")
    endif()

    set(PIGPIO_LINK_LIBS ${PIGPIO_LIB})
    message(STATUS "Found pigpio library: ${PIGPIO_LIB}")
    message(STATUS "Found pigpio include directory: ${PIGPIO_INCLUDE_DIR}")
else()
    #pigpio library isn't required in simulation mode 
    #not sure how simulations will work yet if we do any
    message(STATUS "Building in simulation mode. Pigpio library not used.")
    set(PIGPIO_LINK_LIBS "")
endif()

#create motor library (just header file)
add_library(motor INTERFACE)
target_include_directories(motor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/drivers)

#add pigpio include directory to motor target
if(NOT BUILD_SIMULATION)
    target_include_directories(motor INTERFACE ${PIGPIO_INCLUDE_DIR})
    target_link_libraries(motor INTERFACE ${PIGPIO_LINK_LIBS})
endif()

#executable for testing motor(separate from library)
add_executable(motor_test drivers/main.cpp)
target_link_libraries(motor_test motor)

#define BUILD SIMULATION
if(BUILD_SIMULATION)
    target_compile_definitions(motor_test PRIVATE BUILD_SIMULATION)
endif()

if(NOT BUILD_SIMULATION)
    target_link_libraries(motor_test ${PIGPIO_LINK_LIBS})
endif()

#to use the motor library elsewhere
#target_link_libraries(your_target motor)

