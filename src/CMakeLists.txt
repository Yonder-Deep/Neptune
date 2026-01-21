cmake_minimum_required(VERSION 3.25)

project(neptune CXX)


add_executable(neptune main.cpp)

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
    message(STATUS "Building in simulation mode. Pigpio library not required.")
    set(PIGPIO_LINK_LIBS "")
endif()

#create motor library (just header file)
add_library(motor INTERFACE)
target_include_directories(motor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/api)

#add pigpio include directory to motor target
if(NOT BUILD_SIMULATION)
    target_include_directories(motor INTERFACE ${PIGPIO_INCLUDE_DIR})
    target_link_libraries(motor INTERFACE ${PIGPIO_LINK_LIBS})
endif()

#executable for testing motor(separate from library)
add_executable(motor_test src/drivers/main.cpp)
target_link_libraries(motor_test motor)
if(NOT BUILD_SIMULATION)
    target_link_libraries(motor_test ${PIGPIO_LINK_LIBS})
endif()

if(BUILD_TESTS)
    enable_testing()
    #add_subdirectory(tests)
endif()

#to use the motor library elsewhere
#target_link_libraries(your_target motor)
