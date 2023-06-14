FROM ubuntu:20.04

ENV TZ=Asia/Taipei \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update

RUN apt-get install -y \
        gawk wget git diffstat unzip texinfo

RUN apt-get install -y \
        build-essential gcc g++ make

RUN apt-get install -y \
        chrpath socat cpio

RUN apt-get install -y \
        python3 python3-pip python3-pexpect

RUN apt-get install -y \
        python3-git python3-jinja2 python3-distutils

RUN apt-get install -y \
        xz-utils debianutils iputils-ping

RUN apt-get install -y \
        libegl1-mesa libsdl1.2-dev python3-subunit

RUN apt-get install -y \
        mesa-common-dev zstd liblz4-tool file locales

RUN apt-get install -y \
        bzip2 lz4 vim

RUN locale-gen en_US.UTF-8

ENV LANG en_US.UTF-8  
ENV LANGUAGE en_US:en  
ENV LC_ALL en_US.UTF-8   


ARG user
ARG uid
ARG guid

ENV ACCOUNT $user
ENV GUID $guid
ENV UID $uid
RUN groupadd -f -r -g $guid $user
RUN useradd $user -l -u $uid -g $guid -d /home/$user -m -s /bin/bash
# Add user to sudoers
RUN  echo "$ACCOUNT ALL=(ALL) NOPASSWD:ALL" >>/etc/sudoers

COPY .gitconfig /home/$ACCOUNT/.gitconfig




USER ${user}
