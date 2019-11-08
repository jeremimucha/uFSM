FROM ubuntu:latest
LABEL maintainer="jam.sfinae@gmail.com"
ENV BUILD_ON="2019-11-05"

RUN apt-get -yq update && apt-get -yq install software-properties-common \
    && add-apt-repository ppa:ubuntu-toolchain-r/test \
    && apt-get -yq update \
    && apt-get -yq upgrade \
    && apt-get -yq install \
        build-essential \
        gcc-9 \
        g++-9 \
        ninja-build \
        python3 \
        python3-pip \
        curl \
        git \
    && update-alternatives --install \
        /usr/bin/gcc gcc /usr/bin/gcc-9 90 --slave \
        /usr/bin/g++ g++ /usr/bin/g++-9 --slave \
        /usr/bin/gcov gcov /usr/bin/gcov-9

RUN pip3 install conan cmake

COPY build.sh /build.sh
RUN chmod +x /build.sh

ENTRYPOINT [ "/build.sh" ]
