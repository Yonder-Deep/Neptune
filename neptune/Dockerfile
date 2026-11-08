FROM ubuntu:latest

# Install dev dependencies
RUN apt-get update -y && apt-get install -y \
        cmake build-essential ninja-build \
        git gdb curl \
        clangd clang-format clang-tidy \
        python3 python3-pip

# Get project dependencies
RUN apt-get install -y gpiod libgpiod-dev liblgpio-dev libserial-dev gpsd libgps-dev libeigen3-dev

WORKDIR /usr/local/include

# install httplib and serial lib TODO - this is unstable, we should choose a specfic version to fix
RUN curl -LO https://github.com/yhirose/cpp-httplib/raw/refs/tags/latest/httplib.h > httplib.h

WORKDIR /workspaces/Neptune
