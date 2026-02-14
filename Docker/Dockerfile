FROM osrf/ros:humble-desktop-full

RUN apt-get update && apt-get install -y \
    ros-humble-gazebo-ros-pkgs \
    ros-humble-gazebo-ros-plugins \
    build-essential \
    cmake \
    git \
    ninja-build \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*
    
WORKDIR /opt/uuv_ws/src
RUN git clone https://github.com/uuvsimulator/uuv_simulator.git -b humble-devel
WORKDIR /optuuv_ws
RUN . /opt/ros/humble/setup.bash && colcon build --symlink-install

# Python dependenices for AUV
# WORKDIR /workspace
# COPY auv/requirements/txt .
# RUN pip3 install -r requirements.txt

# install cmake and ninja  
RUN apt-get install -y cmake build-essential
RUN apt-get install -y ninja-build
RUN apt-get install -y git
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
RUN echo "source /opt/uuv_ws/install/setup.bash" >> ~/.bashrc

WORKDIR /workspace/Neptune



