FROM debian:bookworm

RUN apt-get update -y

# install cmake and ninja
RUN apt-get install -y cmake build-essential
RUN apt-get install -y ninja-build

#install spdlog and fmt (spdlog needs fmt)
RUN apt-get install -y libspdlog-dev
RUN apt-get install -y libfmt-dev

# install boost
RUN apt-get install -y libboost-all-dev

# install eigen3
RUN apt-get install -y libeigen3-dev

# install git
RUN apt-get install -y git

# install yaml-cpp (for C++ config loading)
RUN apt-get install -y libyaml-cpp-dev
