FROM osrf/ros:humble-desktop-full

RUN apt-get update && apt-get install -y \
    ros-humble-gazebo-ros-pkgs \
    ros-humble-gazebo-plugins \
    ros-humble-xacro \
    ros-humble-robot-state-publisher \
    ros-humble-ros2-control \
    ros-humble-ros2-controllers \
    build-essential \
    cmake \
    git \
    ninja-build \
    python3-pip \
    python3-vcstool \
    && rm -rf /var/lib/apt/lists/*
    
WORKDIR /opt/anguler_ws/src
RUN git clone -b humble https://github.com/Robotic-Decision-Making-Lab/angler.git
WORKDIR /opt/angler_ws
RUN vcs import src < src/angler/angler.repos || true

RUN apt-get update && \
    rosdep update && \
    rosdep install -y --from-paths src --ignore-src --rosdistro humble && \
    rm -rf /var/lib/apt/lists/*

# Python dependenices for AUV
# WORKDIR /workspace
# COPY auv/requirements/txt .
# RUN pip3 install -r requirements.txt

RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
    colcon build --symlink-install"

# install cmake and ninja  
# RUN apt-get install -y cmake build-essential
# RUN apt-get install -y ninja-build
# RUN apt-get install -y git
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
RUN echo "source /opt/angler_ws/install/setup.bash" >> ~/.bashrc

