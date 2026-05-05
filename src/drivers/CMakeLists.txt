
add_library(motors  motors/motor.cpp motors/sim_motor.cpp motors/thruster.cpp)


find_package(PkgConfig REQUIRED)
pkg_check_modules(GPSD REQUIRED libgps)
add_library(gps_drivers INTERFACE)   
target_include_directories(gps_drivers INTERFACE ${GPSD_INCLUDE_DIRS})

target_include_directories(motors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/motors)
target_include_directories(gps_drivers INTERFACE d ${CMAKE_CURRENT_SOURCE_DIR}/gps)
target_link_libraries(gps_drivers INTERFACE ${GPSD_LIBRARIES})