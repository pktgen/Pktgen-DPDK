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

if [ $name == "keithw-W2600CR" ]; then
./app/pktgen -c 1ff -n 3 --proc-type auto --socket-mem 256,256 --file-prefix pg -b 0000:03:00.0 -b 0000:03:00.1 -b 0000:07:00.0 -b 0000:07:00.1 -- -T -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -f themes/black-yellow.theme
fi

#keithw@keithw-S5520HC:~/projects/dpdk/Pktgen-DPDK/dpdk/examples/pktgen$ lspci | grep Ether
#01:00.0 Ethernet controller: Intel Corporation 82575EB Gigabit Network Connection (rev 02)				 <== blacklisted
#01:00.1 Ethernet controller: Intel Corporation 82575EB Gigabit Network Connection (rev 02)				 <== blacklisted
#04:00.0 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)	 <== using this one
#04:00.1 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)	 <== using this one
#07:00.0 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)
#07:00.1 Ethernet controller: Intel Corporation 82599ES 10-Gigabit SFI/SFP+ Network Connection (rev 01)

if [ $name == "keithw-S5520HC" ]; then
./app/pktgen -c 1f -n 3 --proc-type auto --socket-mem 256,256 --file-prefix pg -b 0000:01:00.0 -b 0000:01:00.1 -- -T -P -m "[1:3].0, [2:4].2" 
fi
