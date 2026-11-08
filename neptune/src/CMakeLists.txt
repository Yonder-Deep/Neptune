find_package(Threads REQUIRED)

add_executable(neptune main.cpp)
add_executable(neptune_sim sim_main.cpp)

add_subdirectory(common)
target_link_libraries(neptune common)
target_link_libraries(neptune_sim common)

#required for pthread_setaffinity_np / CPU_SET (GNU extensions)
# TODO: Is this required? (it is not mentioned in the motor driver code anymore)
#       If it is required can it be in ./drivers/motors/CMakeLists.txt?
add_compile_definitions(_GNU_SOURCE)

#BUILD_SIMULATION option
option(BUILD_SIMULATION "Build in simulation mode" OFF)
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
    message(STATUS "Building in simulation mode. lgpio library not used.")
    set(LGPIO_LINK_LIBS "")
endif()

# Eigen(linear algebra lib)
find_package(Eigen3 REQUIRED)

target_include_directories(neptune PRIVATE ${EIGEN3_INCLUDE_DIRS})
target_include_directories(neptune_sim PRIVATE ${EIGEN3_INCLUDE_DIRS})

add_subdirectory(drivers)
add_subdirectory(types)
add_subdirectory(states)

target_link_libraries(motors ${LGPIO_LINK_LIBS})

#main target
target_include_directories(motors PRIVATE ${LGPIO_INCLUDE_DIR})

target_link_libraries(neptune_sim motors gps_drivers)
target_link_libraries(neptune motors gps_drivers)

xrepo_package("cpp-httplib 0.63.0")
find_package(httplib)
target_link_libraries(neptune_sim httplib::httplib)

# Everything below here is not part of the main target

#Testing
enable_testing()

xrepo_package("gtest 1.17.0")
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Have to create gtest_main target manually since apparently xrepo can't discover it automatically
add_library(GTest::gtest_main INTERFACE IMPORTED)
target_include_directories(GTest::gtest_main INTERFACE "${gtest_INCLUDE_DIRS}")
target_link_directories(GTest::gtest_main INTERFACE "${gtest_LIBRARY_DIRS}")
target_link_libraries(GTest::gtest_main INTERFACE 
    gtest_main
    ${gtest_LINK_LIBRARIES} 
    ${gtest_SYS_LIBRARIES}
)

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

# TODO: Why is this is separately built & is it needed?
add_library(thruster STATIC
  drivers/motors/thruster.cpp
)
target_include_directories(thruster PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers
)
if(NOT BUILD_SIMULATION)
  target_link_libraries(thruster PUBLIC ${LGPIO_LINK_LIBS})
  target_include_directories(thruster PUBLIC ${LGPIO_INCLUDE_DIR})
endif()

