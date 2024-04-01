echo 0 > /sys/bus/pci/devices/0000\:82\:00.0/sriov_numvfs
echo 3 > /sys/bus/pci/devices/0000\:82\:00.0/sriov_numvfs

ifconfig ens260 down
ip link set dev ens260 vf 0 mac 00:11:22:33:44:01
ip link set dev ens260 vf 1 mac 00:11:22:33:44:02
ip link set dev ens260 vf 2 mac 00:11:22:33:44:03

../dpdk/usertools/dpdk-devbind.py -b vfio-pci 82:01.0 82:01.1 82:01.2
../dpdk/usertools/dpdk-devbind.py -s

