FROM ubuntu:latest

RUN apt-get update -y  

# install cmake and ninja  
RUN apt-get install -y cmake build-essential
RUN apt-get install -y ninja-build
RUN apt-get install -y git 

#Get project dependencies 
RUN apt-get install -y gpiod libgpiod-dev liblgpio-dev


