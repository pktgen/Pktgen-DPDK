echo 0 > /sys/bus/pci/devices/0000\:82\:00.0/sriov_numvfs
sleep 2
echo 2 > /sys/bus/pci/devices/0000\:82\:00.0/sriov_numvfs

ifconfig ens260f0 down
ip link set dev ens260f0 vf 0 mac 00:11:22:33:44:01
ip link set dev ens260f0 vf 1 mac 00:11:22:33:44:02

../dpdk/usertools/dpdk-devbind.py -b vfio-pci 82:02.0 82:02.1

