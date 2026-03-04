cmake_minimum_required(VERSION 3.22)
project(neptune_sim)

find_package(ament_cmake REQUIRED)
find_package(gazebo_ros REQUIRED)
find_package(xacro REQUIRED)
find_package(robot_state_publisher REQUIRED)

install(DIRECTORY
  launch
  urdf
  worlds
  config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()