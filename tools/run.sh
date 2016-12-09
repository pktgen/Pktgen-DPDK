#!/bin/bash

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

# 04:00.0 Ethernet controller: Intel Corporation Ethernet Controller X710 for 10GbE SFP+ (rev 01)
# 04:00.1 Ethernet controller: Intel Corporation Ethernet Controller X710 for 10GbE SFP+ (rev 01)
# 04:00.2 Ethernet controller: Intel Corporation Ethernet Controller X710 for 10GbE SFP+ (rev 01)
# 04:00.3 Ethernet controller: Intel Corporation Ethernet Controller X710 for 10GbE SFP+ (rev 01)
# 05:00.0 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
# 05:00.1 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
# 81:00.0 Ethernet controller: Intel Corporation Ethernet Controller X710 for 10GbE SFP+ (rev 01)
# 81:00.1 Ethernet controller: Intel Corporation Ethernet Controller X710 for 10GbE SFP+ (rev 01)
# 81:00.2 Ethernet controller: Intel Corporation Ethernet Controller X710 for 10GbE SFP+ (rev 01)
# 81:00.3 Ethernet controller: Intel Corporation Ethernet Controller X710 for 10GbE SFP+ (rev 01)
# 82:00.0 Ethernet controller: Intel Corporation Ethernet Controller XL710 for 40GbE QSFP+ (rev 02)
# 83:00.0 Ethernet controller: Intel Corporation Ethernet Controller XL710 for 40GbE QSFP+ (rev 02)
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

dpdk_opts="-l 1-3,18-19 -n 4 --proc-type auto --log-level 8 --socket-mem 4096,4096 --file-prefix pg"
#dpdk_opts="${dpdk_opts} --vdev=net_tap0 --vdev=net_tap1"
dpdk_opts="${dpdk_opts} --vdev=net_bonding0,mode=4,xmit_policy=l23,slave=0000:04:00.0,slave=0000:04:00.1,slave=0000:04:00.2,slave=0000:04:00.3"
dpdk_opts="${dpdk_opts} --vdev=net_bonding1,mode=4,xmit_policy=l23,slave=0000:81:00.0,slave=0000:81:00.1,slave=0000:81:00.2,slave=0000:81:00.3"

pktgen_opts="-T -P --crc-strip"
pktgen_opts="${pktgen_opts} -m [2:3].0 -m [18:19].1"
#pktgen_opts="${pktgen_opts} -m [2:3].0 -m [4:5].1 -m [6:7].2 -m [8:9].3"
#pktgen_opts="${pktgen_opts} -m [10:11].4 -m [12:13].5 -m [14:15].6 -m [16:17].7"

black_list="-b 05:00.0 -b 05:00.1"
#black_list="${black_list} -b 04:00.0 -b 04:00.1 -b 04:00.2 -b 04:00.3"
#black_list="${black_list} -b 81:00.0 -b 81:00.1 -b 81:00.2 -b 81:00.3"
black_list="${black_list} -b 82:00.0 -b 83:00.0"

load_file="-f themes/black-yellow.theme"

echo ${cmd} ${dpdk_opts} ${black_list} -- ${pktgen_opts} ${load_file}
sudo ${cmd} ${dpdk_opts} ${black_list} -- ${pktgen_opts} ${load_file}

# Restore the screen and keyboard to a sane state
stty sane
