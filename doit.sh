#!/bin/bash

# Normal setup
#   different cores for each port.

name=`uname -n`

#[15:47][keithw@keithw-W2600CR:umf(dev)]$ lspci | grep Ether
#03:00.0 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)  <== blacklisted
#03:00.1 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)  <== blacklisted
#07:00.0 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)				 <== blacklisted
#07:00.1 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)				 <== blacklisted
#83:00.0 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)  <== using this one
#83:00.1 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)  <== using this one
#85:00.0 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)
#85:00.1 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)
#88:00.0 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)
#88:00.1 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)

if [ $name == "crownpass1.intel.com" ]; then
./app/pktgen -c 1ff -n 3 --proc-type auto --socket-mem 256,256 --file-prefix pg -b 0000:03:00.0 -b 0000:03:00.1 -b 0000:07:00.0 -b 0000:07:00.1 -- -T -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -f themes/black-yellow.theme
fi

#keithw@keithw-S5520HC:~/projects/dpdk/Pktgen-DPDK/dpdk/examples/pktgen$ lspci | grep Ether
#01:00.0 Ethernet controller: Intel Corporation 82575EB Gigabit Network Connection (rev 02)				 <== blacklisted
#01:00.1 Ethernet controller: Intel Corporation 82575EB Gigabit Network Connection (rev 02)				 <== blacklisted
#04:00.0 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)	 <== using this one
#04:00.1 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)	 <== using this one
#07:00.0 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)
#07:00.1 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)

if [ $name == "crownpass2.intel.com" ]; then
./app/pktgen -c 1f -n 3 --proc-type auto --socket-mem 256,256 --file-prefix pg -b 0000:01:00.0 -b 0000:01:00.1 -- -T -P -m "[1:3].0, [2:4].2" 
fi

#00:19.0 Ethernet controller: Intel Corporation Ethernet Connection (2) I218-V
#01:00.1 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.2 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.3 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.4 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)

if [ $name == "mini-i7" ]; then
./app/pktgen -c 1f -n 3 --proc-type auto --socket-mem 512 --file-prefix pg -- -T -P -m "1.0, 2.1, 3.2, 4.3" -f themes/black-yellow.theme
fi
