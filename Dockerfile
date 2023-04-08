FROM ubuntu:22.04 as rootfs

## FOR DEBUGGING UTILS
RUN apt update \
 && apt install -y iputils-ping traceroute sudo \
    vim git tmux silversearcher-ag bash-completion \
    netcat-openbsd telnet iperf tcpdump openvswitch-switch \
    bison flex iproute2 mtr curl pppoe pppoeconf lldpd tcpdump \
 && echo "" > ~/.bashrc \
 && echo "if [ -f /etc/bash_completion ] && ! shopt -oq posix; then" >> ~/.bashrc \
 && echo "  . /etc/bash_completion" >> ~/.bashrc \
 && echo "fi" >> ~/.bashrc

## FOR MLX/DPDK
RUN apt -y install libpcap-dev build-essential wget unzip python3-pip \
    libnuma-dev pciutils libibverbs-dev \
    autotools-dev autoconf chrpath swig libltdl-dev libnl-route-3-200 \
    quilt ethtool libnl-route-3-dev dkms debhelper libnl-3-200 graphviz \
    udev dpatch libnl-3-dev automake lsof \
 && pip3 install pyelftools meson \
 && wget https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip \
 && unzip ninja-linux.zip -d /usr/local/bin/ \
 && update-alternatives --install /usr/bin/ninja ninja /usr/local/bin/ninja 1 --force

## OFED INSTALL
## https://network.nvidia.com/products/ethernet-drivers/linux/mlnx_en/
ADD https://www.mellanox.com/downloads/ofed/MLNX_EN-5.8-2.0.3.0/mlnx-en-5.8-2.0.3.0-ubuntu22.04-x86_64.tgz /
RUN tar xpf mlnx-en-5.8-2.0.3.0-ubuntu22.04-x86_64.tgz \
 && cd mlnx-en-5.8-2.0.3.0-ubuntu22.04-x86_64 \
 && ./install --dpdk --force

## DPDK INSTALL
RUN git clone git://dpdk.org/dpdk /dpdk \
 && cd /dpdk \
 && meson setup build --prefix=/usr \
 && ninja -C build \
 && ninja -C build install

## PKTGEN INSTALL
ADD . /pktgen-dpdk
RUN cd /pktgen-dpdk \
 && meson setup build --prefix=/usr \
 && ninja -C build \
 && ninja -C build install
