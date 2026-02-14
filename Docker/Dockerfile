FROM debian:bookworm



# install cmake and ninja  


FROM python:3.12-slim
RUN apt-get update -y  

RUN apt-get install -y cmake build-essential ninja-build git xvfb \
    x11vnc \
    python3-opengl \
    websockify \
    fluxbox \
    novnc 
RUN pip install pybullet

ENV DISPLAY=:1

