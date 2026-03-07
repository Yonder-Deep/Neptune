import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    ExecuteProcess,
    IncludeLaunchDescription,
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue



def generate_launch_description():
    pkg_neptune_sim = get_package_share_directory("neptune_sim")
    pkg_gazebo_ros = get_package_share_directory("gazebo_ros")

    # ---------- Arguments ----------
    use_sim_time = LaunchConfiguration("use_sim_time", default="true")
    world_file = LaunchConfiguration("world")
    x_spawn = LaunchConfiguration("x", default="0.0")
    y_spawn = LaunchConfiguration("y", default="0.0")
    z_spawn = LaunchConfiguration("z", default="-5.0")  # start 5 m underwater

    xacro_file = os.path.join(pkg_neptune_sim, "udrf", "neptune.udrf.xacro")

    # ---------- Robot description (xacro → URDF string) ----------
    robot_description = ParameterValue(Command(["xacro ", xacro_file]), value_type=str)

    # ---------- Nodes ----------
    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="screen",
        parameters=[
            {"robot_description": robot_description, "use_sim_time": use_sim_time}
        ],
    )

    # Launch Gazebo server
    gzserver = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_gazebo_ros, "launch", "gzserver.launch.py")
        ),
        launch_arguments={"world": world_file, "verbose": "true"}.items(),
    )

    # Launch Gazebo client (GUI)
    gzclient = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_gazebo_ros, "launch", "gzclient.launch.py")
        ),
    )

    # Spawn the robot into Gazebo
    spawn_entity = Node(
        package="gazebo_ros",
        executable="spawn_entity.py",
        arguments=[
            "-topic", "robot_description",
            "-entity", "neptune",
            "-x", x_spawn,
            "-y", y_spawn,
            "-z", z_spawn,
        ],
        output="screen",
    )

    return LaunchDescription(
        [
            DeclareLaunchArgument("use_sim_time", default_value="true"),
            DeclareLaunchArgument("world",default_value=os.path.join(pkg_neptune_sim, "worlds", "ocean.world")),
            DeclareLaunchArgument("x", default_value="0.0"),
            DeclareLaunchArgument("y", default_value="0.0"),
            DeclareLaunchArgument("z", default_value="-5.0"),
            robot_state_publisher,
            gzserver,
            gzclient,
            spawn_entity,
        ]
    )
