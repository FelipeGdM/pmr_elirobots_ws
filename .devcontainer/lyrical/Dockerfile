FROM ros:lyrical
ARG USERNAME=USERNAME
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Delete user if it exists in container (e.g Ubuntu Noble: ubuntu)
RUN if id -u $USER_UID ; then userdel `id -un $USER_UID` ; fi

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    #
    # [Optional] Add sudo support. Omit if you don't need to install software after connecting.
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

ENV ROS_DISTRO lyrical

RUN echo "source /opt/ros/$ROS_DISTRO/setup.bash" >> /home/$USERNAME/.bashrc

RUN apt-get update && apt-get upgrade -y

RUN apt-get update && apt-get install -y \
    python3-pip \
    clangd \
    libboost-all-dev \
    gdb \
    ros-$ROS_DISTRO-control-msgs \
    ros-$ROS_DISTRO-vision-msgs \
    ros-$ROS_DISTRO-cv-bridge \
    ros-$ROS_DISTRO-xacro \
    ros-$ROS_DISTRO-diagnostic-updater \
    ros-$ROS_DISTRO-image-pipeline \
    ros-$ROS_DISTRO-image-transport \
    ros-$ROS_DISTRO-camera-calibration \
    ros-$ROS_DISTRO-camera-info-manager \
    ros-$ROS_DISTRO-image-transport-plugins \
    ros-$ROS_DISTRO-diagnostic-updater \
    ros-$ROS_DISTRO-ffmpeg-image-transport-msgs \
    ros-$ROS_DISTRO-foxglove-msgs \
    ros-$ROS_DISTRO-rviz-imu-plugin \
    ros-$ROS_DISTRO-depth-image-proc \
    ros-$ROS_DISTRO-vision-msgs \
    ros-$ROS_DISTRO-cv-bridge \
    ros-$ROS_DISTRO-depthai-ros \
    ros-$ROS_DISTRO-foxglove-bridge \
    ros-$ROS_DISTRO-trajectory-msgs 

# RUN python3 -m pip install --break-system-packages torch roboticstoolbox-python

ENV SHELL /bin/bash

# ********************************************************
# * Anything else you want to do like clean up goes here *
# ********************************************************

# [Optional] Set the default user. Omit if you want to keep the default as root.
USER $USERNAME
CMD ["/bin/bash"]