FROM ubuntu:latest

RUN apt-get update -y

# install cmake and ninja
RUN apt-get install -y cmake build-essential
RUN apt-get install -y ninja-build
RUN apt-get install -y git gdb curl

#Get project dependencies 
RUN apt-get install -y gpiod libgpiod-dev liblgpio-dev 
WORKDIR /usr/local/include
RUN curl -LO https://github.com/yhirose/cpp-httplib/raw/refs/tags/latest/httplib.h > httplib.h

