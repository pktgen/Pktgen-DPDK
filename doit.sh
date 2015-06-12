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

if [ $name == "rkwiles-supermicro" ]; then
#./app/app/${target}/pktgen -c 1fff0 -n 3 --proc-type auto --log-level=0 --socket-mem 512,512 --file-prefix pg -b 06:00.0 -b 06:00.1 -b 08:00.0 -b 08:00.1 -b 09:00.0 -b 09:00.1 -b 83:00.1 -- -T -P -m "[5:7].0, [6:8].1, [9:11].2, [10:12].3" -f themes/black-yellow.theme
./app/app/${target}/pktgen -c 1fff0 -n 3 --proc-type auto --log-level=0 --socket-mem 512,512 --file-prefix pg -b 06:00.0 -b 06:00.1 -b 08:00.0 -b 08:00.1 -b 09:00.0 -b 09:00.1 -b 83:00.1 -- -T -P -m "[5:7].0" -f themes/black-yellow.theme
fi

#00:19.0 Ethernet controller: Intel Corporation Ethernet Connection (2) I218-V
#01:00.1 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.2 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.3 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.4 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)

if [ $name == "rkwiles-mini-i7" ]; then
./app/app/${target}/pktgen -c 1f -n 3 --proc-type auto --log-level 0 --socket-mem 512 --file-prefix pg -- -T -P -m "1.0, 2.1, 3.2, 4.3" -f themes/black-yellow.theme
fi
