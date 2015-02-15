#!/bin/bash

# Normal setup
#   different cores for each port.

name=`uname -n`

# Use 'sudo -E ./setup.sh' to include environment variables

if [ -z ${RTE_SDK} ] ; then
    echo "*** RTE_SDK is not set, did you forget to do 'sudo -E ./setup.sh'"
    exit 1
else
    sdk=${RTE_SDK}
fi

if [ -z ${RTE_TARGET} ]; then
    echo "*** RTE_TARGET is not set, did you forget to do 'sudo -E ./setup.sh'"
    target=x86_64-pktgen-linuxapp-gcc
else
    target=${RTE_TARGET}
fi


#rkwiles@rkwiles-X10DRH:~/projects/intel/dpdk$ lspci |grep Ether
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

if [ $name == "rkwiles-X10DRH" ]; then
./app/app/${target}/pktgen -c 1ffff -n 3 --proc-type auto --socket-mem 1024,1024 --file-prefix pg -b 0000:09:00.0 -b 0000:09:00.1 -b 0000:83:00.0 -- -T -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3, [9:11].4, [10:12].5, [13:15].6, [14:16].7" -f themes/black-yellow.theme
fi

#00:19.0 Ethernet controller: Intel Corporation Ethernet Connection (2) I218-V
#01:00.1 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.2 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.3 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.4 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)

if [ $name == "mini-i7" ]; then
./app/app/${target}/pktgen -c 1f -n 3 --proc-type auto --socket-mem 512 --file-prefix pg -- -T -P -m "1.0, 2.1, 3.2, 4.3" -f themes/black-yellow.theme
fi
