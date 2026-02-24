cmake_minimum_required(VERSION 3.25)

project(neptune C CXX)

#paths to ST driver repos(relative to src/CMakeLists.txt)
set(LSM6DSOX_PID_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lsm6dsox-pid)
set(LIS3MDL_PID_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lis3mdl-pid)

#do like 'if(NOT BUILD_SIMULATION)' for testing this thing
#compiles lsm6dsox_reg.c into a static library called lsm6dsox_driver
add_library(lsm6dsox_driver STATIC ${LSM6DSOX_PID_DIR}/lsm6dsox_reg.c) 
#compiles lis3mdl_reg.c into a static library called lis3mdl_driver
add_library(lis3mdl_driver STATIC ${LIS3MDL_PID_DIR}/lis3mdl_reg.c)

target_include_directories(lsm6dsox_driver PUBLIC ${LSM6DSOX_PID_DIR})
target_include_directories(lis3mdl_driver PUBLIC ${LIS3MDL_PID_DIR})

add_executable(neptune main.cpp drivers/platform_i2c.cpp)

target_link_libraries(neptune lsm6dsox_driver lis3mdl_driver)
target_include_directories(neptune PRIVATE ${LSM6DSOX_PID_DIR} ${LIS3MDL_PID_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/drivers)

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

#create motor library (just header file)
add_library(motor INTERFACE)
target_include_directories(motor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/drivers)

#add lgpio include directory to motor target
if(NOT BUILD_SIMULATION)
    target_include_directories(motor INTERFACE ${LGPIO_INCLUDE_DIR})
    target_link_libraries(motor INTERFACE ${LGPIO_LINK_LIBS})
endif()

#executable for testing motor(separate from library)
add_executable(motor_test drivers/main.cpp)
target_link_libraries(motor_test motor)

#define BUILD SIMULATION
if(BUILD_SIMULATION)
    target_compile_definitions(motor_test PRIVATE BUILD_SIMULATION)
endif()

if(NOT BUILD_SIMULATION)
    target_link_libraries(motor_test ${LGPIO_LINK_LIBS})
endif()

#motor test includes imu.hpp which needs the ST driver headers and libs
target_include_directories(motor_test PRIVATE ${LSM6DSOX_PID_DIR} ${LIS3MDL_PID_DIR})
target_link_libraries(motor_test lsm6dsox_driver lis3mdl_driver)


#executable for test.cpp
#add_executable(test drivers/test.cpp)
#target_link_libraries(test ${LGPIO_LINK_LIBS})
#target_include_directories(test PRIVATE ${LGPIO_INCLUDE_DIR})





#to use the motor library elsewhere
#target_link_libraries(your_target motor)

