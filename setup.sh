#!/bin/bash

# Use 'sudo -E ./setup.sh' to include environment variables

if [ -z ${RTE_SDK} ] ; then
	echo "*** RTE_SDK is not set, did you forget to do 'sudo -E ./setup.sh'"
	echo "    Using "${RTE_SDK}
	export RTE_SDK=/work/home/rkwiles/projects/intel/dpdk
	export RTE_TARGET=x86_64-native-linuxapp-clang
fi
sdk=${RTE_SDK}

if [ -z ${RTE_TARGET} ]; then
	echo "*** RTE_TARGET is not set, did you forget to do 'sudo -E ./setup.sh'"
	target=x86_64-pktgen-linuxapp-gcc
else
	target=${RTE_TARGET}
fi

echo "Using directory: "$sdk"/"$target

function nr_hugepages_fn {
    echo /sys/devices/system/node/node${1}/hugepages/hugepages-2048kB/nr_hugepages
}

function num_cpu_sockets {
    local sockets=0
    while [ -f $(nr_hugepages_fn $sockets) ]; do
		sockets=$(( $sockets + 1 ))
    done
    echo $sockets
	if [ $sockets -eq 0 ]; then
		echo "Huge TLB support not found make sure you are using a kernel >= 2.6.34" >&2
		exit 1
	fi
}

if [ $UID -ne 0 ]; then
    echo "You must run this script as root" >&2
    exit 1
fi

rm -fr /mnt/huge/*

NR_HUGEPAGES=$(( `sysctl -n vm.nr_hugepages` / $(num_cpu_sockets) ))
echo "Setup "$(num_cpu_sockets)" socket(s) with "$NR_HUGEPAGES" pages."
for socket in $(seq 0 $(( $(num_cpu_sockets) - 1 )) ); do
    echo $NR_HUGEPAGES > $(nr_hugepages_fn $socket)
done

grep -i huge /proc/meminfo
modprobe uio
echo "trying to remove old igb_uio module and may get an error message, ignore it"
rmmod igb_uio
insmod $sdk/$target/kmod/igb_uio.ko
echo "trying to remove old rte_kni module and may get an error message, ignore it"
rmmod rte_kni
insmod $sdk/$target/kmod/rte_kni.ko "lo_mode=lo_mode_ring"

name=`uname -n`
if [ $name == "rkwiles-supermicro" ]; then
$sdk/tools/dpdk_nic_bind.py -b igb_uio 06:00.0 06:00.1 08:00.0 08:00.1 87:00.0 87:00.1 89:00.0 89:00.1
#$sdk/tools/dpdk_nic_bind.py -b igb_uio 87:00.0 87:00.1 89:00.0 89:00.1
fi
if [ $name == "rkwilesmini-i7" ]; then
$sdk/tools/dpdk_nic_bind.py -b igb_uio 01:00.1 01:00.2 01:00.3 01:00.4
fi
$sdk/tools/dpdk_nic_bind.py --status
lspci |grep Ether
