FROM ubuntu:latest
LABEL maintainer="jam.sfinae@gmail.com"
ENV BUILD_ON="2019-11-05"

RUN apt install software-properties-common \
    && add-apt-repository ppa:ubuntu-toolchain-r/test \
    && apt-get -yq update && \
    && apt-get -qy upgrade \
    && apt-get -yq install \
        build-essentials \
        gcc-9 \
        g++-9 \
        ninja \
        python3 \
        python3-pip \
    && update-alternatives --install \
        /usr/bin/gcc gcc /usr/bin/gcc-9 90 --slave \
        /usr/bin/g++ g++ /usr/bin/g++-9 --slave \
        /usr/bin/gcov gcov /usr/bin/gcov-9

RUN curl -o /cmake_install.sh \
    https://github.com/Kitware/CMake/releases/download/v3.16.0-rc3/cmake-3.16.0-rc3-Linux-x86_64.sh \
    && chmod +x /cmake_install.sh \
    && /cmake_install.sh --prefix=/usr/local --exclude-subdir

RUN pip3 install conan --user
