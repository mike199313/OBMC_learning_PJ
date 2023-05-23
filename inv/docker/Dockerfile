FROM ubuntu:20.04

ENV TZ=Asia/Taipei \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
        git build-essential

RUN apt-get install -y \
        libsdl1.2-dev texinfo gawk chrpath diffstat

RUN apt-get install -y \
        python3.8-minimal
RUN apt-get install -y \
        python3.8
RUN apt-get install -y \
        sudo \
        cpio \
        wget \
        locales

RUN apt-get install -y \
        vim \
        liblz4-tool \
        zstd \
        pkg-config

RUN rm /usr/bin/python3
RUN ln -s python3.8 /usr/bin/python3


RUN sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen && \
    locale-gen
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
