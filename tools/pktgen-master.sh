#!/bin/bash
# SPDX-License-Identifier: BSD-3-Clause
#

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

dpdk_opts="-l 8-16 -n 4 --proc-type auto --log-level 0 --socket-mem 512,512 --file-prefix pg1"
pktgen_opts="-T -P"
port_map="-m [9:10].0 -m [11:12].1 -m [13:14].2 -m [15:16].3"
bl_common="-b 05:00.0 -b 05:00.1 -b 82:00.0 -b 83:00.0"
black_list="${bl_common} -b 81:00.0 -b 81:00.1 -b 81:00.2 -b 81:00.3"
load_file="-f themes/black-yellow.theme"

if [ $name == "rkwiles-DESK1.intel.com" ]; then
        echo ${cmd} ${dpdk_opts} ${black_list} -- ${pktgen_opts} ${port_map} ${load_file}
        sudo ${cmd} ${dpdk_opts} ${black_list} -- ${pktgen_opts} ${port_map} ${load_file}

        # Restore the screen and keyboard to a sane state
        stty sane
fi

