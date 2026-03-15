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
    
WORKDIR /opt/blue_ws/src
RUN git clone -b humble https://github.com/Robotic-Decision-Making-Lab/blue.git

WORKDIR /opt/blue_ws
RUN vcs import src < src/blue/blue.repos || true

RUN apt-get update && \
    rosdep update && \
    rosdep install -y --from-paths src --ignore-src --rosdistro humble --skip-keys "blue" && \
    rm -rf /var/lib/apt/lists/*

# Python dependenices for AUV
# WORKDIR /workspace
# COPY auv/requirements/txt .
# RUN pip3 install -r requirements.txt

RUN apt-get update && apt-get install -y ca-certificates gpg wget && \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
    echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | \
    tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
    apt-get update && \
    apt-get install -y cmake && \
    rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    x11vnc xvfb fluxbox \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    novnc websockify \
    && rm -rf /var/lib/apt/lists/*

RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
    colcon build --symlink-install --continue-on-error" || true

# install cmake and ninja  
# RUN apt-get install -y cmake build-essential
# RUN apt-get install -y ninja-build
# RUN apt-get install -y git
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
RUN echo "source /opt/blue_ws/install/setup.bash" >> ~/.bashrc

WORKDIR /workspaces/Neptune

