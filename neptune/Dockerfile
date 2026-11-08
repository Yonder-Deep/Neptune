FROM ubuntu:latest

# Set up the `yonder` user with as a sudoer
# (Xrepo/xmake will not work without a non-root user)
ARG USERNAME=yonder
ARG USER_UID=1001
ARG USER_GID=$USER_UID

RUN apt-get update && apt-get install -y sudo \
    && groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && mkdir -p /etc/sudoers.d \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Install dev dependencies
RUN apt-get update -y && apt-get install -y \
        cmake build-essential ninja-build \
        git gdb curl \
        clangd clang-format clang-tidy \
        python3 python3-pip

# Get project dependencies
RUN apt-get install -y gpiod libgpiod-dev liblgpio-dev libserial-dev gpsd libgps-dev libeigen3-dev

WORKDIR /workspaces/Neptune
USER $USERNAME

