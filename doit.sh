#!/bin/bash

# Normal setup
#   different cores for each port.

name=`uname -n`

# Use 'sudo -E ./setup.sh' to include environment variables

if [ -z ${RTE_SDK} ] ; then echo "*** RTE_SDK is not set, did you forget to do 'sudo -E ./setup.sh'" export RTE_SDK=/work/home/rkwiles/projects/intel/dpdk
	export RTE_TARGET=x86_64-native-linuxapp-clang
fi
sdk=${RTE_SDK}

if [ -z ${RTE_TARGET} ]; then
    echo "*** RTE_TARGET is not set, did you forget to do 'sudo -E ./setup.sh'"
    target=x86_64-native-linuxapp-gcc
else
    target=${RTE_TARGET}
fi

cmd=./app/app/${target}/app/pktgen

#rkwiles@rkwiles-desk:~/projects/intel/dpdk$ lspci |grep Ether
# 06:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
# 06:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
# 08:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
# 08:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
# 09:00.0 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
# 09:00.1 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
# 83:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
# 83:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
# 85:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
# 85:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
# 

#============================================================
#Core and Socket Information (as reported by '/proc/cpuinfo')
#============================================================
#
#cores =  [0, 1, 2, 3, 4, 8, 9, 10, 11, 16, 17, 18, 19, 20, 24, 25, 26, 27]
#sockets =  [0, 1]
#
#        Socket 0        Socket 1        
#        --------        --------        
#Core 0  [0, 36]         [18, 54]        
#Core 1  [1, 37]         [19, 55]        
#Core 2  [2, 38]         [20, 56]        
#Core 3  [3, 39]         [21, 57]        
#Core 4  [4, 40]         [22, 58]        
#Core 8  [5, 41]         [23, 59]        
#Core 9  [6, 42]         [24, 60]        
#Core 10 [7, 43]         [25, 61]        
#Core 11 [8, 44]         [26, 62]        
#Core 16 [9, 45]         [27, 63]        
#Core 17 [10, 46]        [28, 64]        
#Core 18 [11, 47]        [29, 65]        
#Core 19 [12, 48]        [30, 66]        
#Core 20 [13, 49]        [31, 67]        
#Core 24 [14, 50]        [32, 68]        
#Core 25 [15, 51]        [33, 69]        
#Core 26 [16, 52]        [34, 70]        
#Core 27 [17, 53]        [35, 71]        
#

if [ $name == "supermicro" ]; then
	dpdk_opts="-l 8-16 -n 4 --proc-type auto --log-level 0 --socket-mem 256,256 --file-prefix pg"
	pktgen_opts="-T -P"
	port_map="-m [9:10].0 -m [11:12].1 -m [13:14].2 -m [15:16].3"
	#port_map="-m [9-12:13-16].0"
	bl_common="-b 09:00.0 -b 09:00.1"
	black_list="${bl_common} -b 08:00.0 -b 08:00.1 -b 85:00.0 -b 85:00.1"
	load_file="-f themes/black-yellow.theme"

	echo ${cmd} ${dpdk_opts} ${black_list} -- ${pktgen_opts} ${port_map} ${load_file}
	sudo ${cmd} ${dpdk_opts} ${black_list} -- ${pktgen_opts} ${port_map} ${load_file}

	# Restore the screen and keyboard to a sane state
	stty sane
fi

if [ $name == "rkwiles-VirtualBox" ]; then
	dpdk_opts="-l 1-3 -n 4 --proc-type auto --log-level 7 --socket-mem 256 --file-prefix pg"
	dpdk_opts=${dpdk_opts}" --vdev=eth_ring0 --vdev=eth_ring1"
	pktgen_opts="-T -P"
	port_map="-m 2.0 -m 3.1"
	bl_common="-b 00:03.0"
	black_list="${bl_common}"
	load_file="-f themes/black-yellow.theme"

	echo ${cmd} ${dpdk_opts} ${black_list} -- ${pktgen_opts} ${port_map} ${load_file}
	sudo ${cmd} ${dpdk_opts} ${black_list} -- ${pktgen_opts} ${port_map} ${load_file}

	# Restore the screen and keyboard to a sane state
	stty sane
fi
