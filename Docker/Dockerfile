FROM debian:bookworm



# install cmake and ninja  


FROM cyberbotics/webots:latest
RUN apt-get update -y 
RUN  apt-get install -y cmake build-essential ninja-build git xvfb \
    x11vnc \
    python3-opengl \
    websockify \
    fluxbox \
    novnc  \
    python3-pip \
RUN pip3 install opencv-python \
    flask
ENV DISPLAY=:1

