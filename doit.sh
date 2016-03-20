#!/bin/bash

# Normal setup
#   different cores for each port.

name=`uname -n`

# Use 'sudo -E ./setup.sh' to include environment variables

if [ -z ${RTE_SDK} ] ; then
    echo "*** RTE_SDK is not set, did you forget to do 'sudo -E ./setup.sh'"
	export RTE_SDK=/work/home/rkwiles/projects/intel/dpdk
	export RTE_TARGET=x86_64-native-linuxapp-clang
fi
sdk=${RTE_SDK}

if [ -z ${RTE_TARGET} ]; then
    echo "*** RTE_TARGET is not set, did you forget to do 'sudo -E ./setup.sh'"
    target=x86_64-native-linuxapp-gcc
else
    target=${RTE_TARGET}
fi

cmd=./app/app/${target}/pktgen


#rkwiles@rkwiles-desk:~/projects/intel/dpdk$ lspci |grep Ether
#06:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
#06:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
#08:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
#08:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
#09:00.0 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
#09:00.1 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
#83:00.1 Ethernet controller: Intel Corporation DH8900CC Null Device (rev 21)
#87:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
#87:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
#89:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
#89:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)

#enp6s0f0  Link encap:Ethernet  HWaddr 68:05:ca:28:49:a3  
#enp6s0f1  Link encap:Ethernet  HWaddr 68:05:ca:28:49:a2  
#ens9f0    Link encap:Ethernet  HWaddr 68:05:ca:28:49:a0  
#ens9f1    Link encap:Ethernet  HWaddr 68:05:ca:28:49:a1  


dpdk_opts="-l 18-26 -n 3 --proc-type auto --log-level 7 --socket-mem 256,256 --file-prefix pg"
pktgen_opts="-T -P"
port_map="-m [19:20].0 -m [21:22].1 -m [23:24].2 -m [25:26].3"
#port_map="-m [2-4].0 -m [5-7].1"
load_file="-f themes/black-yellow.theme"
#load_file="-f themes/white-black.theme"
black_list="-b 06:00.0 -b 06:00.1 -b 08:00.0 -b 08:00.1 -b 09:00.0 -b 09:00.1 -b 83:00.1"

if [ $name == "supermicro" ]; then
	echo ${cmd} ${dpdk_opts} ${black_list} -- ${pktgen_opts} ${port_map} ${load_file}
	${cmd} ${dpdk_opts} ${black_list} -- ${pktgen_opts} ${port_map} ${load_file}
	stty sane
fi

