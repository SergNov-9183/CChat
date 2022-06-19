FROM ubuntu:20.04
COPY . chat
RUN apt-get update 
ENV DEBIAN_FRONTEND noninteractive
RUN apt install g++
RUN apt-get -y install qt5-default -qq



# RUN aptitude install qt5-default -y
# RUN apt -y install build-essential

# FROM vookimedlo/ubuntu-clang:clang_bionic

# RUN apt-get update -qq
# RUN apt-get -y install qt5-default qt5-image-formats-plugins
# COPY . chat
# WORKDIR /chat

# RUN echo "export QMAKESPEC=/usr/lib/x86_64-linux-gnu/qt5/mkspecs/linux-clang" >> /etc/profile