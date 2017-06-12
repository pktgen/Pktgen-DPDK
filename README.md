Pktgen - Traffic Generator powered by Intel's DPDK
=====================================================

**Pktgen is a traffic generator powered by Intel's DPDK at wire rate traffic with 64 byte frames.**

** (Pktgen) Sounds like 'Packet-Gen'**


---
**Copyright &copy; \<2010-2017\>, Intel Corporation All rights reserved.**

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

 Pktgen: Created 2010 by Keith Wiles @ windriver.com, now at Intel.com
 ---

**======================== README.md file ==============================**

*** Pktgen ***
Copyright &copy \<2015-2017\>, Intel Corporation.

README for setting up Pktgen with DPDK on Ubuntu 10.04 to 16.10 desktop, it
should work on most Linux systems as long as the kernel has hugeTLB page support.

Note: Tested with Ubuntu 13.10 and up to 16.10 kernel versions
Linux 3.5.0-25-generic #39-Ubuntu SMP Mon Feb 25 18:26:58 UTC 2013 x86_64

I am using Ubuntu 16.10 x86_64 (64 bit support) for running Pktgen-DPDK on a
Crownpass Dual socket board running at 2.4GHz with 32GB of ram 16GB per socket.
The current kernel version is 4.4.0-66-generic (as of 2017-04-01) support, but should
work on just about any new Linux kernel version.

Currently using as of 2017-04-01 Ubuntu 16.10 Kernel 4.4.0-66-generic system.

To get hugeTLB page support your Linux kernel must be at least 2.6.33 and in the
DPDK documents it talks about how you can upgrade your Linux kernel.

Here is another document on how to upgrade your Linux kernel.
Ubuntu 10.04 is 2.6.32 by default so upgraded to kernel 2.6.34 using this HOWTO:
http://usablesoftware.wordpress.com/2010/05/26/switch-to-a-newer-kernel-in-ubuntu-10-04/

The pktgen output display needs 132 columns and about 42 lines to display
currentlyt. I am using an xterm of 132x42, but you can have a larger display
and maybe a bit smaller. If you are displaying more then 4-6 ports then you
will need a wider display. Pktgen allows you to view a set of ports if they
do not all fit on the screen at one time via the 'page' command.

Type 'help' at the 'Pktgen>' prompt to see the complete Pktgen command line
commands. Pktgen uses VT100 control codes or escape codes to display the screens,
which means your terminal must support VT100. The Hyperterminal in windows is not
going to work for Pktgen as it has a few problems with VT100 codes.

Pktgen has a number of modes to send packets single, range, random, sequeue and
PCAP modes. Each mode has its own set of packet buffers and you must configure
each mode to work correctly. The single packet mode is the information displayed
at startup screen or when using the 'page main or page 0' command. The other
screens can be accessed using 'page seq|range|rnd|pcap|stats' command.

The pktgen program as built can send up to 16 packets per port in a sequence
and you can configure a port using the 'seq' pktgen command. A script file
can be loaded from the shell command line via the -f option and you can 'load'
a script file from within pktgen as well.

In the BIOS make sure the HPET High Precision Event Timer is enabled. Also
make sure hyper-threading is enabled.

** NOTE **
On a 10GB NIC if the transceivers are not attached the screen updates will go
very slow.

*** Pktgen command line directory format ***

``
-- Pktgen Ver: 3.2.0 (DPDK 17.05.0-rc0)  Powered by Intel® DPDK ---------------



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
Get the DPDK and pktgen source code from github.com or dpdk.org repo via:
``
# cd <InstallDir>
# git clone git://dpdk.org/dpdk.git
``
``
# cd <InstallDir>
# git clone git://dpdk.org/pktgen-dpdk.git
``
** Note **
The site dpdk.org you must also pull down DPDK repo as well. git://dpdk.org/dpdk

Will create a directory called Pktgen-DPDK in the current directory location. Using
the above clone commands you will get DPDK and pktgen source files.

Make sure you have HUGE TLB support in the kernel with the following commands:
``
# grep -i huge /boot/config-2.6.35-24-generic
CONFIG_HUGETLBFS=y
CONFIG_HUGETLB_PAGE=y

# grep -i huge /proc/meminfo
HugePages_Total:128
HugePages_Free: 128
HugePages_Rsvd:0
HugePages_Surp:0
Hugepagesize: 2048 kB
``
NOTE: The values in Total and Free maybe different until you reboot the machine.

Two files in /etc must be setup to support huge TLBs. If you do not have
hugeTLB support then you most likely need a newer kernel.
``
# vi /etc/sysctl.conf
Add to the bottom of the file for 2M hugepages:
vm.nr_hugepages=256
``
If you need more or less hugeTLB pages then you can change the value to a
number you need it to be. In some cases pktgen needs a fair number of pages
and making it too small will effect performance or pktgen will terminate on
startup looking for more pages.
``
# On Ubuntu 15.10 I noticed mounting /mnt/huge is not required as /dev/hugepages
# is already mounted. Check your system and verify that /mnt/huge is required.
# vi /etc/fstab
Add to the bottom of the file:
huge /mnt/huge hugetlbfs defaults 0 0

# mkdir /mnt/huge
# chmod 777 /mnt/huge
``
Reboot your machine as the huge pages must be setup just after boot to make
sure you have contiguous memory for the 2Meg pages, setting up 1G pages can
also be done.

** Note: If you startup Eclipse or WR Workbench before starting pktgen the first
 time after reboot, pktgen will fail to load because it can not get all of the
 huge pages as eclipse has consumed some of the huge pages. If you did start eclipse
 or WR Workbench then you need to close that application first.

This is my current machine you will have a few different numbers depending on
how your system was booted and if you had hugeTLB support enabled.

At the pktgen-DPDK level directory we have the 'tools/setup.sh' script,
which needs to be run as root once per boot. The script contains a commands to setup
the environment.

Make sure you run the setup script as root via './tools/setup.sh'. The setup
script is a bash script and tries to setup the system correctly, but you may have to
change the script to match your number of huge pages you configured above and ports.

The modprobe uio command, in the setup script, loads the UIO support module into the
kernel plus it loads the igb-uio.ko module into the kernel. The two echo commands,
in the setup script, finish setting up the huge pages one for each socket. If you
only have a single socket system then comment out the second echo command. The last
command is to display the huge TLB setup.

Edit your .bashrc or .profile or .cshrc to add the environment variables.
I am using bash:
`# vi ~/.bashrc`
Add the following lines: Change the $RTE_SDK to the location of the DPDK version
directory. Your SDK directory maybe named differently, but should point to the DPDK
SDK directory.

Make sure you have the Linux kernel headers installed as DPDK requires them to build
the kernel modules. On Ubuntu I run the following:

`# sudo apt-get install linux-headers-`uname -r``

You will need to adjust the version number to match your current kernel version.
If you upgrade your system or kernel version you will need to install the correct
headers and rebuild the RTE_TARGET directory.

``
# sudo apt-get install libpcap-dev

export RTE_SDK=<DPDKinstallDir>
export RTE_TARGET=x86_64-native-linuxapp-gcc
or use clang if you have it installed
export RTE_TARGET=x86_64-native-linuxapp-clang
``
Create the DPDK build tree:
``
# cd $RTE_SDK
# make install T=x86_64-native-linuxapp-gcc -j
``
This above command will create the x86_64-native-linuxapp-gcc directory in the
top level of the current-dkdp directory. The above command will build the basic
DPDK libraries and build tree.

Next we build pktgen:
``
# cd <PktgenInstallDir>
# make
``
You should now have pktgen built and to run pktgen type 'sudo -E ./tools/run.sh', which is a script
to help with the command line options of pktgen. You may need to modify this script for
your system and configuration.
``
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
./app/app/${target}/pktgen -l 4-12 -n 3 --proc-type auto --socket-mem 512,512 --file-prefix pg -b 06:00.0 -b 06:00.1 -b 08:00.0 -b 08:00.1 -b 09:00.0 -b 09:00.1 -b 83:00.1 -- -T -P -m "[5:7].0, [6:8].1, [9:11].2, [10:12].3" -f themes/black-yellow.theme
fi
``
** Note: The '-m NNN' in the DPDK arguments is to have DPDK allocate 512 megs of memory.
 The '--socket-mem 256,156' DPDK command will allocate 256M from each CPU (two in this
 case). Do not use the '-m NNN' and '--socket-mem NN,NN' commands on the same command
 line.

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

The '-n 2' is a required argument for DPDK and denotes the number of memory channels.

*** New setup and run python script with config files ***

Using the new tools/run.py script to setup and run pktgen with different configurations. The configuration files are located in the cfg directory with filenames ending in .cfg.

To use a configuration file;
``
$ ./tools/run.py -s default  # to setup the ports and attach them to DPDK (only needed once per boot)

$ ./tools/run.py default     # Run the default configuration
``
The configuration files are python scritps or a set of variables that run.py uses to initialize and run pktgen.
Here is an example of the default.cfg file:

``
# Setup configuration
setup = {
	'devices': [
		'04:00.0 04:00.1 04:00.2 04:00.3',
		'81:00.0 81:00.1 81:00.2 81:00.3',
		'82:00.0 83:00.0'
		],
		
	'opts': [
		'-b igb_uio'
		]
	}

# Run command and options
run = {
	'dpdk': [
		'-l 8,9-16',
		'-n 4',
		'--proc-type auto',
		'--log-level 7',
		'--socket-mem 2048,2048',
		'--file-prefix pg'
		],
	
	'blacklist': [
		'-b 05:00.0 -b 05:00.1',
		'-b 04:00.0 -b 04:00.1 -b 04:00.2 -b 04:00.3',
		#'-b 81:00.0 -b 81:00.1 -b 81:00.2 -b 81:00.3',
		'-b 82:00.0 -b 83:00.0'
		],
		
	'pktgen': [
		'-T',
		'-P',
		'--crc-strip',
		'-m [9:10].0',
		'-m [11:12].1',
		'-m [13:14].2',
		'-m [15:16].3'
		],
	
	'misc': [
		'-f themes/black-yellow.theme'
		]
	}
``

``
Usage: ./app/pktgen -l CORELIST -n NUM [-m NB] [-r NUM] [-b <domain:bus:devid.func>][--proc-type primary|secondary|auto]

Copyright (c) <2010-2017>, Intel Corporation. All rights reserved. Powered by Intel® DPDK
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

Usage: ./app/app/x86_64-dnet-linuxapp-gcc/pktgen [EAL options] -- [-h] [-P] [-G] [-T] [-f cmd_file] [-l log_file] [-s P:PCAP_file] [-m <string>]
  -s P:file    PCAP packet stream file, 'P' is the port number
  -f filename  Command file (.pkt) to execute or a Lua script (.lua) file
  -l filename  Write log to filename
  -I           use CLI
  -P           Enable PROMISCUOUS mode on all ports
  -g address   Optional IP address and port number default is (localhost:0x5606)
               If -g is used that enable socket support as a server application
  -G           Enable socket support using default server values localhost:0x5606
  -N           Enable NUMA support
  -T           Enable the color output
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
./app/app/${target}/pktgen -l 4-16 -n 3 --proc-type auto --socket-mem 512,512 --file-prefix pg -b 06:00.0 -b 06:00.1 -b 08:00.0 -b 08:00.1 -b 09:00.0 -b 09:00.1 -b 83:00.1 -- -T -P -m "[5:7].0, [6:8].1, [9:11].2, [10:12].3" -f themes/black-yellow.theme
fi

#00:19.0 Ethernet controller: Intel Corporation Ethernet Connection (2) I218-V
#01:00.1 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.2 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.3 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.4 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)

if [ $name == "rkwiles-mini-i7" ]; then
./app/app/${target}/pktgen -l 0-4 -n 3 --proc-type auto --socket-mem 512 --file-prefix pg -- -T -P -m "1.0, 2.1, 3.2, 4.3" -f themes/black-yellow.theme
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
rkwiles@rkwiles-DESK1 (dev):~/.../intel/pktgen$ ./tools/run.sh
./app/app/x86_64-dnet-linuxapp-gcc/pktgen -l 1,2-9,18-19 -n 4 --proc-type auto --log-level 8 --socket-mem 10240,2048 --file-prefix pg --vdev=net_bonding1,mode=4,xmit_policy=l23,slave=0000:81:00.0,slave=0000:81:00.1,slave=0000:81:00.2,slave=0000:81:00.3 -b 05:00.0 -b 05:00.1 -b 82:00.0 -b 83:00.0 -- -I -T -P --crc-strip -m [2:3].0 -m [4:5].1 -m [6:7].2 -m [8:9].3 -m [18:19].8 -f themes/black-yellow.theme

Copyright (c) <2010-2017>, Intel Corporation. All rights reserved. Powered by Intel® DPDK
EAL: Detected lcore 0 as core 0 on socket 0
EAL: Detected lcore 1 as core 1 on socket 0
EAL: Detected lcore 2 as core 2 on socket 0
EAL: Detected lcore 3 as core 3 on socket 0
EAL: Detected lcore 4 as core 4 on socket 0
EAL: Detected lcore 5 as core 8 on socket 0
EAL: Detected lcore 6 as core 9 on socket 0
EAL: Detected lcore 7 as core 10 on socket 0
EAL: Detected lcore 8 as core 11 on socket 0
EAL: Detected lcore 9 as core 16 on socket 0
EAL: Detected lcore 10 as core 17 on socket 0
EAL: Detected lcore 11 as core 18 on socket 0
EAL: Detected lcore 12 as core 19 on socket 0
EAL: Detected lcore 13 as core 20 on socket 0
EAL: Detected lcore 14 as core 24 on socket 0
EAL: Detected lcore 15 as core 25 on socket 0
EAL: Detected lcore 16 as core 26 on socket 0
EAL: Detected lcore 17 as core 27 on socket 0
EAL: Detected lcore 18 as core 0 on socket 1
EAL: Detected lcore 19 as core 1 on socket 1
EAL: Detected lcore 20 as core 2 on socket 1
EAL: Detected lcore 21 as core 3 on socket 1
EAL: Detected lcore 22 as core 4 on socket 1
EAL: Detected lcore 23 as core 8 on socket 1
EAL: Detected lcore 24 as core 9 on socket 1
EAL: Detected lcore 25 as core 10 on socket 1
EAL: Detected lcore 26 as core 11 on socket 1
EAL: Detected lcore 27 as core 16 on socket 1
EAL: Detected lcore 28 as core 17 on socket 1
EAL: Detected lcore 29 as core 18 on socket 1
EAL: Detected lcore 30 as core 19 on socket 1
EAL: Detected lcore 31 as core 20 on socket 1
EAL: Detected lcore 32 as core 24 on socket 1
EAL: Detected lcore 33 as core 25 on socket 1
EAL: Detected lcore 34 as core 26 on socket 1
EAL: Detected lcore 35 as core 27 on socket 1
EAL: Detected lcore 36 as core 0 on socket 0
EAL: Detected lcore 37 as core 1 on socket 0
EAL: Detected lcore 38 as core 2 on socket 0
EAL: Detected lcore 39 as core 3 on socket 0
EAL: Detected lcore 40 as core 4 on socket 0
EAL: Detected lcore 41 as core 8 on socket 0
EAL: Detected lcore 42 as core 9 on socket 0
EAL: Detected lcore 43 as core 10 on socket 0
EAL: Detected lcore 44 as core 11 on socket 0
EAL: Detected lcore 45 as core 16 on socket 0
EAL: Detected lcore 46 as core 17 on socket 0
EAL: Detected lcore 47 as core 18 on socket 0
EAL: Detected lcore 48 as core 19 on socket 0
EAL: Detected lcore 49 as core 20 on socket 0
EAL: Detected lcore 50 as core 24 on socket 0
EAL: Detected lcore 51 as core 25 on socket 0
EAL: Detected lcore 52 as core 26 on socket 0
EAL: Detected lcore 53 as core 27 on socket 0
EAL: Detected lcore 54 as core 0 on socket 1
EAL: Detected lcore 55 as core 1 on socket 1
EAL: Detected lcore 56 as core 2 on socket 1
EAL: Detected lcore 57 as core 3 on socket 1
EAL: Detected lcore 58 as core 4 on socket 1
EAL: Detected lcore 59 as core 8 on socket 1
EAL: Detected lcore 60 as core 9 on socket 1
EAL: Detected lcore 61 as core 10 on socket 1
EAL: Detected lcore 62 as core 11 on socket 1
EAL: Detected lcore 63 as core 16 on socket 1
EAL: Detected lcore 64 as core 17 on socket 1
EAL: Detected lcore 65 as core 18 on socket 1
EAL: Detected lcore 66 as core 19 on socket 1
EAL: Detected lcore 67 as core 20 on socket 1
EAL: Detected lcore 68 as core 24 on socket 1
EAL: Detected lcore 69 as core 25 on socket 1
EAL: Detected lcore 70 as core 26 on socket 1
EAL: Detected lcore 71 as core 27 on socket 1
EAL: Support maximum 128 logical core(s) by configuration.
EAL: Detected 72 lcore(s)
EAL: Auto-detected process type: PRIMARY
EAL: No free hugepages reported in hugepages-1048576kB
EAL: Probing VFIO support...
EAL: Module /sys/module/vfio_pci not found! error 2 (No such file or directory)
EAL: VFIO modules not loaded, skipping VFIO support...
EAL: Module /sys/module/vfio_pci not found! error 2 (No such file or directory)
EAL: Setting up physically contiguous memory...
EAL: Ask a virtual area of 0x2fc00000 bytes
EAL: Virtual area found at 0x7fa7c3600000 (size = 0x2fc00000)
EAL: Ask a virtual area of 0x3cc00000 bytes
EAL: Virtual area found at 0x7fa786800000 (size = 0x3cc00000)
EAL: Ask a virtual area of 0x200000 bytes
EAL: Virtual area found at 0x7fa786400000 (size = 0x200000)
EAL: Ask a virtual area of 0x200000 bytes
EAL: Virtual area found at 0x7fa786000000 (size = 0x200000)
EAL: Ask a virtual area of 0x28f400000 bytes
EAL: Virtual area found at 0x7fa4f6a00000 (size = 0x28f400000)
EAL: Ask a virtual area of 0x200000 bytes
EAL: Virtual area found at 0x7fa4f6600000 (size = 0x200000)
EAL: Ask a virtual area of 0x200000 bytes
EAL: Virtual area found at 0x7fa4f6200000 (size = 0x200000)
EAL: Ask a virtual area of 0x200000 bytes
EAL: Virtual area found at 0x7fa4f5e00000 (size = 0x200000)
EAL: Ask a virtual area of 0x2fc000000 bytes
EAL: Virtual area found at 0x7fa1f9c00000 (size = 0x2fc000000)
EAL: Ask a virtual area of 0x200000 bytes
EAL: Virtual area found at 0x7fa1f9800000 (size = 0x200000)
EAL: Requesting 5120 pages of size 2MB from socket 0
EAL: Requesting 1024 pages of size 2MB from socket 1
EAL: TSC frequency is ~2299980 KHz
EAL: Master lcore 1 is ready (tid=f855a8c0;cpuset=[1])
EAL: lcore 7 is ready (tid=f4200700;cpuset=[7])
EAL: lcore 5 is ready (tid=f5202700;cpuset=[5])
EAL: lcore 2 is ready (tid=f6a05700;cpuset=[2])
EAL: lcore 3 is ready (tid=f6204700;cpuset=[3])
EAL: lcore 19 is ready (tid=847fc700;cpuset=[19])
EAL: lcore 6 is ready (tid=f4a01700;cpuset=[6])
EAL: lcore 9 is ready (tid=857fe700;cpuset=[9])
EAL: lcore 18 is ready (tid=84ffd700;cpuset=[18])
EAL: lcore 4 is ready (tid=f5a03700;cpuset=[4])
EAL: lcore 8 is ready (tid=85fff700;cpuset=[8])
EAL: PCI device 0000:04:00.0 on NUMA socket 0
EAL:   probe driver: 8086:1572 net_i40e
EAL:   PCI memory mapped at 0x7fa7f3200000
EAL:   PCI memory mapped at 0x7fa7f8588000
PMD: eth_i40e_dev_init(): FW 5.0 API 1.5 NVM 05.00.04 eetrack 800024ca
EAL: PCI device 0000:04:00.1 on NUMA socket 0
EAL:   probe driver: 8086:1572 net_i40e
EAL:   PCI memory mapped at 0x7fa7837fc000
EAL:   PCI memory mapped at 0x7fa7f8580000
PMD: eth_i40e_dev_init(): FW 5.0 API 1.5 NVM 05.00.04 eetrack 800024ca
EAL: PCI device 0000:04:00.2 on NUMA socket 0
EAL:   probe driver: 8086:1572 net_i40e
EAL:   PCI memory mapped at 0x7fa782ffc000
EAL:   PCI memory mapped at 0x7fa7f8578000
PMD: eth_i40e_dev_init(): FW 5.0 API 1.5 NVM 05.00.04 eetrack 800024ca
EAL: PCI device 0000:04:00.3 on NUMA socket 0
EAL:   probe driver: 8086:1572 net_i40e
EAL:   PCI memory mapped at 0x7fa7827fc000
EAL:   PCI memory mapped at 0x7fa7f8570000
PMD: eth_i40e_dev_init(): FW 5.0 API 1.5 NVM 05.00.04 eetrack 800024ca
EAL: PCI device 0000:05:00.0 on NUMA socket 0
EAL:   Device is blacklisted, not initializing
EAL: PCI device 0000:05:00.1 on NUMA socket 0
EAL:   Device is blacklisted, not initializing
EAL: PCI device 0000:81:00.0 on NUMA socket 1
EAL:   probe driver: 8086:1572 net_i40e
EAL:   PCI memory mapped at 0x7fa781ffc000
EAL:   PCI memory mapped at 0x7fa7f8568000
PMD: eth_i40e_dev_init(): FW 5.0 API 1.5 NVM 05.00.04 eetrack 800024ca
EAL: PCI device 0000:81:00.1 on NUMA socket 1
EAL:   probe driver: 8086:1572 net_i40e
EAL:   PCI memory mapped at 0x7fa7817fc000
EAL:   PCI memory mapped at 0x7fa7f8560000
PMD: eth_i40e_dev_init(): FW 5.0 API 1.5 NVM 05.00.04 eetrack 800024ca
EAL: PCI device 0000:81:00.2 on NUMA socket 1
EAL:   probe driver: 8086:1572 net_i40e
EAL:   PCI memory mapped at 0x7fa780ffc000
EAL:   PCI memory mapped at 0x7fa7f841f000
PMD: eth_i40e_dev_init(): FW 5.0 API 1.5 NVM 05.00.04 eetrack 800024ca
EAL: PCI device 0000:81:00.3 on NUMA socket 1
EAL:   probe driver: 8086:1572 net_i40e
EAL:   PCI memory mapped at 0x7fa7807fc000
EAL:   PCI memory mapped at 0x7fa7f8417000
PMD: eth_i40e_dev_init(): FW 5.0 API 1.5 NVM 05.00.04 eetrack 800024ca
EAL: PCI device 0000:82:00.0 on NUMA socket 1
EAL:   Device is blacklisted, not initializing
EAL: PCI device 0000:83:00.0 on NUMA socket 1
EAL:   Device is blacklisted, not initializing
EAL: Initializing pmd_bond for net_bonding1
PMD: Using mode 4, it is necessary to do TX burst and RX burst at least every 100ms.
EAL: Create bonded device net_bonding1 on port 8 in mode 4 on socket 0.

   Copyright (c) <2010-2017>, Intel Corporation. All rights reserved.
   Pktgen created by: Keith Wiles -- >>> Powered by Intel® DPDK <<<

Lua 5.3.3  Copyright (C) 1994-2016 Lua.org, PUC-Rio
>>> Packet Burst 32, RX Desc 512, TX Desc 1024, mbufs/port 8192, mbuf cache 1024

=== port to lcore mapping table (# lcores 11) ===
   lcore:    1       2       3       4       5       6       7       8       9      10      11      12      13      14      15      16      17      18      19      Total
port   0: ( D: T) ( 1: 0) ( 0: 1) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) = ( 1: 1)
port   1: ( D: T) ( 0: 0) ( 0: 0) ( 1: 0) ( 0: 1) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) = ( 1: 1)
port   2: ( D: T) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 1: 0) ( 0: 1) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) = ( 1: 1)
port   3: ( D: T) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 1: 0) ( 0: 1) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) = ( 1: 1)
port   8: ( D: T) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 1: 0) ( 0: 1) = ( 1: 1)
Total   : ( 0: 0) ( 1: 0) ( 0: 1) ( 1: 0) ( 0: 1) ( 1: 0) ( 0: 1) ( 1: 0) ( 0: 1) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 0: 0) ( 1: 0) ( 0: 1)
  Display and Timer on lcore 1, rx:tx counts per port/lcore

Configuring 9 ports, MBUF Size 1920, MBUF Cache Size 1024
Lcore:
    2, RX-Only
                RX_cnt( 1): (pid= 0:qid= 0)
    3, TX-Only
                TX_cnt( 1): (pid= 0:qid= 0)
    4, RX-Only
                RX_cnt( 1): (pid= 1:qid= 0)
    5, TX-Only
                TX_cnt( 1): (pid= 1:qid= 0)
    6, RX-Only
                RX_cnt( 1): (pid= 2:qid= 0)
    7, TX-Only
                TX_cnt( 1): (pid= 2:qid= 0)
    8, RX-Only
                RX_cnt( 1): (pid= 3:qid= 0)
    9, TX-Only
                TX_cnt( 1): (pid= 3:qid= 0)
   18, RX-Only
                RX_cnt( 1): (pid= 8:qid= 0)
   19, TX-Only
                TX_cnt( 1): (pid= 8:qid= 0)

Port :
    0, nb_lcores  2, private 0x9a54c0, lcores:  2  3
    1, nb_lcores  2, private 0x9a7820, lcores:  4  5
    2, nb_lcores  2, private 0x9a9b80, lcores:  6  7
    3, nb_lcores  2, private 0x9abee0, lcores:  8  9
    8, nb_lcores  2, private 0x9b6fc0, lcores: 18 19



** Default Info (0000:04:00.0, if_index:0) **
   max_vfs        :   0, min_rx_bufsize    :1024, max_rx_pktlen :  9728
   max_rx_queues  : 192, max_tx_queues     : 192
   max_mac_addrs  :  64, max_hash_mac_addrs:   0, max_vmdq_pools:    32
   rx_offload_capa:  47, tx_offload_capa   :8127, reta_size     :   512, flow_type_rss_offloads:0000000000007ef8
   vmdq_queue_base:  64, vmdq_queue_num    : 128, vmdq_pool_base:     1
** RX Conf **
   pthresh        :   8, hthresh          :   8, wthresh        :     0
   Free Thresh    :  32, Drop Enable      :   0, Deferred Start :     0
** TX Conf **
   pthresh        :  32, hthresh          :   0, wthresh        :     0
   Free Thresh    :  32, RS Thresh        :  32, Deferred Start :     0, TXQ Flags:00000f01

    Create: Default RX  0:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=0, queue=0.
      Set RX queue stats mapping pid 0, q 0, lcore 2


    Create: Default TX  0:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Range TX    0:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Sequence TX 0:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Special TX  0:0  - Memory used (MBUFs   64 x (size 1920 + Hdr 128)) + 192 =    129 KB headroom 128 2176
PMD: i40e_set_tx_function_flag(): Vector tx can be enabled on this txq.

                                                                       Port memory used =  65665 KB
Initialize Port 0 -- TxQ 1, RxQ 1,  Src MAC 3c:fd:fe:9c:5c:d8

** Default Info (0000:04:00.1, if_index:0) **
   max_vfs        :   0, min_rx_bufsize    :1024, max_rx_pktlen :  9728
   max_rx_queues  : 192, max_tx_queues     : 192
   max_mac_addrs  :  64, max_hash_mac_addrs:   0, max_vmdq_pools:    32
   rx_offload_capa:  47, tx_offload_capa   :8127, reta_size     :   512, flow_type_rss_offloads:0000000000007ef8
   vmdq_queue_base:  64, vmdq_queue_num    : 128, vmdq_pool_base:     1
** RX Conf **
   pthresh        :   8, hthresh          :   8, wthresh        :     0
   Free Thresh    :  32, Drop Enable      :   0, Deferred Start :     0
** TX Conf **
   pthresh        :  32, hthresh          :   0, wthresh        :     0
   Free Thresh    :  32, RS Thresh        :  32, Deferred Start :     0, TXQ Flags:00000f01

    Create: Default RX  1:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=1, queue=0.
      Set RX queue stats mapping pid 1, q 0, lcore 4


    Create: Default TX  1:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Range TX    1:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Sequence TX 1:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Special TX  1:0  - Memory used (MBUFs   64 x (size 1920 + Hdr 128)) + 192 =    129 KB headroom 128 2176
PMD: i40e_set_tx_function_flag(): Vector tx can be enabled on this txq.

                                                                       Port memory used =  65665 KB
Initialize Port 1 -- TxQ 1, RxQ 1,  Src MAC 3c:fd:fe:9c:5c:d9

** Default Info (0000:04:00.2, if_index:0) **
   max_vfs        :   0, min_rx_bufsize    :1024, max_rx_pktlen :  9728
   max_rx_queues  : 192, max_tx_queues     : 192
   max_mac_addrs  :  64, max_hash_mac_addrs:   0, max_vmdq_pools:    32
   rx_offload_capa:  47, tx_offload_capa   :8127, reta_size     :   512, flow_type_rss_offloads:0000000000007ef8
   vmdq_queue_base:  64, vmdq_queue_num    : 128, vmdq_pool_base:     1
** RX Conf **
   pthresh        :   8, hthresh          :   8, wthresh        :     0
   Free Thresh    :  32, Drop Enable      :   0, Deferred Start :     0
** TX Conf **
   pthresh        :  32, hthresh          :   0, wthresh        :     0
   Free Thresh    :  32, RS Thresh        :  32, Deferred Start :     0, TXQ Flags:00000f01

    Create: Default RX  2:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=2, queue=0.
      Set RX queue stats mapping pid 2, q 0, lcore 6


    Create: Default TX  2:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Range TX    2:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Sequence TX 2:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Special TX  2:0  - Memory used (MBUFs   64 x (size 1920 + Hdr 128)) + 192 =    129 KB headroom 128 2176
PMD: i40e_set_tx_function_flag(): Vector tx can be enabled on this txq.

                                                                       Port memory used =  65665 KB
Initialize Port 2 -- TxQ 1, RxQ 1,  Src MAC 3c:fd:fe:9c:5c:da

** Default Info (0000:04:00.3, if_index:0) **
   max_vfs        :   0, min_rx_bufsize    :1024, max_rx_pktlen :  9728
   max_rx_queues  : 192, max_tx_queues     : 192
   max_mac_addrs  :  64, max_hash_mac_addrs:   0, max_vmdq_pools:    32
   rx_offload_capa:  47, tx_offload_capa   :8127, reta_size     :   512, flow_type_rss_offloads:0000000000007ef8
   vmdq_queue_base:  64, vmdq_queue_num    : 128, vmdq_pool_base:     1
** RX Conf **
   pthresh        :   8, hthresh          :   8, wthresh        :     0
   Free Thresh    :  32, Drop Enable      :   0, Deferred Start :     0
** TX Conf **
   pthresh        :  32, hthresh          :   0, wthresh        :     0
   Free Thresh    :  32, RS Thresh        :  32, Deferred Start :     0, TXQ Flags:00000f01

    Create: Default RX  3:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=3, queue=0.
      Set RX queue stats mapping pid 3, q 0, lcore 8


    Create: Default TX  3:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Range TX    3:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Sequence TX 3:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Special TX  3:0  - Memory used (MBUFs   64 x (size 1920 + Hdr 128)) + 192 =    129 KB headroom 128 2176
PMD: i40e_set_tx_function_flag(): Vector tx can be enabled on this txq.

                                                                       Port memory used =  65665 KB
Initialize Port 3 -- TxQ 1, RxQ 1,  Src MAC 3c:fd:fe:9c:5c:db

** Default Info (net_bonding1, if_index:0) **
   max_vfs        :   0, min_rx_bufsize    :   0, max_rx_pktlen :  2048
   max_rx_queues  : 128, max_tx_queues     : 512
   max_mac_addrs  :   1, max_hash_mac_addrs:   0, max_vmdq_pools:     0
   rx_offload_capa:   0, tx_offload_capa   :   0, reta_size     :     0, flow_type_rss_offloads:00000000003ffffc
   vmdq_queue_base:   0, vmdq_queue_num    :   0, vmdq_pool_base:     0
** RX Conf **
   pthresh        :   0, hthresh          :   0, wthresh        :     0
   Free Thresh    :   0, Drop Enable      :   0, Deferred Start :     0
** TX Conf **
   pthresh        :   0, hthresh          :   0, wthresh        :     0
   Free Thresh    :   0, RS Thresh        :   0, Deferred Start :     0, TXQ Flags:00000000

    Create: Default RX  8:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
      Set RX queue stats mapping pid 8, q 0, lcore 18


    Create: Default TX  8:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Range TX    8:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Sequence TX 8:0  - Memory used (MBUFs 8192 x (size 1920 + Hdr 128)) + 192 =  16385 KB headroom 128 2176
    Create: Special TX  8:0  - Memory used (MBUFs   64 x (size 1920 + Hdr 128)) + 192 =    129 KB headroom 128 2176

                                                                       Port memory used =  65665 KB
Initialize Port 8 -- TxQ 1, RxQ 1,  Src MAC 3c:fd:fe:9c:5c:b8
                                                                      Total memory used = 328325 KB
PMD: i40e_set_tx_function(): Vector tx finally be used.
PMD: i40e_set_rx_function(): Vector rx enabled, please make sure RX burst size no less than 4 (port=0).
PMD: i40e_set_tx_function(): Vector tx finally be used.
PMD: i40e_set_rx_function(): Vector rx enabled, please make sure RX burst size no less than 4 (port=1).
PMD: i40e_set_tx_function(): Vector tx finally be used.
PMD: i40e_set_rx_function(): Vector rx enabled, please make sure RX burst size no less than 4 (port=2).
PMD: i40e_set_tx_function(): Vector tx finally be used.
PMD: i40e_set_rx_function(): Vector rx enabled, please make sure RX burst size no less than 4 (port=3).
**** nb_rx_queues 2
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=4, queue=0.
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=4, queue=1.
PMD: i40e_set_tx_function(): Xmit tx finally be used.
PMD: i40e_set_rx_function(): Vector rx enabled, please make sure RX burst size no less than 4 (port=4).
**** nb_rx_queues 2
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=5, queue=0.
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=5, queue=1.
PMD: i40e_set_tx_function(): Xmit tx finally be used.
PMD: i40e_set_rx_function(): Vector rx enabled, please make sure RX burst size no less than 4 (port=5).
**** nb_rx_queues 2
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=6, queue=0.
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=6, queue=1.
PMD: i40e_set_tx_function(): Xmit tx finally be used.
PMD: i40e_set_rx_function(): Vector rx enabled, please make sure RX burst size no less than 4 (port=6).
**** nb_rx_queues 2
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=7, queue=0.
PMD: i40e_dev_rx_queue_setup(): Rx Burst Bulk Alloc Preconditions are satisfied. Rx Burst Bulk Alloc function will be used on port=7, queue=1.
PMD: i40e_set_tx_function(): Xmit tx finally be used.
PMD: i40e_set_rx_function(): Vector rx enabled, please make sure RX burst size no less than 4 (port=7).
Port  0: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>
Port  1: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>
Port  2: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>
Port  3: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>
Port  8: Link Up - speed 40000 Mbps - full-duplex <Enable promiscuous mode>


=== Display processing on lcore 1
  RX processing lcore:   2 rx:  1 tx:  0
For RX found 1 port(s) for lcore 2
  TX processing lcore:   3 rx:  0 tx:  1
For TX found 1 port(s) for lcore 3
  RX processing lcore:   4 rx:  1 tx:  0
For RX found 1 port(s) for lcore 4
  TX processing lcore:   5 rx:  0 tx:  1
For TX found 1 port(s) for lcore 5
  RX processing lcore:   6 rx:  1 tx:  0
For RX found 1 port(s) for lcore 6
  TX processing lcore:   7 rx:  0 tx:  1
For TX found 1 port(s) for lcore 7
  RX processing lcore:   8 rx:  1 tx:  0
For RX found 1 port(s) for lcore 8
  TX processing lcore:   9 rx:  0 tx:  1
For TX found 1 port(s) for lcore 9
  RX processing lcore:  18 rx:  1 tx:  0
For RX found 1 port(s) for lcore 18
  TX processing lcore:  19 rx:  0 tx:  1
For TX found 1 port(s) for lcore 19
``
------------------
/ Ports 0-3 of 9   <Main Page>  Copyright (c) <2010-2016>, Intel Corporation
  Flags:Port      :   P--------------:0   P--------------:1   P--------------:2   P--------------:3
Link State        :       <UP-10000-FD>       <UP-10000-FD>       <UP-10000-FD>       <UP-10000-FD>     ----TotalRate----
Pkts/s Max/Rx     :                 3/1                 3/1                 3/1                 3/1                  12/4
       Max/Tx     :                 0/0                 0/0                 0/0                 0/0                  12/4
MBits/s Rx/Tx     :                 0/0                 0/0                 0/0                 0/0                   0/0
Broadcast         :                   0                   0                   0                   0
Multicast         :                   0                   0                   0                   0
  64 Bytes        :                   0                   0                   0                   0
  65-127          :                   0                   0                   0                   0
  128-255         :                  22                  22                  22                  22
  256-511         :                   0                   0                   0                   0
  512-1023        :                   0                   0                   0                   0
  1024-1518       :                   0                   0                   0                   0
Runts/Jumbos      :                 0/0                 0/0                 0/0                 0/0
Errors Rx/Tx      :                 0/0                 0/0                 0/0                 0/0
Total Rx Pkts     :                  21                  21                  21                  21
      Tx Pkts     :                   0                   0                   0                   0
      Rx MBs      :                   0                   0                   0                   0
      Tx MBs      :                   0                   0                   0                   0
ARP/ICMP Pkts     :                 0/0                 0/0                 0/0                 0/0
                  :
Pattern Type      :             abcd...             abcd...             abcd...             abcd...
Tx Count/% Rate   :       Forever /100%       Forever /100%       Forever /100%       Forever /100%
PktSize/Tx Burst  :           64 /   32           64 /   32           64 /   32           64 /   32
Src/Dest Port     :         1234 / 5678         1234 / 5678         1234 / 5678         1234 / 5678
Pkt Type:VLAN ID  :     IPv4 / TCP:0001     IPv4 / TCP:0001     IPv4 / TCP:0001     IPv4 / TCP:0001
Dst  IP Address   :         192.168.1.1         192.168.0.1         192.168.3.1         192.168.2.1
Src  IP Address   :      192.168.0.1/24      192.168.1.1/24      192.168.2.1/24      192.168.3.1/24
Dst MAC Address   :   3c:fd:fe:9c:5c:d9   3c:fd:fe:9c:5c:d8   3c:fd:fe:9c:5c:db   3c:fd:fe:9c:5c:da
Src MAC Address   :   3c:fd:fe:9c:5c:d8   3c:fd:fe:9c:5c:d9   3c:fd:fe:9c:5c:da   3c:fd:fe:9c:5c:db
VendID/PCI Addr   :   8086:1572/04:00.0   8086:1572/04:00.1   8086:1572/04:00.2   8086:1572/04:00.3

-- Pktgen Ver: 3.2.0 (DPDK 17.05.0-rc0)  Powered by Intel® DPDK ---------------















Pktgen:/> quit
$
``
------------------------------------------------------------------------
``
   *** Pktgen Help information ***

page <pages>                       - Show the port pages or configuration or sequence page
     [0-7]                         - Page of different ports
     main                          - Display page zero
     range                         - Display the range packet page
     config | cfg                  - Display the configuration page
     pcap                          - Display the pcap page
     cpu                           - Display some information about the CPU system
     next                          - Display next page of PCAP packets.
     sequence | seq                - sequence will display a set of packets for a given port
                                     Note: use the 'port <number>' to display a new port sequence
     rnd                           - Display the random bitfields to packets for a given port
                                     Note: use the 'port <number>' to display a new port sequence
     log                           - Display the log messages page
     latency                       - Display the latency page
     stats                         - Display physical ports stats for all ports

enable|disable <portlist> <features>
    Feature - process              - Enable or Disable processing of ARP/ICMP/IPv4/IPv6 packets
              mpls                 - Enable/disable sending MPLS entry in packets
              qinq                 - Enable/disable sending Q-in-Q header in packets
              gre                  - Enable/disable GRE support
              gre_eth              - Enable/disable GRE with Ethernet frame payload
              vlan                 - Enable/disable VLAN tagging
              garp                 - Enable or Disable GARP packet processing and update MAC address
              random               - Enable/disable Random packet support
              latency              - Enable/disable latency testing
              pcap                 - Enable or Disable sending pcap packets on a portlist
              blink                - Blink LED on port(s)
              rx_tap               - Enable/Disable RX Tap support
              tx_tap               - Enable/Disable TX Tap support
              icmp                 - Enable/Disable sending ICMP packets
              range                - Enable or Disable the given portlist for sending a range of packets
              capture              - Enable/disable packet capturing on a portlist

enable|disable screen              - Enable/disable updating the screen and unlock/lock window
               mac_from_arp        - Enable/disable MAC address from ARP packet
off                                - screen off shortcut
on                                 - screen on shortcut

set <portlist> <type> value        - Set a few port values
  <portlist>                       - a list of ports as 2,4,6-9,12 or the word 'all'
  <type>         count             - number of packets to transmit
                 size              - size of the packet to transmit
                 rate              - Packet rate in percentage
                 burst             - number of packets in a burst
                 sport             - Source port number for TCP
                 dport             - Destination port number for TCP
                 prime             - Set the number of packets to send on prime command
                 seq_cnt           - Set the number of packet in the sequence to send
                 dump              - Dump the next <value> received packets to the screen
                 vlanid            - Set the VLAN ID value for the portlist
                 jitter            - Set the jitter threshold in micro-seconds
                 mpls entry        - Set the MPLS entry for the portlist (must be specified in hex)
                 gre_key           - Set the GRE key
                 mac dst|src <etheraddr> - Set MAC addresses 00:11:22:33:44:55
                                     You can use 0011:2233:4455 format as well
set <portlist> jitter <value>      - Set the jitter value
set <portlist> type ipv4|ipv6|vlan|arp - Set the packet type to IPv4 or IPv6 or VLAN
set <portlist> proto udp|tcp|icmp  - Set the packet protocol to UDP or TCP or ICMP per port
set <portlist> pattern <type>      - Set the fill pattern type
     type - abc                    - Default pattern of abc string
            none                   - No fill pattern, maybe random data
            zero                   - Fill of zero bytes
            user                   - User supplied string of max 16 bytes
set <portlist> user pattern <string> - A 16 byte string, must set 'pattern user' command
set <portlist> [src|dst] ip ipaddr - Set IP addresses
set ports_per_page <value>         - Set ports per page value 1 - 6
set <portlist> qinqids <id1> <id2> - Set the Q-in-Q ID's for the portlist
set <portlist> rnd <idx> <off> <mask> - Set random mask for all transmitted packets from portlist
                                     idx: random mask slot
                                     off: offset in packets, where to apply mask
                                     mask: up to 32 bit long mask specification (empty to disable):
                                       0: bit will be 0
                                       1: bit will be 1
                                       .: bit will be ignored (original value is retained)
                                       X: bit will get random value

  -- Setup the packet range values --
range <portlist> [dst|src] mac <SMMI> <etheraddr> - Set destination/source MAC address
range <portlist> [src|dst] ip <SMMI> <ipaddr> - Set source IP start address
range <portlist> proto [tcp|udp]              - Set the IP protocol type (alias range.proto)
range <portlist> [src|dst] port <SMMI> <value> - Set UDP/TCP source/dest port number
range <portlist> vlan <SMMI> <value>          - Set vlan id start address
range <portlist> size <SMMI> <value>          - Set pkt size start address
range <portlist> teid <SMMI> <value>          - Set TEID value
range <portlist> mpls entry <hex-value>       - Set MPLS entry value
range <portlist> qinq index <val1> <val2>     - Set QinQ index values
range <portlist> gre key <value>              - Set GRE key value
                 - SMMI = start|min|max|inc (start, minimum, maximum, increment)

sequence <seq#> <portlist> dst <Mac> src <Mac> dst <IP> src <IP> sport <val> dport <val> ipv4|ipv6 udp|tcp|icmp vlan <val> pktsize <val> [teid <val>]
sequence <seq#> <portlist> <dst-Mac> <src-Mac> <dst-IP> <src-IP> <sport> <dport> ipv4|ipv6 udp|tcp|icmp <vlanid> <pktsize> [<teid>]
                                   - Set the sequence packet information, make sure the src-IP
                                     has the netmask value eg 1.2.3.4/24

pcap show                          - Show PCAP information
pcap index                         - Move the PCAP file index to the given packet number,  0 - rewind, -1 - end of file
pcap filter <portlist> <string>    - PCAP filter string to filter packets on receive


start <portlist>                   - Start transmitting packets
stop <portlist>                    - Stop transmitting packets
stp                                - Stop all ports from transmitting
str                                - Start all ports transmitting
start <portlist> prime             - Transmit packets on each port listed. See set prime command above
start <portlist> arp <type>        - Send a ARP type packet
    type - request | gratuitous | req | grat

debug l2p                          - Dump out internal lcore to port mapping
debug tx_debug                     - Enable tx debug output
debug mempool <portlist> <type>    - Dump out the mempool info for a given type
debug pdump <portlist>             - Hex dump the first packet to be sent, single packet mode only

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

       Flags: P---------------- - Promiscuous mode enabled
               E                - ICMP Echo enabled
                A               - Send ARP Request flag
                 G              - Send Gratuitous ARP flag
                  C             - TX Cleanup flag
                   p            - PCAP enabled flag
                    S           - Send Sequence packets enabled
                     R          - Send Range packets enabled
                      D         - DPI Scanning enabled (If Enabled)
                       I        - Process packets on input enabled
                        *       - Using TAP interface for this port can be [-rt*]
                         L      - Send Latency packets                          V     - Send VLAN ID tag
                          M     - Send MPLS header
                          Q     - Send Q-in-Q tags
                           g    - Process GARP packets
                            g   - Perform GRE with IPv4 payload
                            G   - Perform GRE with Ethernet payload
                             C  - Capture received packets
                              R - Random bitfield(s) are applied
Notes: <state>       - Use enable|disable or on|off to set the state.
       <portlist>    - a list of ports (no spaces) as 2,4,6-9,12 or 3-5,8 or 5 or the word 'all'
       Color best seen on a black background for now
       To see a set of example Lua commands see the files in wr-examples/pktgen/test
``
---------------------------------------------------------------------------
``
\                  <Sequence Page>  Copyright (c) <2010-2016>, Intel Corporation
Port:  0, Sequence Count:  4 of 16                                                                             GTPu
  Seq:            Dst MAC           Src MAC          Dst IP            Src IP    Port S/D Protocol:VLAN  Size  TEID
*   0:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
*   1:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
*   2:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
*   3:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0













-- Pktgen Ver: 3.2.0 (DPDK 17.05.0-rc0)  Powered by Intel® DPDK ---------------
























Pktgen:/> set all seq_cnt 4
Pktgen:/>
``
---------------------------------------------------------------------------
``
| Port 0           <Random bitfield Page>  Copyright (c) <2010-2016>, Intel Corporation
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
-- Pktgen Ver: 3.2.0 (DPDK 17.05.0-rc0)  Powered by Intel® DPDK ---------------











Pktgen:/>
``
---------------------------------------------------------------------------
-- Example command lines.
``
./app/pktgen -l 0-8 -n 3 --proc-type auto --socket-mem 256,256 -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -s 0:pcap/large.pcap
./app/pktgen -l 0-4 -n 3 --proc-type auto --socket-mem 128,128 --file-prefix pg -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -s 0:pcap/test1.pcap -s 1:pcap/large.pcap
./app/pktgen -l 0-4 -n 3 --proc-type auto --socket-mem 128,128 --file-prefix pg -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -s 0:pcap/test1.pcap -s 1:pcap/large.pcap
./app/pktgen -l 1-3 -n 3 --proc-type auto --socket-mem 128,128 --file-prefix pg -- -P -m "2.0, 3.1"
./app/pktgen -l 0-8 -n 3 --proc-type auto --socket-mem 256,256 -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3"
``

A command line passing in a pktgen/test/set_seq.pkt file to help initialize pktgen with some
default values and configurations. You can also replace the filename using the '-f' command
with a Lua script file ending in .lua instead of .pkt. BTW, if the filename ends in anything
other then .lua it is treated as a .pkt file.

`./app/pktgen -l 0-4 -n 3 --proc-type auto --socket-mem 128,128 -- -P -m "[1:3].0, [2:4].1" -f test/set_seq.pkt`

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
./app/pktgen -l 0-4 -n 3 --proc-type auto --socket-mem 128,128 -- -P -m "[1:3].0, [2:4].1" -f test/set_seq.lua`

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

Lua Vesrion: Lua 5.2
Pktgen Version : 2.9.x
Pktgen Copyright : Copyright (c) `<2010-2016>`, Intel Corporation
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
Lua Version: Lua 5.2
Pktgen Version : 2.9.x
Pktgen Copyright : Copyright (c) `<2010-2016>`, Intel Corporation
Pktgen Authors : Keith Wiles @ Intel Corporation

Hello World!!!!
<Control-D>
------------------

You can also just send it commands via echo.

-----------------
$ echo "f,e = loadfile('test/hello-world.lua'); f();"| socat - TCP4:172.25.40.163:22022
Lua Version: Lua 5.2
Pktgen Version : 2.9.x
Pktgen Copyright : Copyright (c) `<2010-2016>`, Intel Corporation
Pktgen Authors : Keith Wiles @ Intel Corporation

Hello World!!!!
----------------------

Keith Wiles @ Intel Corporation
