cmake_minimum_required(VERSION 3.25)

project(neptune C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

#required for pthread_setaffinity_np / CPU_SET (GNU extensions)
add_compile_definitions(_GNU_SOURCE)

add_executable(neptune main.cpp)

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

#need pthreads for RT thread
find_package(Threads REQUIRED)

#executable for testing motor(separate from library)
add_executable(motor_test drivers/main.cpp)
target_link_libraries(motor_test motor)
target_link_libraries(motor_test Threads::Threads)

#executable for pool test
add_executable(pool_test drivers/pool_test.cpp)
target_link_libraries(pool_test motor)
target_link_libraries(pool_test Threads::Threads)

#executable for rc test (interactive single-motor keyboard control)
add_executable(rc_test drivers/rc_test.cpp)
target_link_libraries(rc_test motor)
target_link_libraries(rc_test Threads::Threads)

#executable for manual_test (4-motor mixer driven from stdin; talks to sim Flask in BUILD_SIMULATION mode)
add_executable(manual_test drivers/manual_test.cpp)
target_link_libraries(manual_test motor)
target_link_libraries(manual_test Threads::Threads)
#cpp-httplib uses Winsock on Windows; harmless on POSIX
if(WIN32)
    target_link_libraries(manual_test ws2_32)
endif()

#define BUILD SIMULATION
if(BUILD_SIMULATION)
    target_compile_definitions(motor_test PRIVATE BUILD_SIMULATION)
    target_compile_definitions(pool_test PRIVATE BUILD_SIMULATION)
    target_compile_definitions(rc_test PRIVATE BUILD_SIMULATION)
    target_compile_definitions(manual_test PRIVATE BUILD_SIMULATION)
endif()

if(NOT BUILD_SIMULATION)
    target_link_libraries(motor_test ${LGPIO_LINK_LIBS})
    target_link_libraries(pool_test ${LGPIO_LINK_LIBS})
    target_link_libraries(rc_test ${LGPIO_LINK_LIBS})
    target_link_libraries(manual_test ${LGPIO_LINK_LIBS})
endif()

#to use the motor library elsewhere
#target_link_libraries(your_target motor)
