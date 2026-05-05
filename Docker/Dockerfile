FROM ubuntu:latest

RUN apt-get update -y

# install dev dependcies
RUN apt-get install -y cmake build-essential
RUN apt-get install -y ninja-build
RUN apt-get update -y
RUN apt-get install -y git gdb curl clang-tidy

#Get project dependencies 
RUN apt-get install -y gpiod libgpiod-dev liblgpio-dev libserial-dev gpsd libgps-dev
WORKDIR /usr/local/include
#install httplib and serial lib TODO - this is unstable, we should choose a specfic version to fix
RUN curl -LO https://github.com/yhirose/cpp-httplib/raw/refs/tags/latest/httplib.h > httplib.h
RUN curl -LO https://github.com/CLIUtils/CLI11/releases/download/v2.6.2/CLI11.hpp > CLI11.hpp
