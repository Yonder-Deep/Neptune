cmake_minimum_required(VERSION 3.22)
project(neptune_sim)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(ament_cmake REQUIRED)
find_package(gazebo_ros REQUIRED)
find_package(xacro REQUIRED)
find_package(robot_state_publisher REQUIRED)

install(DIRECTORY
  launch
  udrf
  worlds
  DESTINATION share/${PROJECT_NAME}
)

ament_package()