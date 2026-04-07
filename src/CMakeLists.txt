cmake_minimum_required(VERSION 3.25)

project(neptune C CXX)

add_executable(neptune main.cpp)

#BUILD_SIMULATION option
option(BUILD_SIMULATION "Build in simulation mode" OFF)

#find lgpio library (only needed for non-motor targets when not in simulation)
if(NOT BUILD_SIMULATION)
    find_library(LGPIO_LIB lgpio)
    find_path(LGPIO_INCLUDE_DIR lgpio.h)

    if(LGPIO_LIB AND LGPIO_INCLUDE_DIR)
        set(LGPIO_LINK_LIBS ${LGPIO_LIB})
        message(STATUS "Found lgpio library: ${LGPIO_LIB}")
        message(STATUS "Found lgpio include directory: ${LGPIO_INCLUDE_DIR}")
        target_link_libraries(neptune ${LGPIO_LINK_LIBS})
        target_include_directories(neptune PRIVATE ${LGPIO_INCLUDE_DIR})
    else()
        message(STATUS "lgpio library not found (not needed for motor tests)")
    endif()
else()
    message(STATUS "Building in simulation mode.")
    set(LGPIO_LINK_LIBS "")
endif()

#create motor library (header-only, uses sysfs - no lgpio dependency)
add_library(motor INTERFACE)
target_include_directories(motor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/drivers)

#executable for testing motor (uses sysfs hardware PWM, no lgpio needed)
add_executable(motor_test drivers/main.cpp)
target_link_libraries(motor_test motor)

#executable for pool test
add_executable(pool_test drivers/pool_test.cpp)
target_link_libraries(pool_test motor)

#define BUILD_SIMULATION
if(BUILD_SIMULATION)
    target_compile_definitions(motor_test PRIVATE BUILD_SIMULATION)
    target_compile_definitions(pool_test PRIVATE BUILD_SIMULATION)
endif()

#to use the motor library elsewhere
#target_link_libraries(your_target motor)
