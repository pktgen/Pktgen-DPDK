Pktgen - Traffic Generator powered by DPDK
=====================================================

**Pktgen is a traffic generator powered by DPDK at wire rate traffic with 64 byte frames.**

** (Pktgen) Sounds like 'Packet-Gen'**

---
**Copyright &copy; \<2010-2019\>, Intel Corporation. All rights reserved.**

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

- Neither the name of Intel Corporation nor the names of its
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

SPDX-License-Identifier: BSD-3-Clause

Pktgen: Created 2010-2018 by Keith Wiles @ Intel.com
---




*** Pktgen command line directory format ***

``
-- Pktgen Ver: 3.2.0 (DPDK 17.05.0-rc0)  Powered by DPDK ---------------



Pktgen:/> ls
[pktgen]        [sbin]          copyright
Pktgen:/> ls pktgen/bin
off             on              debug           set             pcap
stp             str             stop            start           disable
enable          range           theme           page            seq
sequence        ping4           port            restart         rst
reset           cls             redisplay       save            lua
script          load            geom            geometry        clr
clear.stats     help
Pktgen:/>
``
-------------------------------------------------------------------------------
``
Pktgen:/> ls
[pktgen]        [sbin]          copyright
Pktgen:/> ls sbin
env             dbg             path            hugepages       cmap
sizes           more            history         quit            clear
pwd             cd              ls              rm              mkdir
chelp           sleep           delay
Pktgen:/>
``
-------------------------------------------------------------------------------
``
Pktgen:/> cd sbin
Pktgen:/sbin/>
Pktgen:/sbin/> ls -l
  env              Command : Set up environment variables
  dbg              Command : debug commands
  path             Command : display the command path list
  hugepages        Command : hugepages # display hugepage info
  cmap             Command : cmap # display the core mapping
  sizes            Command : sizes # display some internal sizes
  more             Command : more <file> # display a file content
  history          Command : history # display the current history
  quit             Command : quit # quit the application
  clear            Command : clear # clear the screen
  pwd              Command : pwd # display current working directory
  cd               Command : cd <dir> # change working directory
  ls               Command : ls [-lr] <dir> # list current directory
  rm               Command : remove a file or directory
  mkdir            Command : create a directory
  chelp            Command : CLI help - display information for DPDK
  sleep            Command : delay a number of seconds
  delay            Command : delay a number of milliseconds

Pktgen:/sbin/>
Pktgen:/sbin/> cd ..
Pktgen:/>
``
-------------------------------------------------------------------------------
``
Pktgen:/pktgen/> cd bin
Pktgen:/pktgen/bin/> ls -l
  off              Alias : disable screen
  on               Alias : enable screen
  debug            Command : debug commands
  set              Command : set a number of options
  pcap             Command : pcap commands
  stp              Alias : stop all
  str              Alias : start all
  stop             Command : stop features
  start            Command : start features
  disable          Command : disable features
  enable           Command : enable features
  range            Command : Range commands
  theme            Command : Set, save, show the theme
  page             Command : change page displays
  seq              Alias : sequence
  sequence         Command : sequence command
  ping4            Command : Send a ping packet for IPv4
  port             Command : Switch between ports
  restart          Command : restart port
  rst              Alias : reset all
  reset            Command : reset pktgen configuration
  cls              Alias : redisplay
  redisplay        Command : redisplay the screen
  save             Command : save the current state
  lua              Command : execute a Lua string
  script           Command : run a Lua script
  load             Command : load command file
  geom             Alias : geometry
  geometry         Command : set the screen geometry
  clr              Alias : clear.stats all
  clear.stats      Command : clear stats
  help             Command : help command

Pktgen:/pktgen/bin/>
``


run pktgen type `sudo -E ./tools/run.sh`.
`run.sh` is a script designed to help you with the command line options of pktgen.
You may need to modify this script for your system and configuration.

    # cat ./tools/run.sh
    #!/bin/bash

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
./app/app/${target}/pktgen -l 4-12 -n 3 --proc-type auto --file-prefix pg -b 06:00.0 -b 06:00.1 -b 08:00.0 -b 08:00.1 -b 09:00.0 -b 09:00.1 -b 83:00.1 -- -T -P -m "[5:7].0, [6:8].1, [9:11].2, [10:12].3" -f themes/black-yellow.theme
fi
``
** Note: The '-m NNN' in the DPDK arguments is to have DPDK allocate 512 megs of memory.
 The '--socket-mem 256,156' DPDK command will allocate 256M from each CPU (two in this
 case). Do not use the '-m NNN' and '--socket-mem NN,NN' commands on the same command
 line.

** Note: As of DPDK 18.05 using -m or --socket-mem is not required as memory is now
dynamically allocated.

The pktgen program follows the same format as a standard DPDK linuxapp, meaning
the first set of arguments '-l 0-4' are the standard DPDK arguments. This option
defines the number of logical cores to be used by pktgen. The 1f states 5 lcores
are used and the '3c' is just a bit array for each lcore to be used. The '-P' enables
promiscuous mode on all ports if you need that support. The '-m "..."' sets up the
port to lcore mapping for pktgen.

The second half of the command line followed by the '--' is pktgen specific
options.

The pktgen requires 2 logical cores for a minimum system. The first lcore 0 used
for pktgen command line and is used for timers also displaying the text on the
screen and lcores 1-n are used to do the packet receive and transmits along with
anything else related to packets. You do not need to start at lcore zero, but the first
lcore in the bitmap from in the least signification bit location is the display and
timer lcore used by pktgen.

The -m format now allows the developer to have different lcores for rx and tx
instead of rx/tx on the same lcore. The above format of {1:3}.0 states to use
lcore 1 for rx and lcore 3 for tx on port 0. Make sure you select different
lcores for the rx and tx so that only a single rx or tx function is on
a single physical core. If you pick the rx/tx functions on different lcores, but
on the same physical core you will not be able to get the performance you want as the
single physical core will be trying to do both Rx/Tx functions.

*** New setup and run python script with config files ***

Using the new tools/run.py script to setup and run pktgen with different configurations. The configuration files are located in the cfg directory with filenames ending in .cfg.

To use a configuration file;
``
$ ./tools/run.py -s default  # to setup the ports and attach them to DPDK (only needed once per boot)

$ ./tools/run.py default     # Run the default configuration
``
The configuration files are python scripts or a set of variables that run.py uses to initialize and run pktgen.
Here is an example of the default.cfg file:

``
	description = 'A Pktgen default simple configuration'

	# Setup configuration
	setup = {
	    'exec': (
		'sudo', '-E'
		),

	    'devices': (
		    '81:00.0', '81:00.1', '81:00.2', '81:00.3',
		    '83:00.0', '83:00.1', '83:00.2', '83:00.3'
		    ),
	    # UIO module type, igb_uio, vfio-pci or uio_pci_generic
	    'uio': 'vfio-pci'
	    }

	# Run command and options
	run = {
	    'exec': (
		'sudo', '-E'
		),

	    # Application name and use app_path to help locate the app
	    'app_name': 'pktgen',

	    # using (sdk) or (target) for specific variables
	    # add (app_name) of the application
	    # Each path is tested for the application
	    'app_path': (
		'./app/%(target)s/%(app_name)s',
		'%(sdk)s/%(target)s/app/%(app_name)s',
		),

	    'cores': '14,15-22',
	    'nrank': '4',
	    'proc': 'auto',
	    'log': '7',
	    'prefix': 'pg',
    
	    'blacklist': (
		#'81:00.0', '81:00.1', '81:00.2', '81:00.3',
		#'83:00.0', '83:00.1', '83:00.2', '83:00.3',
		'81:00.2', '81:00.3',
		'83:00.2', '83:00.3'
		),
		
	    'opts': (
		'-T',
		'-P',
		),
	    'map': (
		'[15:16].0',
		'[17:18].1',
		'[19:20].2',
		'[21:22].3'
		),

	    'theme': 'themes/black-yellow.theme'
	}
``

``
Usage: ./app/pktgen -l CORELIST [-n NUM] [-m NB] [-r NUM] [-b <domain:bus:devid.func>][--proc-type primary|secondary|auto]

Copyright (c) <2010-2019>, Intel Corporation. All rights reserved. Powered by DPDK
./app/app/x86_64-dnet-linuxapp-gcc/pktgen: invalid option -- 'x'
EAL: Detected 72 lcore(s)
./app/app/x86_64-dnet-linuxapp-gcc/pktgen: invalid option -- 'x'

Usage: ./app/app/x86_64-dnet-linuxapp-gcc/pktgen [options]

EAL common options:
  -c COREMASK         Hexadecimal bitmask of cores to run on
  -l CORELIST         List of cores to run on
                      The argument format is <c1>[-c2][,c3[-c4],...]
                      where c1, c2, etc are core indexes between 0 and 128
  --lcores COREMAP    Map lcore set to physical cpu set
                      The argument format is
                            '<lcores[@cpus]>[<,lcores[@cpus]>...]'
                      lcores and cpus list are grouped by '(' and ')'
                      Within the group, '-' is used for range separator,
                      ',' is used for single number separator.
                      '( )' can be omitted for single element group,
                      '@' can be omitted if cpus and lcores have the same value
  --master-lcore ID   Core ID that is used as master
  -n CHANNELS         Number of memory channels
  -m MB               Memory to allocate (see also --socket-mem)
  -r RANKS            Force number of memory ranks (don't detect)
  -b, --pci-blacklist Add a PCI device in black list.
                      Prevent EAL from using this PCI device. The argument
                      format is <domain:bus:devid.func>.
  -w, --pci-whitelist Add a PCI device in white list.
                      Only use the specified PCI devices. The argument format
                      is <[domain:]bus:devid.func>. This option can be present
                      several times (once per device).
                      [NOTE: PCI whitelist cannot be used with -b option]
  --vdev              Add a virtual device.
                      The argument format is <driver><id>[,key=val,...]
                      (ex: --vdev=net_pcap0,iface=eth2).
  -d LIB.so|DIR       Add a driver or driver directory
                      (can be used multiple times)
  --vmware-tsc-map    Use VMware TSC map instead of native RDTSC
  --proc-type         Type of this process (primary|secondary|auto)
  --syslog            Set syslog facility
  --log-level         Set default log level
  -v                  Display version information on startup
  -h, --help          This help

EAL options for DEBUG use only:
  --huge-unlink       Unlink hugepage files after init
  --no-huge           Use malloc instead of hugetlbfs
  --no-pci            Disable PCI
  --no-hpet           Disable HPET
  --no-shconf         No shared config (mmap'd files)

EAL Linux options:
  --socket-mem        Memory to allocate on sockets (comma separated values)
  --huge-dir          Directory where hugetlbfs is mounted
  --file-prefix       Prefix for hugepage filenames
  --base-virtaddr     Base virtual address
  --create-uio-dev    Create /dev/uioX (usually done by hotplug)
  --vfio-intr         Interrupt mode for VFIO (legacy|msi|msix)
  --xen-dom0          Support running on Xen dom0 without hugetlbfs


===== Application Usage =====

Usage: ./app/app/x86_64-dnet-linuxapp-gcc/pktgen [EAL options] -- [-h] [-v] [-P] [-G] [-T] [-f cmd_file] [-l log_file] [-s P:PCAP_file[,PCap_file]] [-m <string>]
  -s P:file1,file2    PCAP packet stream file per queue, 'P' is the port number
  -f filename  Command file (.pkt) to execute or a Lua script (.lua) file
  -l filename  Write log to filename
  -P           Enable PROMISCUOUS mode on all ports
  -g address   Optional IP address and port number default is (localhost:0x5606)
               If -g is used that enable socket support as a server application
  -G           Enable socket support using default server values localhost:0x5606
  -N           Enable NUMA support
  -T           Enable the color output
  -v           Verbose flags for startup messages
  --crc-strip  Strip CRC on all ports
  -m <string>  matrix for mapping ports to logical cores
      BNF: (or kind of BNF)
      <matrix-string>   := """ <lcore-port> { "," <lcore-port>} """
      <lcore-port>      := <lcore-list> "." <port-list>
      <lcore-list>      := "[" <rx-list> ":" <tx-list> "]"
      <port-list>       := "[" <rx-list> ":" <tx-list>"]"
      <rx-list>         := <num> { "/" (<num> | <list>) }
      <tx-list>         := <num> { "/" (<num> | <list>) }
      <list>            := <num> { "/" (<range> | <list>) }
      <range>           := <num> "-" <num> { "/" <range> }
      <num>             := <digit>+
      <digit>           := 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
      1.0, 2.1, 3.2                 - core 1 handles port 0 rx/tx,
                                      core 2 handles port 1 rx/tx
                                      core 3 handles port 2 rx/tx
      1.[0-2], 2.3, ...             - core 1 handle ports 0,1,2 rx/tx,
                                      core 2 handle port 3 rx/tx
      [0-1].0, [2/4-5].1, ...       - cores 0-1 handle port 0 rx/tx,
                                      cores 2,4,5 handle port 1 rx/tx
      [1:2].0, [4:6].1, ...         - core 1 handles port 0 rx,
                                      core 2 handles port 0 tx,
      [1:2].[0-1], [4:6].[2/3], ... - core 1 handles port 0 & 1 rx,
                                      core 2 handles port  0 & 1 tx
      [1:2-3].0, [4:5-6].1, ...     - core 1 handles port 0 rx, cores 2,3 handle port 0 tx
                                      core 4 handles port 1 rx & core 5,6 handles port 1 tx
      [1-2:3].0, [4-5:6].1, ...     - core 1,2 handles port 0 rx, core 3 handles port 0 tx
                                      core 4,5 handles port 1 rx & core 6 handles port 1 tx
      [1-2:3-5].0, [4-5:6/8].1, ... - core 1,2 handles port 0 rx, core 3,4,5 handles port 0 tx
                                      core 4,5 handles port 1 rx & core 6,8 handles port 1 tx
      [1:2].[0:0-7], [3:4].[1:0-7], - core 1 handles port 0 rx, core 2 handles ports 0-7 tx
                                      core 3 handles port 1 rx & core 4 handles port 0-7 tx
      BTW: you can use "{}" instead of "[]" as it does not matter to the syntax.
  -h           Display the help information
``
A new feature for pktgen and DPDK is to run multiple instances of pktgen. This
allows the developer to share ports on the same machine.

------------- run.sh script ----------------
``
#!/bin/bash

# Normal setup
# different cores for each port.

name=`uname -n`

# Use './tools/setup.sh' to include environment variables

if [ -z ${RTE_SDK} ] ; then
echo "*** RTE_SDK is not set, did you forget to do 'sudo -E ./tools/setup.sh'"
	export RTE_SDK=/work/home/rkwiles/projects/intel/dpdk
	export RTE_TARGET=x86_64-native-linuxapp-clang
fi
sdk=${RTE_SDK}

if [ -z ${RTE_TARGET} ]; then
echo "*** RTE_TARGET is not set, did you forget to do 'sudo -E ./tools/setup.sh'"
target=x86_64-pktgen-linuxapp-gcc
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
./app/app/${target}/pktgen -l 4-16 -n 3 --proc-type auto --file-prefix pg -b 06:00.0 -b 06:00.1 -b 08:00.0 -b 08:00.1 -b 09:00.0 -b 09:00.1 -b 83:00.1 -- -T -P -m "[5:7].0, [6:8].1, [9:11].2, [10:12].3" -f themes/black-yellow.theme
fi

#00:19.0 Ethernet controller: Intel Corporation Ethernet Connection (2) I218-V
#01:00.1 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.2 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.3 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.4 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)

if [ $name == "rkwiles-mini-i7" ]; then
./app/app/${target}/pktgen -l 0-4 -n 3 --proc-type auto --file-prefix pg -- -T -P -m "1.0, 2.1, 3.2, 4.3" -f themes/black-yellow.theme
fi
``
------------- run.sh script ----------------

------------- setup.sh script ----------------
``
#!/bin/bash

# Use './tools/setup.sh' to include environment variables

if [ -z ${RTE_SDK} ] ; then
	echo "*** RTE_SDK is not set, did you forget to do 'sudo -E ./tools/setup.sh'"
	echo "Using "${RTE_SDK}
	export RTE_SDK=/work/home/rkwiles/projects/intel/dpdk
	export RTE_TARGET=x86_64-native-linuxapp-clang
fi
sdk=${RTE_SDK}

if [ -z ${RTE_TARGET} ]; then
	echo "*** RTE_TARGET is not set, did you forget to do 'sudo -E ./tools/setup.sh'"
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
fi
if [ $name == "rkwilesmini-i7" ]; then
	$sdk/tools/dpdk_nic_bind.py -b igb_uio 01:00.1 01:00.2 01:00.3 01:00.4
fi
$sdk/tools/dpdk_nic_bind.py --status
lspci |grep Ether
``
------------- setup script ----------------

If you have run pktgen before then remove the files in /mnt/huge/* before
running the new version.

Running the run.sh script produces output as follows, but maybe different on your
system configuration.
``
rkwiles@broadwell (dev):~/.../intel/pktgen$ ./tools/run.py default
>>> sdk '/work/home/rkwiles/projects/intel/dpdk.org', target 'x86_64-native-linuxapp-gcc'
   Trying ./app/x86_64-native-linuxapp-gcc/pktgen
sudo -E ./app/x86_64-native-linuxapp-gcc/pktgen -l 14,15-22 -n 4 --proc-type auto --log-level 7 --file-prefix pg -b 81:00.2 -b 81:00.3 -b 85:00.2 -b 85:00.3 -b 83:00.0 -- -T -P --crc-strip -m [15:16].0 -m [17:18].1 -m [19:20].2 -m [21:22].3 -f themes/black-yellow.theme

Copyright (c) <2010-2019>, Intel Corporation. All rights reserved. Powered by DPDK
EAL: Detected 56 lcore(s)
EAL: Detected 2 NUMA nodes
EAL: Auto-detected process type: PRIMARY
EAL: Multi-process socket /var/run/dpdk/pg/mp_socket
EAL: No free hugepages reported in hugepages-1048576kB
EAL: Probing VFIO support...
EAL: PCI device 0000:01:00.0 on NUMA socket 0
EAL:   probe driver: 8086:1521 net_e1000_igb
EAL: PCI device 0000:01:00.1 on NUMA socket 0
EAL:   probe driver: 8086:1521 net_e1000_igb
EAL: PCI device 0000:81:00.0 on NUMA socket 1
EAL:   probe driver: 8086:1572 net_i40e
EAL: PCI device 0000:81:00.1 on NUMA socket 1
EAL:   probe driver: 8086:1572 net_i40e
EAL: PCI device 0000:81:00.2 on NUMA socket 1
EAL:   Device is blacklisted, not initializing
EAL: PCI device 0000:81:00.3 on NUMA socket 1
EAL:   Device is blacklisted, not initializing
EAL: PCI device 0000:83:00.0 on NUMA socket 1
EAL:   Device is blacklisted, not initializing
EAL: PCI device 0000:85:00.0 on NUMA socket 1
EAL:   probe driver: 8086:1572 net_i40e
EAL: PCI device 0000:85:00.1 on NUMA socket 1
EAL:   probe driver: 8086:1572 net_i40e
EAL: PCI device 0000:85:00.2 on NUMA socket 1
EAL:   Device is blacklisted, not initializing
EAL: PCI device 0000:85:00.3 on NUMA socket 1
EAL:   Device is blacklisted, not initializing
Lua 5.3.5  Copyright (C) 1994-2018 Lua.org, PUC-Rio

*** Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
*** Pktgen created by: Keith Wiles -- >>> Powered by DPDK <<<

Initialize Port 0 -- TxQ 1, RxQ 1,  Src MAC 3c:fd:fe:a1:2b:40
Initialize Port 1 -- TxQ 1, RxQ 1,  Src MAC 3c:fd:fe:a1:2b:41
Initialize Port 2 -- TxQ 1, RxQ 1,  Src MAC 3c:fd:fe:a1:2b:08
Initialize Port 3 -- TxQ 1, RxQ 1,  Src MAC 3c:fd:fe:a1:2b:09

Port  0: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>
Port  1: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>
Port  2: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>
Port  3: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>


- Ports 0-3 of 4   <Main Page>  Copyright (c) <2010-2019>, Intel Corporation
  Flags:Port        :   P--------------:0   P--------------:1   P--------------:2   P--------------:3
Link State          :       <UP-10000-FD>       <UP-10000-FD>       <UP-10000-FD>       <UP-10000-FD>     ----TotalRate----
Pkts/s Max/Rx       :                 0/0                 0/0                 0/0                 0/0                   0/0
       Max/Tx       :                 0/0                 0/0                 0/0                 0/0                   0/0
MBits/s Rx/Tx       :                 0/0                 0/0                 0/0                 0/0                   0/0
Broadcast           :                   0                   0                   0                   0
Multicast           :                   0                   0                   0                   0
Sizes 64            :                   0                   0                   0                   0
      65-127        :                   0                   0                   0                   0
      128-255       :                   0                   0                   0                   0
      256-511       :                   0                   0                   0                   0
      512-1023      :                   0                   0                   0                   0
      1024-1518     :                   0                   0                   0                   0
Runts/Jumbos        :                 0/0                 0/0                 0/0                 0/0
ARP/ICMP Pkts       :                 0/0                 0/0                 0/0                 0/0
Errors Rx/Tx        :                 0/0                 0/0                 0/0                 0/0
Total Rx Pkts       :                   0                   0                   0                   0
      Tx Pkts       :                   0                   0                   0                   0
      Rx MBs        :                   0                   0                   0                   0
      Tx MBs        :                   0                   0                   0                   0
                    :
Pattern Type        :             abcd...             abcd...             abcd...             abcd...
Tx Count/% Rate     :       Forever /100%       Forever /100%       Forever /100%       Forever /100%
Pkt Size/Tx Burst   :           64 /   64           64 /   64           64 /   64           64 /   64
Port Src/Dest       :         1234 / 5678         1234 / 5678         1234 / 5678         1234 / 5678
Pkt Type:VLAN ID    :     IPv4 / TCP:0001     IPv4 / TCP:0001     IPv4 / TCP:0001     IPv4 / TCP:0001
802.1p CoS/DSCP/IPP :           0/  0/  0           0/  0/  0           0/  0/  0           0/  0/  0
VxLAN Flg/Grp/vid   :       0/    0/    0       0/    0/    0       0/    0/    0       0/    0/    0
IP  Destination     :         192.168.1.1         192.168.0.1         192.168.3.1         192.168.2.1
    Source          :      192.168.0.1/24      192.168.1.1/24      192.168.2.1/24      192.168.3.1/24
MAC Destination     :   3c:fd:fe:a1:2b:41   3c:fd:fe:a1:2b:40   3c:fd:fe:a1:2b:09   3c:fd:fe:a1:2b:08
    Source          :   3c:fd:fe:a1:2b:40   3c:fd:fe:a1:2b:41   3c:fd:fe:a1:2b:08   3c:fd:fe:a1:2b:09
PCI Vendor/Addr     :   8086:1572/81:00.0   8086:1572/81:00.1   8086:1572/85:00.0   8086:1572/85:00.1

-- Pktgen Ver: 3.5.9 (DPDK 19.02.0-rc1)  Powered by DPDK ----------------------


Pktgen:/> quit
$
``
------------------------------------------------------------------------
``
                                                  ** Pktgen Help Information **
page [0-7]                         - Show the port pages or configuration or sequence page
page main                          - Display page zero
page range                         - Display the range packet page
page config | cfg                  - Display the configuration page
page pcap                          - Display the pcap page
page cpu                           - Display some information about the CPU system
page next                          - Display next page of PCAP packets.
page sequence | seq                - sequence will display a set of packets for a given port
                                     Note: use the 'port <number>' to display a new port sequence
page rnd                           - Display the random bitfields to packets for a given port
                                     Note: use the 'port <number>' to display a new port sequence
page log                           - Display the log messages page
page latency                       - Display the latency page
page stats                         - Display physical ports stats for all ports
page xstats                        - Display port XSTATS values

enable|disable <portlist> process  - Enable or Disable processing of ARP/ICMP/IPv4/IPv6 packets
enable|disable <portlist> mpls     - Enable/disable sending MPLS entry in packets
enable|disable <portlist> qinq     - Enable/disable sending Q-in-Q header in packets
enable|disable <portlist> gre      - Enable/disable GRE support
enable|disable <portlist> gre_eth  - Enable/disable GRE with Ethernet frame payload
enable|disable <portlist> vlan     - Enable/disable VLAN tagging
enable|disable <portlist> garp     - Enable or Disable GARP packet processing and update MAC address
enable|disable <portlist> random   - Enable/disable Random packet support
enable|disable <portlist> latency  - Enable/disable latency testing
enable|disable <portlist> pcap     - Enable or Disable sending pcap packets on a portlist
enable|disable <portlist> blink    - Blink LED on port(s)
enable|disable <portlist> rx_tap   - Enable/Disable RX Tap support
enable|disable <portlist> tx_tap   - Enable/Disable TX Tap support
enable|disable <portlist> icmp     - Enable/Disable sending ICMP packets
enable|disable <portlist> range    - Enable or Disable the given portlist for sending a range of packets
enable|disable <portlist> capture  - Enable/disable packet capturing on a portlist, disable to save capture
                                     Disable capture on a port to save the data into the currect working directory.
enable|disable <portlist> bonding  - Enable call TX with zero packets for bonding driver
enable|disable <portlist> short    - Allow shorter then 64 byte frames to be sent
enable|disable <portlist> vxlan    - Send VxLAN packets
enable|disable mac_from_arp        - Enable/disable MAC address from ARP packet
enable|disable screen              - Enable/disable updating the screen and unlock/lock window
    off                            - screen off shortcut
    on                             - screen on shortcut

    note: <portlist>               - a list of ports (no spaces) e.g. 2,4,6-9,12 or the word 'all'
set <portlist> count <value>       - number of packets to transmit
set <portlist> size <value>        - size of the packet to transmit
set <portlist> rate <percent>      - Packet rate in percentage
set <portlist> burst <value>       - number of packets in a burst
set <portlist> tx_cycles <value>   - DEBUG to set the number of cycles per TX burst
set <portlist> sport <value>       - Source port number for TCP
set <portlist> dport <value>       - Destination port number for TCP
set <portlist> seq_cnt|seqcnt|seqCnt <value>
                                   - Set the number of packet in the sequence to send [0-16]
set <portlist> prime <value>       - Set the number of packets to send on prime command
set <portlist> dump <value>        - Dump the next N received packets to the screen
set <portlist> vlan <value>        - Set the VLAN ID value for the portlist
set <portlist> jitter <value>      - Set the jitter threshold in micro-seconds
set <portlist> src|dst mac <addr>  - Set MAC addresses 00:11:22:33:44:55 or 0011:2233:4455 format
set <portlist> type ipv4|ipv6|vlan|arp - Set the packet type to IPv4 or IPv6 or VLAN
set <portlist> proto udp|tcp|icmp  - Set the packet protocol to UDP or TCP or ICMP per port
set <portlist> pattern <type>      - Set the fill pattern type
                 type - abc        - Default pattern of abc string
                        none       - No fill pattern, maybe random data
                        zero       - Fill of zero bytes
                        user       - User supplied string of max 16 bytes
set <portlist> user pattern <string> - A 16 byte string, must set 'pattern user' command
set <portlist> [src|dst] ip ipaddr - Set IP addresses, Source must include network mask e.g. 10.1.2.3/24
set <portlist> qinqids <id1> <id2> - Set the Q-in-Q ID's for the portlist
set <portlist> rnd <idx> <off> <mask> - Set random mask for all transmitted packets from portlist
    idx: random mask index slot
    off: offset in bytes to apply mask value
    mask: up to 32 bit long mask specification (empty to disable):
          0: bit will be 0
          1: bit will be 1
          .: bit will be ignored (original value is retained)
          X: bit will get random value
set <portlist> cos <value>         - Set the CoS value for the portlist
set <portlist> tos <value>         - Set the ToS value for the portlist
set <portlist> vxlan <flags> <group id> <vxlan_id> - Set the vxlan values
set ports_per_page <value>         - Set ports per page value 1 - 6

  -- Setup the packet range values --
   note: SMMI = start|min|max|inc (start, minimum, maximum, increment)

range <portlist> src|dst mac <SMMI> <etheraddr> - Set destination/source MAC address
      e.g: range 0 src mac start 00:00:00:00:00:00
           range 0 dst mac max 00:12:34:56:78:90
      or  range 0 src mac 00:00:00:00:00:00 00:00:00:00:00:00 00:12:34:56:78:90 00:00:00:01:01:01
range <portlist> src|dst ip <SMMI> <ipaddr>   - Set source IP start address
      e.g: range 0 dst ip start 0.0.0.0
           range 0 dst ip min 0.0.0.0
           range 0 dst ip max 1.2.3.4
           range 0 dst ip inc 0.0.1.0
       or  range 0 dst ip 0.0.0.0 0.0.0.0 1.2.3.4 0.0.1.0
range <portlist> proto tcp|udp                - Set the IP protocol type
range <portlist> src|dst port <SMMI> <value>  - Set UDP/TCP source/dest port number
       or  range <portlist> src|dst port <start> <min> <max> <inc>
range <portlist> vlan <SMMI> <value>          - Set vlan id start address
      or  range <portlist> vlan <start> <min> <max> <inc>
range <portlist> size <SMMI> <value>          - Set pkt size start address
      or  range <portlist> size <start> <min> <max> <inc>
range <portlist> teid <SMMI> <value>          - Set TEID value
      or  range <portlist> teid <start> <min> <max> <inc>
range <portlist> mpls entry <hex-value>       - Set MPLS entry value
range <portlist> qinq index <val1> <val2>     - Set QinQ index values
range <portlist> gre key <value>              - Set GRE key value
range <portlist> cos <SMMI> <value>           - Set cos value
range <portlist> tos <SMMI> <value>           - Set tos value

sequence <seq#> <portlist> dst <Mac> src <Mac> dst <IP> src <IP> sport <val> dport <val> ipv4|ipv6 udp|tcp|icmp vlan <val> size <val> [teid <val>]
sequence <seq#> <portlist> <dst-Mac> <src-Mac> <dst-IP> <src-IP> <sport> <dport> ipv4|ipv6 udp|tcp|icmp <vlanid> <pktsize> [<teid>]
sequence <seq#> <portlist> cos <cos> tos <tos>
                                   - Set the sequence packet information, make sure the src-IP
                                     has the netmask value eg 1.2.3.4/24

pcap show                          - Show PCAP information
pcap <index>                       - Move the PCAP file index to the given packet number,  0 - rewind, -1 - end of file
pcap filter <portlist> <string>    - PCAP filter string to filter packets on receive

start <portlist>                   - Start transmitting packets
stop <portlist>                    - Stop transmitting packets
stp                                - Stop all ports from transmitting
str                                - Start all ports transmitting
start <portlist> prime             - Transmit packets on each port listed. See set prime command above
start <portlist> arp <type>        - Send a ARP type packet
    type - request | gratuitous | req | grat

dbg l2p                          - Dump out internal lcore to port mapping
dbg tx_dbg                       - Enable tx debug output
dbg mempool|dump <portlist> <type>    - Dump out the mempool info for a given type
dbg pdump <portlist>             - Hex dump the first packet to be sent, single packet mode only
dbg memzone                      - List all of the current memzones
dbg memseg                       - List all of the current memsegs
dbg hexdump <addr> <len>         - hex dump memory at given address
dbg break                        - break into the debugger
dbg memcpy [loop-cnt KBytes]     - run a memcpy test

save <path-to-file>                - Save a configuration file using the filename
load <path-to-file>                - Load a command/script file from the given path
script <filename>                  - Execute the Lua script code in file (www.lua.org).
lua 'lua string'                   - Execute the Lua code in the string needs quotes
geometry <geom>                    - Set the display geometry Columns by Rows (ColxRow)
clear <portlist> stats             - Clear the statistics
clr                                - Clear all Statistices
reset <portlist>                   - Reset the configuration the ports to the default
rst                                - Reset the configuration for all ports
ports per page [1-6]               - Set the number of ports displayed per page
port <number>                      - Sets the sequence packets to display for a given port
restart <portlist>                 - Restart or stop a ethernet port and restart
ping4 <portlist>                   - Send a IPv4 ICMP echo request on the given portlist

theme <item> <fg> <bg> <attr>      - Set color for item with fg/bg color and attribute value
theme show                         - List the item strings, colors and attributes to the items
theme save <filename>              - Save the current color theme to a file

       Flags: P--------------- - Promiscuous mode enabled
               E               - ICMP Echo enabled
                A              - Send ARP Request flag
                 G             - Send Gratuitous ARP flag
                  C            - TX Cleanup flag
                   p           - PCAP enabled flag
                    S          - Send Sequence packets enabled
                     R         - Send Range packets enabled
                      D        - DPI Scanning enabled (If Enabled)
                       I       - Process packets on input enabled
                        *      - Using TAP interface for this port can be [-rt*]
                         L     - Send Latency packets
                          V    - Send VLAN ID tag
                          X    - Send VxLAN packets
                          M    - Send MPLS header
                          Q    - Send Q-in-Q tags
                           g   - Perform GRE with IPv4 payload
                           G   - Perform GRE with Ethernet payload
                            C  - Capture received packets
                             R - Random bitfield(s) are applied
Notes: <state>       - Use enable|disable or on|off to set the state.
       <portlist>    - a list of ports (no spaces) as 2,4,6-9,12 or 3-5,8 or 5 or the word 'all'
       Color best seen on a black background for now

``
---------------------------------------------------------------------------
``
\                  <Sequence Page>  Copyright (c) <2010-2019>, Intel Corporation
Port:  0, Sequence Count:  4 of 16                                                                             GTPu
  Seq:            Dst MAC           Src MAC          Dst IP            Src IP    Port S/D Protocol:VLAN  Size  TEID
*   0:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
*   1:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
*   2:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
*   3:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0













-- Pktgen Ver: 3.2.0 (DPDK 17.05.0-rc0)  Powered by DPDK ---------------
























Pktgen:/> set all seq_cnt 4
Pktgen:/>
``
---------------------------------------------------------------------------
``
| Port 0           <Random bitfield Page>  Copyright (c) <2010-2019>, Intel Corporation
   Index   Offset     Act?  Mask [0 = 0 bit, 1 = 1 bit, X = random bit, . = ignore]
       0        0      No   00000000 00000000 00000000 00000000
       1        0      No   00000000 00000000 00000000 00000000
       2        0      No   00000000 00000000 00000000 00000000
       3        0      No   00000000 00000000 00000000 00000000
       4        0      No   00000000 00000000 00000000 00000000
       5        0      No   00000000 00000000 00000000 00000000
       6        0      No   00000000 00000000 00000000 00000000
       7        0      No   00000000 00000000 00000000 00000000
       8        0      No   00000000 00000000 00000000 00000000
       9        0      No   00000000 00000000 00000000 00000000
      10        0      No   00000000 00000000 00000000 00000000
      11        0      No   00000000 00000000 00000000 00000000
      12        0      No   00000000 00000000 00000000 00000000
      13        0      No   00000000 00000000 00000000 00000000
      14        0      No   00000000 00000000 00000000 00000000
      15        0      No   00000000 00000000 00000000 00000000
      16        0      No   00000000 00000000 00000000 00000000
      17        0      No   00000000 00000000 00000000 00000000
      18        0      No   00000000 00000000 00000000 00000000
      19        0      No   00000000 00000000 00000000 00000000
      20        0      No   00000000 00000000 00000000 00000000
      21        0      No   00000000 00000000 00000000 00000000
      22        0      No   00000000 00000000 00000000 00000000
      23        0      No   00000000 00000000 00000000 00000000
      24        0      No   00000000 00000000 00000000 00000000
      25        0      No   00000000 00000000 00000000 00000000
      26        0      No   00000000 00000000 00000000 00000000
      27        0      No   00000000 00000000 00000000 00000000
      28        0      No   00000000 00000000 00000000 00000000
      29        0      No   00000000 00000000 00000000 00000000
      30        0      No   00000000 00000000 00000000 00000000
      31        0      No   00000000 00000000 00000000 00000000
-- Pktgen Ver: 3.2.0 (DPDK 17.05.0-rc0)  Powered by DPDK ---------------











Pktgen:/>
``
---------------------------------------------------------------------------
-- Example command lines.
``
./app/pktgen -l 0-8 -n 3 --proc-type auto -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -s 0:pcap/large.pcap
./app/pktgen -l 0-4 -n 3 --proc-type auto --file-prefix pg -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -s 0:pcap/test1.pcap -s 1:pcap/large.pcap
./app/pktgen -l 0-4 -n 3 --proc-type auto --file-prefix pg -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -s 0:pcap/test1.pcap -s 1:pcap/large.pcap
./app/pktgen -l 1-3 -n 3 --proc-type auto --file-prefix pg -- -P -m "2.0, 3.1"
./app/pktgen -l 0-8 -n 3 --proc-type auto -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3"
``

A command line passing in a pktgen/test/set_seq.pkt file to help initialize pktgen with some
default values and configurations. You can also replace the filename using the '-f' command
with a Lua script file ending in .lua instead of .pkt. BTW, if the filename ends in anything
other then .lua it is treated as a .pkt file.

`./app/pktgen -l 0-4 -n 3 --proc-type auto -- -P -m "[1:3].0, [2:4].1" -f test/set_seq.pkt`

-- test/set_seq.pkt
``
seq 0 all 0000:4455:6677 0000:1234:5678 10.11.0.1 10.10.0.1/16 5 6 ipv4 udp 1 128
set all seqCnt 1
``
The set_seq.pkt command file can also be one of the files in pktgen/test directory,
which are Lua based scripts instead of command line scripts as in set_seq.pkt file.

-- test/set_seq.lua
The Lua version is easier to remember the layout of the agruments if you want to
use that one instead of set_seq.pkt file.

``
./app/pktgen -l 0-4 -n 3 --proc-type auto -- -P -m "[1:3].0, [2:4].1" -f test/set_seq.lua`

-- The '--' is a comment in Lua
local seq_table = {			-- entries can be in any order
["eth_dst_addr"] = "0011:4455:6677",
["eth_src_addr"] = "0011:1234:5678",
["ip_dst_addr"] = "10.12.0.1",
["ip_src_addr"] = "10.12.0.1/16",	-- the 16 is the size of the mask value
["sport"] = 9,			-- Standard port numbers
["dport"] = 10,			-- Standard port numbers
["ethType"] = "ipv4",	-- ipv4|ipv6
["ipProto"] = "udp",	-- udp|tcp|icmp
["vlanid"] = 1,			-- 1 - 4095
["pktSize"] = 128		-- 64 - 1518
};
-- seqTable( seq#, portlist, table );
pktgen.seqTable(0, "all", seq_table );
pktgen.set("all", "seqCnt", 1);
``
------------------------------------------------------------------------------------------
-- Two Pktgens running on the same machine with connected via a loopback ports

Look at the two new files pktgen-master.sh and pktgen-slave.sh for some help on
the configuration to run two pktgens at the same time on the same machine.

------------------------------------------------------------------------------------------
-- Socket Support for Pktgen.

Pktgen has a TCP socket connection to allow you to control Pktgen from a remote
program or console. The TCP connection is using port 0x5606 or 22022 and presents
a Lua command shell interface. If you telnet to the machine running Pktgen on port
22022 you will get a lua command shell like interface. This interface does not have
a command line prompt, but you can issue Lua code or load script files from the local
disk of the machine. You can also send programs to the remote Pktgen machine to
load scripts from a remote location.

One method to connect to Pktgen is using telnet, but another method would be to
use 'socat' program on a Linux machine. The socat program is very powerfull application
and can do a lot of things. I used socat to debug Pktgen using the following
command, which gives me a readline inteface to Pktgen's socket interface.

$ socat -d -d READLINE TCP4:localhost:22022

'You will see socat create the connection and then wait for Lua command scripts for you'
To exit this command type Control-D to exit and close the connection.

You can also just send Pktgen a script file and display the ouptut.

---------
$ socat - TCP4:localhost:22022 < test/hello-world.lua

Lua Version: Lua 5.3
Pktgen Version : 3.6.1
Pktgen Copyright : Copyright (c) `<2010-2019>`, Intel Corporation
Pktgen Authors : Keith Wiles @ Intel Corporation

Hello World!!!!
--------

Here is the program I sent Pktgen:

$ cat test/hello-world.lua
package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

printf("Lua Vesrion: %s\n", pktgen.info.Lua_Version);
printf("Pktgen Version : %s\n", pktgen.info.Pktgen_Version);
printf("Pktgen Copyright : %s\n", pktgen.info.Pktgen_Copyright);
printf("Pktgen Authors : %s\n", pktgen.info.Pktgen_Authors);

printf("\nHello World!!!!\n");
-----------

Here is a command from my Mac Book pro laptop, which loads a file from the local
disk where Pktgen is running and then we execute the file with 'f()'.

------------------
$ socat READLINE TCP4:172.25.40.163:22022
f,e = loadfile("test/hello-world.lua")
f()
Lua Version: Lua 5.3
Pktgen Version : 3.6.1
Pktgen Copyright : Copyright (c) `<2010-2019>`, Intel Corporation
Pktgen Authors : Keith Wiles @ Intel Corporation

Hello World!!!!
<Control-D>
------------------

You can also just send it commands via echo.

-----------------
$ echo "f,e = loadfile('test/hello-world.lua'); f();"| socat - TCP4:172.25.40.163:22022
Lua Version: Lua 5.3
Pktgen Version : 3.6.1
Pktgen Copyright : Copyright (c) `<2010-2019>`, Intel Corporation
Pktgen Authors : Keith Wiles @ Intel Corporation

Hello World!!!!
----------------------

Keith Wiles @ Intel Corporation
