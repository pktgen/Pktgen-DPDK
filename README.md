Pktgen version 2.9.1 using DPDK-2.0.0
=====================================

**Pktgen is a traffic generator powered by Intel's DPDK at 10Gbit wire rate traffic with 64 byte frames.**

**Sounds like 'Packet-Gen'**

**=== Modifications ===**
 - 2.9.1   - Fix up the sequeue help to remove vlan option with ipv4/ipv6
 - 2.9.0   - Update to DPDK 2.0.0 and Lua 5.3.0 with README update.
 - 2.8.6   - Fix argument for rte_mempool_create, which caused a crash.
 - 2.8.5   - Fix compat problem with latest Pktgen and DPDK 1.8.0
 - 2.8.4   - Minor updates for comments.
 - 2.8.3   - Updated the Makefiles to use rte.extXYZ.mk files.
             Updated the code to build with DPDK 2.0.0-rc1 as some function prototype changed.
 - 2.8.2   - Fix bug in pktgen_main_receive routine not using the correct port number.
 - 2.8.1   - Add a new docs directory using Sphinx format and update version numbers.
 - 2.8.0   - Update to release 1.8.0 of DPDK.
 - 2.7.7   - Update Lua to 5.2.3 and fixed setting vlan ID on single ports plus added new Lua functions
			 New Lua functions are pktgen.portCount() and pktgen.totalPorts() portCount() is the number of
			 port used by Pktgen and totalPorts() is the total number seen by DPDK.
 - 2.7.6   - Update code from dpdk.org version of Pktgen, which hopefull fixes the send foreve problem.
 - 2.7.5   - Update to latest dpdk.org and move scrn to lib directory with name changes.
 - 2.7.4   - Removed old printf_info() calls for printf_status() calls.
 - 2.7.3   - Fixed race condition with updating the TX count value with a small count.
 - 2.7.1   - Add a command line option '-T' to enable themes and set themes off by default.
 - 2.7.0   - Update to DPDK 1.7.0, Note: DPDK 1.7.0 changed how ports are detected and blacklisted,
             which means the port index is now different. You will need to blacklist or whitelist ports
             with the DPDK '-b' or '--pci-blacklist or --pci-whitelist' options. Pktgen does not blacklist
             ports anymore.
           - Moved pktgen to the examples directory plus removed the libwr_* from the lib directory
           - Pktgen now supports ANSI color terminals only the main screen ATM, but more later.
           - Best viewed on a black background display, unless you want to change it with the new theme commands.
           - More supported generator types, checkout the help screens.
 - 2.6.8   - Fixed a transmit problem when count is set to one. Plus increase the link down delays.
 - 2.6.7   - Add more support for GRE packets, log support and more testing code.
 - 2.6.6   - Fix compile problem when not SSE4.2 instructions are not supported. Allowing QEMU and other
             systems to build and run. Also added a patch to take into account huge reserved pages.
 - 2.6.5   - Added support for logging packet information.
 - 2.6.4   - It consists of 3 commits: improvements to the pktgen-random.c unit tests,
             the real CentOS compilation fixes and a small update to tap.{c,h} so they
             are identical to those from zorgnax/libtap on github.
 - 2.6.3   - Add a delay when traffic stops to make sure all packets are sent.
             Remove the `rte_hash_crc.h` include in wr_pcap.c file.
 - 2.6.2   - Fixup GRE and ARP problems
 - 2.6.1   - Add random bits support and more cleanup
 - 2.6.0   - Split up the code for testing to be added later
 - 2.5.2   - Remove extra ethertypes.h file.
 - 2.5.1   - Added the following updates.
           - Implement-Rx-packet-dump-functionality
           - Add-packet-capture-functionality
           - Add-MPLS-functionality
           - Add-Q-in-Q-802.11ad-functionality
           - Add-GRE-header-generation
           - Fix-UDP-TCP-ICMP-protocol-selection
           - Add-ARP-protocol
 - 2.5.0   - Update to DPDK 1.6.0 plus a few bug fixes.
 - 2.4.1   - Fixed a bug in range packets when 'inc' value is zero use start values.
 - 2.4.0   - Add support for TX tap packets. Change 'tap' command to rxtap and txtap.
 - 2.3.4   - Minor update to help eliminate RX errors and be able to receive at wire rate.
 - 2.3.3   - Update to minor release 1.5.2
 - 2.3.2   - Fixed VLAN detection problem in ARP and special GARP support.
 - 2.3.1   - Getting closer to line rate tx speed.
 - 2.3.0   - Now supports the VLAN encapsulated packets for ARP replies
             Also added a special GARP processing to update the destination MAC
             address to help support a special request for failover support.
             Added support for DPDK 1.5.1
 - 2.2.7   - Updated the code to handle multiple TX queues per port.
 - 2.2.6   - Fixed a crash if the port is not up with link status
 - 2.2.5   - Remove the flow control code as some systems it does not work.
 - 2.2.4   - Fix the `inet_h64tom and inet_mtoh64` functions to account for endianness
 - 2.2.3   - range packet fixes for packet size and source/destination mac
 - 2.2.2   - Minor performance changes for receive packet performance.
 - 2.2.1   - Change MAC address from XXXX:XXXX:XXXX to XX:XX:XX:XX:XX:XX format
             Fixed Pktgen to allow packet changes without having to restart the tool.
 - 2.2.0   - Update to DPDK 1.5.0
 
**=====================**

Please look at the 3rd party PDF for license information.

---
**Copyright &copy; \<2010-2015\>, Intel Corporation All rights reserved.**
 
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

---
**Copyright &copy; \<2010-2015\>, Wind River Systems, Inc.**

 Redistribution and use in source and binary forms, with or without modification, are
 permitted provided that the following conditions are met:

 1) Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

 2) Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation and/or
    other materials provided with the distribution.

 3) Neither the name of Wind River Systems nor the names of its contributors may be
    used to endorse or promote products derived from this software without specific
    prior written permission.

 4) The screens displayed by the application must contain the copyright notice as defined
    above and can not be removed without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Pktgen: Created 2010 by Keith Wiles @ windriver.com, now at Intel.com
 ---

**======================== README.pktgen file ==============================**

                          *** Pktgen ****
          Copyright &copy \<2010-2015\>, Wind River Systems, Inc.

README for setting up Pktgen with DPDK on Ubuntu 10.04 to 13.10 desktop, it
should work on most Linux systems as long as the kernel has hugeTLB page support.

*** Note: Tested with Ubuntu 13.10 and up to 14.04 kernel version
    Linux 3.5.0-25-generic #39-Ubuntu SMP Mon Feb 25 18:26:58 UTC 2013 x86_64

I am using Ubuntu 13.10 x86_64 (64 bit support) for running pktgen/DPDK on a
Westmere Dual socket board running at 2.4GHz with 12GB of ram 6GB per socket.
The current kernel version is 3.5.0-25 (as of 2013-03-18) support, but should
work on just about any new Linux kernel version.

To get hugeTLB page support your Linux kernel must be at least 2.6.33 and in the
DPDK documents it talks about how you can upgrade your Linux kernel.

Here is another document on how to upgrade your Linux kernel.
Ubuntu 10.04 is 2.6.32 by default so upgraded to kernel 2.6.34 using this HOWTO:
http://usablesoftware.wordpress.com/2010/05/26/switch-to-a-newer-kernel-in-ubuntu-10-04/

The pktgen output display needs 132 columns and about 42 lines to display
correctly. I am using an xterm of 132x42, but you can have a larger display
and maybe a bit smaller. If you are displaying more then 4-6 ports then you
will need a wider display. The pktgen allows you to view a set ports if they
do not all fit on the screen at one time via the 'page' command in pktgen.
Type 'help' at the 'pktgen>' prompt to see the complete pktgen command line
commands. Pktgen uses VT100 control codes or escape codes to display the screens,
which means your terminal must support VT100. The Hyperterminal in windows is not
going to work for Pktgen as it has a few problems with VT100 codes.

The pktgen program as built can send up to 16 packets per port in a sequence
and you can configure a port using the 'seq' pktgen command. A script file
can be loaded from the shell command line via the -f option and you can 'load'
a script file from within pktgen as well.

In the BIOS make sure the HPET High Precision Event Timer is enabled. Also
make sure hyper-threading is enabled.

** NOTE **
  On a 10GB NIC if the transceivers are not attached the screen updates will go
  very slow.

Get the DPDK and pktgen source code from github.com or dpdk.org repo via:
```
# cd <InstallDir>
# git clone git://dpdk.org/dpdk.git
```
```
# cd <InstallDir>
# git clone git://dpdk.org/pktgen-dpdk.git
```
** Note **
  The site dpdk.org you must also pull down DPDK repo as well. git://dpdk.org/dpdk

Will create a directory called Pktgen-DPDK in the current directory location. Using
this clone command you will get DPDK and pktgen source files.

Make sure you have HUGE TLB support in the kernel with the following commands:
```
# grep -i huge /boot/config-2.6.35-24-generic 
CONFIG_HUGETLBFS=y
CONFIG_HUGETLB_PAGE=y

# grep -i huge /proc/meminfo
HugePages_Total:      128
HugePages_Free:       128
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
```
NOTE: The values in Total and Free maybe different until you reboot the machine.

Two files in /etc must be setup to support huge TLBs. If you do not have
hugeTLB support then you most likely need a newer kernel.
```
# vi /etc/sysctl.conf
Add to the bottom of the file:
vm.nr_hugepages=256
```
If you need more or less hugeTLB pages then you can change the value to a
number you need it to be. In some cases pktgen needs a fair number of pages
and making it too small will effect performance or pktgen will terminate on
startup looking for more pages.
```
# vi /etc/fstab
Add to the bottom of the file:
huge /mnt/huge hugetlbfs defaults 0 0

# mkdir /mnt/huge
# chmod 777 /mnt/huge
```
Reboot your machine as the huge pages must be setup just after boot to make
sure you have contiguous memory for the 2Meg pages.

** Note: If you startup Eclipse or WR Workbench before starting pktgen the first
   time after reboot, pktgen will fail to load because it can not get all of the
   huge pages as eclipse has consumed some of the huge pages. If you did start eclipse
   or WR Workbench then you need to close that application first. 

This is my current machine you will have a few different numbers depending on 
how your system was booted and if you had hugeTLB support enabled.

At the ${RTE_SDK}/examples/pktgen level directory we have the 'setup' script,
which needs to be run as root once per boot. The script contains a commands to setup
the environment.

Make sure you run the setup script as root via 'sudo -E ./setup'. The setup script
is a bash script and tries to setup the system correctly, but you may have to 
change the script to match your number of huge pages you configured above.

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

`# sudo apt-get install linux-headers-3.5.0-32-generic`

You will need to adjust the version number to match your current kernel version.
If you upgrade your system or kernel version you will need to install the correct
headers and rebuild the RTE_TARGET directory.

```
# sudo apt-get install libpcap-dev

export RTE_SDK=<DPDKinstallDir>
export RTE_TARGET=x86_64-native-linuxapp-gcc
or use clang if you have it installed
export RTE_TARGET=x86_64-native-linuxapp-clang
```
Create the DPDK build tree:
```
# cd $RTE_SDK
# make install T=x86_64-native-linuxapp-gcc
```
This above command will create the x86_64-native-linuxapp-gcc directory in the
top level of the current-dkdp directory. The above command will build the basic
DPDK libraries and build tree.

Next we build pktgen:
```
# cd <PktgenInstallDir>
# make
```
You should now have pktgen built and to run pktgen type 'sudo -E ./doit', which is a script
to help with the command line options of pktgen. You may need to modify this script for
your system and configuration.
```
# cat doit
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
./app/app/${target}/pktgen -c 1fff0 -n 3 --proc-type auto --socket-mem 512,512 --file-prefix pg -b 06:00.0 -b 06:00.1 -b 08:00.0 -b 08:00.1 -b 09:00.0 -b 09:00.1 -b 83:00.1 -- -T -P -m "[5:7].0, [6:8].1, [9:11].2, [10:12].3" -f themes/black-yellow.theme
fi
```
** Note: The '-m NNN' in the DPDK arguments is to have DPDK allocate 512 megs of memory.
   The '--socket-mem 256,156' DPDK command will allocate 256M from each CPU (two in this
   case). Do not use the '-m NNN' and '--socket-mem NN,NN' commands on the same command
   line.
   
The pktgen program follows the same format as a standard DPDK linuxapp, meaning
the first set of arguments '-c 1f' are the standard DPDK arguments. This option
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

```
Usage: ./app/pktgen -c COREMASK -n NUM [-m NB] [-r NUM] [-b <domain:bus:devid.func>][--proc-type primary|secondary|auto]

EAL options:                                                                                                                        
  -c COREMASK  : A hexadecimal bitmask of cores to run on                                                                           
  -n NUM       : Number of memory channels                                                                                          
  -v           : Display version information on startup                                                                             
  -d LIB.so    : add driver (can be used multiple times)                                                                            
  -m MB        : memory to allocate (see also --socket-mem)                                                                         
  -r NUM       : force number of memory ranks (don't detect)                                                                        
  --xen-dom0 : support application running on Xen Domain0 without hugetlbfs                                                         
  --syslog     : set syslog facility                                                                                                
  --socket-mem : memory to allocate on specific                                                                                     
                 sockets (use comma separated values)                                                                               
  --huge-dir   : directory where hugetlbfs is mounted                                                                               
  --proc-type  : type of this process                                                                                               
  --file-prefix: prefix for hugepage filenames                                                                                      
  --pci-blacklist, -b: add a PCI device in black list.                                                                              
               Prevent EAL from using this PCI device. The argument                                                                 
               format is `<domain:bus:devid.func>`.                                                                                   
  --pci-whitelist, -w: add a PCI device in white list.                                                                              
               Only use the specified PCI devices. The argument format                                                              
               is <[domain:]bus:devid.func>. This option can be present                                                             
               several times (once per device).                                                                                     
               [NOTE: PCI whitelist cannot be used with -b option]                                                                  
  --vdev: add a virtual device.                                                                                                     
               The argument format is `<driver><id>[,key=val,...]`                                                                    
               (ex: --vdev=eth_pcap0,iface=eth2).                                                                                   
  --vmware-tsc-map: use VMware TSC map instead of native RDTSC                                                                      
  --base-virtaddr: specify base virtual address                                                                                     
  --vfio-intr: specify desired interrupt mode for VFIO (legacy|msi|msix)                                                            
  --create-uio-dev: create /dev/uioX (usually done by hotplug)                                                                      

EAL options for DEBUG use only:                                                                                                     
  --no-huge  : use malloc instead of hugetlbfs                                                                                      
  --no-pci   : disable pci                                                                                                          
  --no-hpet  : disable hpet                                                                                                         
  --no-shconf: no shared config (mmap'd files)

===== Application Usage =====

Usage: ./app/pktgen [EAL options] -- [-h] [-P] [-G] [-f cmd_file] [-l log_file] [-s P:PCAP_file] [-m <string>]
  -s P:file    PCAP packet stream file, 'P' is the port number
  -f filename  Command file (.pkt) to execute or a Lua script (.lua) file
  -l filename  Write log to filename
  -P           Enable PROMISCUOUS mode on all ports
  -g address   Optional IP address and port number default is (localhost:0x5606)
               If -g is used that enable socket support as a server application
  -G           Enable socket support using default server values localhost:0x5606 
  -N           Enable NUMA support
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
** Note: To determine the Ethernet ports in your system use 'lspci | grep Ethernet' to 
   get a list of all ports in the system. Some ports may not be useable by DPDK/Pktgen.
   The first port listed is bit 0 or least signification bit in the '-c' mask.
   Another method is to compile the test_pmd example and run './test_pmd -c 0x3 -n 2'
   command to list out the ports DPDK is able to use.
```
A new feature for pktgen and DPDK is to run multiple instances of pktgen. This
allows the developer to share ports on the same machine.

------------- doit script ----------------
```
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
./app/app/${target}/pktgen -c 1fff0 -n 3 --proc-type auto --socket-mem 512,512 --file-prefix pg -b 06:00.0 -b 06:00.1 -b 08:00.0 -b 08:00.1 -b 09:00.0 -b 09:00.1 -b 83:00.1 -- -T -P -m "[5:7].0, [6:8].1, [9:11].2, [10:12].3" -f themes/black-yellow.theme
fi

#00:19.0 Ethernet controller: Intel Corporation Ethernet Connection (2) I218-V
#01:00.1 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.2 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.3 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)
#01:00.4 Ethernet controller: Intel Corporation DH8900CC Series Gigabit Network Connection (rev 10)

if [ $name == "rkwiles-mini-i7" ]; then
./app/app/${target}/pktgen -c 1f -n 3 --proc-type auto --socket-mem 512 --file-prefix pg -- -T -P -m "1.0, 2.1, 3.2, 4.3" -f themes/black-yellow.theme
fi
```
------------- doit script ----------------

------------- setup script ----------------
```
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
fi
if [ $name == "rkwilesmini-i7" ]; then
$sdk/tools/dpdk_nic_bind.py -b igb_uio 01:00.1 01:00.2 01:00.3 01:00.4
fi
$sdk/tools/dpdk_nic_bind.py --status
lspci |grep Ether
```
------------- setup script ----------------

If you have run pktgen before then remove the files in /mnt/huge/* before
running the new version.

Running the doit script produces output as follows, but maybe different on your
system configuration.
```
[22:20][keithw@keithw-S5520HC:pktgen(master)]$ sudo -E ./doit
------------------------------------------------------------------------
-----------------------
  
     BSD LICENSE
  
     Copyright(c) 2010-2015 Intel Corporation. All rights reserved.
     All rights reserved.
  
     Redistribution and use in source and binary forms, with or without
     modification, are permitted provided that the following conditions
     are met:
  
       * Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in
         the documentation and/or other materials provided with the
         distribution.
       * Neither the name of Intel Corporation nor the names of its
         contributors may be used to endorse or promote products derived
         from this software without specific prior written permission.
  
     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
     A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
     OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
     LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
-----------------------
    Copyright (c) <2010-2015>, Wind River Systems, Inc.

     Redistribution and use in source and binary forms, with or without modification, are
     permitted provided that the following conditions are met:
  
       1) Redistributions of source code must retain the above copyright notice,
          this list of conditions and the following disclaimer.
  
       2) Redistributions in binary form must reproduce the above copyright notice,
          this list of conditions and the following disclaimer in the documentation and/or
          other materials provided with the distribution.
  
       3) Neither the name of Wind River Systems nor the names of its contributors may be
          used to endorse or promote products derived from this software without specific
          prior written permission.
  
       4) The screens displayed by the application must contain the copyright notice as defined
          above and can not be removed without specific prior written permission.
  
     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
     AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
     ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
     LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
     DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
     SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
     OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
     USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
  Pktgen created by: Keith Wiles -- >>> Powered by IntelÂ® DPDK <<<
-----------------------
EAL: Detected lcore 0 as core 0 on socket 0
EAL: Detected lcore 1 as core 1 on socket 0                                                                                         
EAL: Detected lcore 2 as core 2 on socket 0                                                                                         
EAL: Detected lcore 3 as core 3 on socket 0                                                                                         
EAL: Detected lcore 4 as core 4 on socket 0                                                                                         
EAL: Detected lcore 5 as core 8 on socket 0                                                                                         
EAL: Detected lcore 6 as core 9 on socket 0                                                                                         
EAL: Detected lcore 7 as core 10 on socket 0                                                                                        
EAL: Detected lcore 8 as core 11 on socket 0                                                                                        
EAL: Detected lcore 9 as core 12 on socket 0                                                                                        
EAL: Detected lcore 10 as core 0 on socket 1                                                                                        
EAL: Detected lcore 11 as core 1 on socket 1                                                                                        
EAL: Detected lcore 12 as core 2 on socket 1                                                                                        
EAL: Detected lcore 13 as core 3 on socket 1                                                                                        
EAL: Detected lcore 14 as core 4 on socket 1                                                                                        
EAL: Detected lcore 15 as core 8 on socket 1                                                                                        
EAL: Detected lcore 16 as core 9 on socket 1                                                                                        
EAL: Detected lcore 17 as core 10 on socket 1                                                                                       
EAL: Detected lcore 18 as core 11 on socket 1                                                                                       
EAL: Detected lcore 19 as core 12 on socket 1                                                                                       
EAL: Detected lcore 20 as core 0 on socket 0                                                                                        
EAL: Detected lcore 21 as core 1 on socket 0                                                                                        
EAL: Detected lcore 22 as core 2 on socket 0                                                                                        
EAL: Detected lcore 23 as core 3 on socket 0                                                                                        
EAL: Detected lcore 24 as core 4 on socket 0                                                                                        
EAL: Detected lcore 25 as core 8 on socket 0                                                                                        
EAL: Detected lcore 26 as core 9 on socket 0                                                                                        
EAL: Detected lcore 27 as core 10 on socket 0                                                                                       
EAL: Detected lcore 28 as core 11 on socket 0                                                                                       
EAL: Detected lcore 29 as core 12 on socket 0                                                                                       
EAL: Detected lcore 30 as core 0 on socket 1                                                                                        
EAL: Detected lcore 31 as core 1 on socket 1                                                                                        
EAL: Detected lcore 32 as core 2 on socket 1                                                                                        
EAL: Detected lcore 33 as core 3 on socket 1                                                                                        
EAL: Detected lcore 34 as core 4 on socket 1                                                                                        
EAL: Detected lcore 35 as core 8 on socket 1                                                                                        
EAL: Detected lcore 36 as core 9 on socket 1                                                                                        
EAL: Detected lcore 37 as core 10 on socket 1                                                                                       
EAL: Detected lcore 38 as core 11 on socket 1                                                                                       
EAL: Detected lcore 39 as core 12 on socket 1                                                                                       
EAL: Support maximum 64 logical core(s) by configuration.                                                                           
EAL: Detected 40 lcore(s)                                                                                                           
EAL: Auto-detected process type: PRIMARY                                                                                            
EAL:   cannot open VFIO container, error 2 (No such file or directory)                                                              
EAL: VFIO support could not be initialized                                                                                          
EAL: Setting up memory...                                                                                                           
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3c8600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3c8200000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3c7e00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3c7a00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3c7600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3c7200000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3c6e00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x400000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3c6800000 (size = 0x400000)                                                                         
EAL: Ask a virtual area of 0x400000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3c6200000 (size = 0x400000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3c5e00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0xe200000 bytes                                                                                          
EAL: Virtual area found at 0x7fa3b7a00000 (size = 0xe200000)                                                                        
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3b7600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3b7200000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa3b6e00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x2f800000 bytes                                                                                         
EAL: Virtual area found at 0x7fa387400000 (size = 0x2f800000)                                                                       
EAL: Ask a virtual area of 0x400000 bytes                                                                                           
EAL: Virtual area found at 0x7fa386e00000 (size = 0x400000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa386a00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x600000 bytes                                                                                           
EAL: Virtual area found at 0x7fa386200000 (size = 0x600000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa385e00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa385a00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa385600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa385200000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa384e00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa384a00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa384600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x400000 bytes                                                                                           
EAL: Virtual area found at 0x7fa384000000 (size = 0x400000)                                                                         
EAL: Ask a virtual area of 0x400000 bytes                                                                                           
EAL: Virtual area found at 0x7fa383a00000 (size = 0x400000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa383600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa383200000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa382e00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                               
EAL: Virtual area found at 0x7fa382a00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa382600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa382200000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa381e00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa381a00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x400000 bytes                                                                                           
EAL: Virtual area found at 0x7fa381400000 (size = 0x400000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa381000000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa380c00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa380800000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa380400000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa380000000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x400000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37fa00000 (size = 0x400000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37f600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37f200000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37ee00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37ea00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37e600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37e200000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37de00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37da00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37d600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37d200000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0xa00000 bytes                                                                                           
EAL: Virtual area found at 0x7fa37c600000 (size = 0xa00000)                                                                         
EAL: Ask a virtual area of 0x47a00000 bytes                                                                                         
EAL: Virtual area found at 0x7fa334a00000 (size = 0x47a00000)                                                                       
EAL: Ask a virtual area of 0xf600000 bytes                                                                                          
EAL: Virtual area found at 0x7fa325200000 (size = 0xf600000)                                                                        
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa324e00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa324a00000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa324600000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa324200000 (size = 0x200000)                                                                         
EAL: Ask a virtual area of 0x200000 bytes                                                                                           
EAL: Virtual area found at 0x7fa323e00000 (size = 0x200000)                                                                         
EAL: Requesting 128 pages of size 2MB from socket 0                                                                                 
EAL: Requesting 128 pages of size 2MB from socket 1                                                                                 
EAL: TSC frequency is ~2793266 KHz                                                                                                  
EAL: Master core 0 is ready (tid=c9f8c880)                                                                                          
EAL: Core 4 is ready (tid=b4dfb700)                                                                                                 
EAL: Core 3 is ready (tid=b55fc700)                                                                                                 
EAL: Core 2 is ready (tid=b5dfd700)                                                                                                 
EAL: Core 1 is ready (tid=b65fe700)                                                                                                 
EAL: PCI device 0000:03:00.0 on NUMA socket 0                                                                                       
EAL:   probe driver: 8086:10fb rte_ixgbe_pmd                                                                                        
EAL:   Device is blacklisted, not initializing                                                                                      
EAL: PCI device 0000:03:00.1 on NUMA socket 0                                                                                       
EAL:   probe driver: 8086:10fb rte_ixgbe_pmd                                                                                        
EAL:   Device is blacklisted, not initializing                                                                                      
EAL: PCI device 0000:07:00.0 on NUMA socket 0                                                                                       
EAL:   probe driver: 8086:1521 rte_igb_pmd                                                                                          
EAL:   0000:07:00.0 not managed by UIO driver, skipping                                                                             
EAL: PCI device 0000:07:00.1 on NUMA socket 0                                                                                       
EAL:   probe driver: 8086:1521 rte_igb_pmd                                                                                          
EAL:   0000:07:00.1 not managed by UIO driver, skipping                                                                             
EAL: PCI device 0000:83:00.0 on NUMA socket 1                                                                                       
EAL:   probe driver: 8086:10fb rte_ixgbe_pmd                                                                                        
EAL:   PCI memory mapped at 0x7fa3c9ed6000                                                                                          
EAL:   PCI memory mapped at 0x7fa3c9fac000                                                                                          
EAL: PCI device 0000:83:00.1 on NUMA socket 1                                                                                       
EAL:   probe driver: 8086:10fb rte_ixgbe_pmd                                                                                        
EAL:   PCI memory mapped at 0x7fa3c9e56000                                                                                          
EAL:   PCI memory mapped at 0x7fa3c9fa8000                                                                                          
EAL: PCI device 0000:85:00.0 on NUMA socket 1                                                                                       
EAL:   probe driver: 8086:10fb rte_ixgbe_pmd                                                                                        
EAL:   PCI memory mapped at 0x7fa3c9dd6000                                                                                          
EAL:   PCI memory mapped at 0x7fa3c9fa4000                                                                                          
EAL: PCI device 0000:85:00.1 on NUMA socket 1                                                                                       
EAL:   probe driver: 8086:10fb rte_ixgbe_pmd                                                                                        
EAL:   PCI memory mapped at 0x7fa3c8de1000                                                                                          
EAL:   PCI memory mapped at 0x7fa3c9fa0000                                                                                          
EAL: PCI device 0000:88:00.0 on NUMA socket 1                                                                                       
EAL:   probe driver: 8086:10fb rte_ixgbe_pmd                                                                                        
EAL:   PCI memory mapped at 0x7fa3c8d61000                                                                                          
EAL:   PCI memory mapped at 0x7fa3c9f9c000                                                                                          
EAL: PCI device 0000:88:00.1 on NUMA socket 1                                                                                       
EAL:   probe driver: 8086:10fb rte_ixgbe_pmd                                                                                        
EAL:   PCI memory mapped at 0x7fa3c8ce1000                                                                                          
EAL:   PCI memory mapped at 0x7fa3c9f98000                                                                                          
[1:3].0          = lcores(rx 0000000000000002, tx 0000000000000008) ports(rx 0000000000000001, tx 0000000000000001)                 
[2:4].1          = lcores(rx 0000000000000004, tx 0000000000000010) ports(rx 0000000000000002, tx 0000000000000002)                 
EAL: PCI device 0000:03:00.0 on NUMA socket 0                                                                                       
EAL:   probe driver: 8086:10fb rte_ixgbe_pmd                                                                                        
EAL:   Device is blacklisted, not initializing                                                                                      
EAL: PCI device 0000:03:00.1 on NUMA socket 0                                                                                       
EAL:   probe driver: 8086:10fb rte_ixgbe_pmd                                                                                        
EAL:   Device is blacklisted, not initializing                                                                                      
EAL: PCI device 0000:07:00.0 on NUMA socket 0                                                                                       
EAL:   probe driver: 8086:1521 rte_igb_pmd                                                                                          
EAL:   0000:07:00.0 not managed by UIO driver, skipping                                                                             
EAL: PCI device 0000:07:00.1 on NUMA socket 0                                                                                       
EAL:   probe driver: 8086:1521 rte_igb_pmd                                                                                          
EAL:   0000:07:00.1 not managed by UIO driver, skipping                                                                             
Lua 5.2.1  Copyright (C) 1994-2012 Lua.org, PUC-Rio                                                                                 
>>> Packet Burst 16, RX Desc 256, TX Desc 256, mbufs/port 2048, mbuf cache 256                                                      

=== port to lcore mapping table (# lcores 5) ===                                                                                    
   lcore:     0     1     2     3     4                                                                                             
port   0:  D: T  1: 0  0: 0  0: 1  0: 0 =  1: 1                                                                                     
port   1:  D: T  0: 0  1: 0  0: 0  0: 1 =  1: 1                                                                                     
Total   :  0: 0  1: 0  1: 0  0: 1  0: 1                                                                                             
    Display and Timer on lcore 0, rx:tx counts per port/lcore                                                                       

Configuring 6 ports, MBUF Size 1984, MBUF Cache Size 256                                                                            
Lcore:                                                                                                                              
    1, type  RX , rx_cnt  1, tx_cnt  0 private (nil), RX (pid:qid): ( 0: 0) , TX (pid:qid):                                         
    2, type  RX , rx_cnt  1, tx_cnt  0 private (nil), RX (pid:qid): ( 1: 0) , TX (pid:qid):                                         
    3, type  TX , rx_cnt  0, tx_cnt  1 private (nil), RX (pid:qid): , TX (pid:qid): ( 0: 0)                                         
    4, type  TX , rx_cnt  0, tx_cnt  1 private (nil), RX (pid:qid): , TX (pid:qid): ( 1: 0)                                         

Port :                                                                                                                              
    0, nb_lcores  2, private 0x7d08d8, lcores:  1  3                                                                                
    1, nb_lcores  2, private 0x7d1c48, lcores:  2  4                                                                                


Initialize Port 0 -- TxQ 1, RxQ 1,  Src MAC 90:e2:ba:5a:f7:90                                                                       
    Create: Default RX  0:0  - Memory used (MBUFs 2048 x (size 1984 + Hdr 64)) + 790720 =   4869 KB                                 

    Create: Default TX  0:0  - Memory used (MBUFs 2048 x (size 1984 + Hdr 64)) + 790720 =   4869 KB                                 
    Create: Range TX    0:0  - Memory used (MBUFs 2048 x (size 1984 + Hdr 64)) + 790720 =   4869 KB                                 
    Create: Sequence TX 0:0  - Memory used (MBUFs 2048 x (size 1984 + Hdr 64)) + 790720 =   4869 KB                                 
    Create: Special TX  0:0  - Memory used (MBUFs   64 x (size 1984 + Hdr 64)) + 790720 =    901 KB                                 

                                                                       Port memory used =  20373 KB                                 
Initialize Port 1 -- TxQ 1, RxQ 1,  Src MAC 90:e2:ba:5a:f7:91                                                                       
    Create: Default RX  1:0  - Memory used (MBUFs 2048 x (size 1984 + Hdr 64)) + 790720 =   4869 KB                                 

    Create: Default TX  1:0  - Memory used (MBUFs 2048 x (size 1984 + Hdr 64)) + 790720 =   4869 KB                                 
    Create: Range TX    1:0  - Memory used (MBUFs 2048 x (size 1984 + Hdr 64)) + 790720 =   4869 KB                                 
    Create: Sequence TX 1:0  - Memory used (MBUFs 2048 x (size 1984 + Hdr 64)) + 790720 =   4869 KB                                 
    Create: Special TX  1:0  - Memory used (MBUFs   64 x (size 1984 + Hdr 64)) + 790720 =    901 KB                                 

                                                                       Port memory used =  20373 KB                                 
                                                                      Total memory used =  40746 KB                                 
Port  0: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>                                                         
Port  1: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>                                                         


=== Display processing on lcore 0                                                                                                   
=== RX processing on lcore  1, rxcnt 1, port/qid, 0/0                                                                               
=== RX processing on lcore  2, rxcnt 1, port/qid, 1/0                                                                               
=== TX processing on lcore  3, txcnt 1, port/qid, 0/0                                                                               
=== TX processing on lcore  4, txcnt 1, port/qid, 1/0

      #     #
      #  #  #     #    #    #  #####
      #  #  #     #    ##   #  #    #
      #  #  #     #    # #  #  #    #
      #  #  #     #    #  # #  #    #
      #  #  #     #    #   ##  #    #
       ## ##      #    #    #  #####

      ######
      #     #     #    #    #  ######  #####
      #     #     #    #    #  #       #    #
      ######      #    #    #  #####   #    #
      #   #       #    #    #  #       #####
      #    #      #     #  #   #       #   #
      #     #     #      ##    ######  #    #

       #####
      #     #   #   #   ####    #####  ######  #    #   ####
      #          # #   #          #    #       ##  ##  #
       #####      #     ####      #    #####   # ## #   ####
            #     #         #     #    #       #    #       #
      #     #     #    #    #     #    #       #    #  #    #
       #####      #     ####      #    ######  #    #   ####

               Copyright (c) <2010-2015>, Wind River Systems, Inc.
                     >>> Pktgen is Powered by IntelÂ® DPDK <<<

---------------------

               Copyright (c) <2010-2015>, Wind River Systems, Inc.

         Redistribution and use in source and binary forms, with or without modification, are
         permitted provided that the following conditions are met:

           1) Redistributions of source code must retain the above copyright notice,
              this list of conditions and the following disclaimer.

           2) Redistributions in binary form must reproduce the above copyright notice,
              this list of conditions and the following disclaimer in the documentation and/or
              other materials provided with the distribution.

           3) Neither the name of Wind River Systems nor the names of its contributors may be
              used to endorse or promote products derived from this software without specific
              prior written permission.

           4) The screens displayed by the application must contain the copyright notice as defined
              above and can not be removed without specific prior written permission.

         THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
         AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
         ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
         LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
         DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
         SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
         CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
         OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
         USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

               Pktgen created by Keith Wiles -- >>> Powered by IntelÂ® DPDK <<<
```
------------------
```
- Ports 0-3 of 6   ** Main Page **  Copyright (c) <2010-2015>, Wind River Systems, Inc. Powered by IntelÂ® DPDK
  Flags:Port    :   P-------------:0   P-------------:1
Link State      :      <UP-10000-FD>      <UP-10000-FD>                                          ---TotalRate---
Pkts/s  Rx      :                  0                  0                                                        0
        Tx      :                  0                  0                                                        0
MBits/s Rx/Tx   :                0/0                0/0                                                      0/0
Broadcast       :                  0                  0
Multicast       :                  0                  0
  64 Bytes      :                  0                  0
  65-127        :                  0                  0
  128-255       :                  0                  0
  256-511       :                  0                  0
  512-1023      :                  0                  0
  1024-1518     :                  0                  0
Runts/Jumbos    :                0/0                0/0
Errors Rx/Tx    :                0/0                0/0
Total Rx Pkts   :                  0                  0
      Tx Pkts   :                  0                  0
      Rx MBs    :                  0                  0
      Tx MBs    :                  0                  0
ARP/ICMP Pkts   :                0/0                0/0
                :
Tx Count/% Rate :       Forever/100%       Forever/100%
PktSize/Tx Burst:              64/16              64/16
Src/Dest Port   :          1234/5678          1234/5678
Pkt Type:VLAN ID:      IPv4/TCP:0001      IPv4/TCP:0001
Dst  IP Address :        192.168.1.1        192.168.0.1
Src  IP Address :     192.168.0.1/24     192.168.1.1/24
Dst MAC Address :  90:e2:ba:5a:f7:91  90:e2:ba:5a:f7:90
Src MAC Address :  90:e2:ba:5a:f7:90  90:e2:ba:5a:f7:91
-- Pktgen Ver:2.9.1(DPDK-2.0.0) -------------------------------------------------------------------------------------















Pktgen> quit
$
```
------------------------------------------------------------------------
```
   *** Help Information for Pktgen ***         Copyright (c) <2010-2015>, Wind River Systems, Inc.

set <portlist> <xxx> value         - Set a few port values
  <portlist>                       - a list of ports as 2,4,6-9,12 or the word 'all'
  <xxx>          count             - number of packets to transmit
                 size              - size of the packet to transmit
                 rate              - Packet rate in percentage
                 burst             - number of packets in a burst
                 sport             - Source port number for TCP
                 dport             - Destination port number for TCP
                 prime             - Set the number of packets to send on prime command
                 seqCnt            - Set the number of packet in the sequence to send
                 dump              - Dump the next <value> received packets to the screen
seq <seq#> <portlist> dst-Mac src-Mac dst-IP src-IP sport dport ipv4|ipv6 udp|tcp|icmp vid pktsize
                                   - Set the sequence packet information, make sure the src-IP
                                     has the netmask value eg 1.2.3.4/24
save <path-to-file>                - Save a configuration file using the filename
load <path-to-file>                - Load a command/script file from the given path
ppp [1-6]                          - Set the number of ports displayed per page
icmp.echo <portlist> <state>       - Enable/disable ICMP echo responses per port
send arp req|grat <portlist>       - Send a ARP request or gratuitous ARP on a set of ports
set mac <portlist> etheraddr       - Set MAC addresses 00:11:22:33:44:55
                                     You can use 0011:2233:4455 format as well
mac_from_arp <state>               - Set the option to get MAC from ARP request
proto udp|tcp|icmp <portlist>      - Set the packet protocol to UDP or TCP or ICMP per port
type ipv4|ipv6|vlan <portlist>     - Set the packet type to IPv4 or IPv6 or VLAN
set ip src|dst <portlist> ipaddr   - Set IP addresses
geometry <geom>                    - Set the display geometry Columns by Rows (ColxRow)
capture <portlist> <state>         - Enable/disable packet capturing on a portlist
rxtap <portlist> <state>           - Enable/disable Rx tap interface support pg_rxtapN
txtap <portlist> <state>           - Enable/disable Tx tap interface support pg_txtapN
vlan <portlist> <state>            - Enable/disable sending VLAN ID in packets
vlanid <portlist> <vlanid>         - Set the VLAN ID for the portlist
mpls <portlist> <state>            - Enable/disable sending MPLS entry in packets
mpls_entry <portlist> <entry>      - Set the MPLS entry for the portlist (must be specified in hex)
qinq <portlist> <state>            - Enable/disable sending Q-in-Q header in packets
qinqids <portlist> <id1> <id2>     - Set the Q-in-Q ID's for the portlist
gre <portlist> <state>             - Enable/disable GRE with IPv4 payload
gre_eth <portlist> <state>         - Enable/disable GRE with Ethernet frame payload
gre_key <portlist> <state>         - Set the GRE key
pcap <portlist> <state>            - Enable or Disable sending pcap packets on a portlist
pcap.show                          - Show the PCAP information
pcap.index                         - Move the PCAP file index to the given packet number,  0 - rewind, -1 - end of file
pcap.filter <portlist> <string>    - PCAP filter string to filter packets on receive
script <filename>                  - Execute the Lua script code in file (www.lua.org).
ping4 <portlist>                   - Send a IPv4 ICMP echo request on the given portlist
page [0-7]|main|range|config|seq|pcap|next|cpu|rnd- Show the port pages or configuration or sequence page
     [0-7]                         - Page of different ports
     main                          - Display page zero
     range                         - Display the range packet page
     config                        - Display the configuration page (not used)
     pcap                          - Display the pcap page
     cpu                           - Display some information about the CPU system
     next                          - Display next page of PCAP packets.
     sequence | seq                - sequence will display a set of packets for a given port
                                     Note: use the 'port <number>' to display a new port sequence
     rnd                           - Display the random bitfields to packets for a given port
                                     Note: use the 'port <number>' to display a new port sequence
     log                           - Display the log messages page
port <number>                      - Sets the sequence of packets to display for a given port
process <portlist> <state>         - Enable or Disable processing of ARP/ICMP/IPv4/IPv6 packets
garp <portlist> <state>            - Enable or Disable GARP packet processing and update MAC address
blink <portlist> <state>           - Blink the link led on the given port list
rnd <portlist> <idx> <off> <mask>  - Set random mask for all transmitted packets from portlist
                                     idx: random mask slot
                                     off: offset in packets, where to apply mask
                                     mask: up to 32 bit long mask specification (empty to disable):
                                       0: bit will be 0
                                       1: bit will be 1
                                       .: bit will be ignored (original value is retained)
                                       X: bit will get random value
theme <state>                      - Enable or Disable the theme
theme <item> <fg> <bg> <attr>      - Set color for item with fg/bg color and attribute value
theme.show                         - List the item strings, colors and attributes to the items
theme.save <filename>              - Save the current color theme to a file
start <portlist>                   - Start transmitting packets
stop <portlist>                    - Stop transmitting packets
stp                                - Stop all ports from transmitting
str                                - Start all ports transmitting
screen stop|start                  - stop/start updating the screen and unlock/lock window
off                                - screen off shortcut
on                                 - screen on shortcut
prime <portlist>                   - Transmit N packets on each port listed. See set prime command above
delay milliseconds                 - Wait a number of milliseconds for scripting commands
sleep seconds                      - Wait a number of seconds for scripting commands
dev.list                           - Show the device whitelist/blacklist/Virtual
pci.list                           - Show all the PCI devices
clear <portlist>                   - Clear the statistics
clr                                - Clear all Statistices
cls                                - Clear the screen
reset <portlist>                   - Reset the configuration to the default
rst                                - Reset the configuration for all ports
help                               - Display this help message
quit                               - Quit the Pktgen program
  -- Setup the packet range values --
dst.mac start <portlist> etheraddr - Set destination MAC address start
src.mac start <portlist> etheraddr - Set source MAC address start
src.ip start <portlist> ipaddr     - Set source IP start address
src.ip min <portlist> ipaddr       - Set source IP minimum address
src.ip max <portlist> ipaddr       - Set source IP maximum address
src.ip inc <portlist> ipaddr       - Set source IP increment address
dst.ip start <portlist> ipaddr     - Set destination IP start address
dst.ip min <portlist> ipaddr       - Set destination IP minimum address
dst.ip max <portlist> ipaddr       - Set destination IP maximum address
dst.ip inc <portlist> ipaddr       - Set destination IP increment address
src.port start <portlist> value    - Set source port start address
src.port min <portlist> value      - Set source port minimum address
src.port max <portlist> value      - Set source port maximum address
src.port inc <portlist> value      - Set source port increment address
dst.port start <portlist> value    - Set source port start address
dst.port min <portlist> value      - Set source port minimum address
dst.port max <portlist> value      - Set source port maximum address
dst.port inc <portlist> value      - Set source port increment address
vlan.id start <portlist> value     - Set vlan id start address
vlan.id min <portlist> value       - Set vlan id minimum address
vlan.id max <portlist> value       - Set vlan id maximum address
vlan.id inc <portlist> value       - Set vlan id increment address
pkt.size start <portlist> value    - Set vlan id start address
pkt.size min <portlist> value      - Set vlan id minimum address
pkt.size max <portlist> value      - Set vlan id maximum address
pkt.size inc <portlist> value      - Set vlan id increment address
range <portlist> <state>           - Enable or Disable the given portlist for sending a range of packets
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
                        T      - Using TAP interface for this port
                         V     - Send VLAN ID tag
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

```
---------------------------------------------------------------------------
```
\  Port 0 of 4    ** PCAP Page **   Copyright (c) <2010-2015>, Wind River Systems, Inc., Powered by Intel® DPDK
Port: 0, PCAP Count: 0 of 9716, skipped 0
  Seq            Dst MAC           Src MAC            Dst IP              Src IP    Port S/D  Protocol:VLAN Size-FCS
    0:    0014:2273:0a68    000f:ea34:177e    192.168.119.23     192.168.117.213    43934/53  IPv4/UDP:   0   73
    1:    000f:ea34:177e    0014:2273:0a68   192.168.117.213      192.168.119.23    53/43934  IPv4/UDP:   0  445
    2:    0013:720b:515b    000f:ea34:177e    209.131.36.158     192.168.117.213    34918/80  IPv4/UDP:   0   74
    3:    000f:ea34:177e    0013:720b:515b   192.168.117.213      209.131.36.158    80/34918  IPv4/UDP:   0   78
    4:    0013:720b:515b    000f:ea34:177e    209.131.36.158     192.168.117.213    34918/80  IPv4/UDP:   0   66
    5:    0013:720b:515b    000f:ea34:177e    209.131.36.158     192.168.117.213    34918/80  IPv4/UDP:   0  842
    6:    000f:ea34:177e    0013:720b:515b   192.168.117.213      209.131.36.158    80/34918  IPv4/UDP:   0   66
    7:    000f:ea34:177e    0013:720b:515b   192.168.117.213      209.131.36.158    80/34918  IPv4/UDP:   0 1155
    8:    0013:720b:515b    000f:ea34:177e    209.131.36.158     192.168.117.213    34918/80  IPv4/UDP:   0   66
    9:    0014:2273:0a68    000f:ea34:177e    192.168.119.23     192.168.117.213    47919/53  IPv4/UDP:   0   72
   10:    0013:720b:515b    000f:ea34:177e    209.131.36.158     192.168.117.213    34918/80  IPv4/UDP:   0   66
   11:    000f:ea34:177e    0013:720b:515b   192.168.117.213      209.131.36.158    80/34918  IPv4/UDP:   0   66
   12:    000f:ea34:177e    0013:720b:515b   192.168.117.213      209.131.36.158    80/34918  IPv4/UDP:   0 1090
   13:    0013:720b:515b    000f:ea34:177e    209.131.36.158     192.168.117.213    34918/80  IPv4/UDP:   0   54
   14:    000f:ea34:177e    0014:2273:0a68   192.168.117.213      192.168.119.23    53/47919  IPv4/UDP:   0  352
   15:    0013:720b:515b    000f:ea34:177e     203.84.217.32     192.168.117.213    40202/80  IPv4/UDP:   0   74
   16:    000f:ea34:177e    0013:720b:515b   192.168.117.213       203.84.217.32    80/40202  IPv4/UDP:   0   78
   17:    0013:720b:515b    000f:ea34:177e     203.84.217.32     192.168.117.213    40202/80  IPv4/UDP:   0   66
   18:    0013:720b:515b    000f:ea34:177e     203.84.217.32     192.168.117.213    40202/80  IPv4/UDP:   0  846
   19:    000f:ea34:177e    0013:720b:515b   192.168.117.213       203.84.217.32    80/40202  IPv4/UDP:   0   66
   20:    000f:ea34:177e    0013:720b:515b   192.168.117.213       203.84.217.32    80/40202  IPv4/UDP:   0 1486
   21:    0013:720b:515b    000f:ea34:177e     203.84.217.32     192.168.117.213    40202/80  IPv4/UDP:   0   66
   22:    000f:ea34:177e    0013:720b:515b   192.168.117.213       203.84.217.32    80/40202  IPv4/UDP:   0  104
   23:    0013:720b:515b    000f:ea34:177e     203.84.217.32     192.168.117.213    40202/80  IPv4/UDP:   0   66
   24:    000f:ea34:177e    0013:720b:515b   192.168.117.213       203.84.217.32    80/40202  IPv4/UDP:   0 1466

- Pktgen Ver:2.9.0(DPDK-2.0.0) --------------------------------------------------------------------------------------










pktgen> quit
```
------------------------------------------------------------------------------------------
-- Example command lines.
```
./app/pktgen -c 1ff -n 3 --proc-type auto --socket-mem 256,256 -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -s 0:pcap/large.pcap
./app/pktgen -c 1f -n 3 --proc-type auto --socket-mem 128,128 --file-prefix pg -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -s 0:pcap/test1.pcap -s 1:pcap/large.pcap
./app/pktgen -c 1f -n 3 --proc-type auto --socket-mem 128,128 --file-prefix pg -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3" -s 0:pcap/test1.pcap -s 1:pcap/large.pcap
./app/pktgen -c e -n 3 --proc-type auto --socket-mem 128,128 --file-prefix pg -- -P -m "2.0, 3.1"
./app/pktgen -c 1ff -n 3 --proc-type auto --socket-mem 256,256 -- -P -m "[1:3].0, [2:4].1, [5:7].2, [6:8].3"
```

A command line passing in a pktgen/test/set_seq.pkt file to help initialize pktgen with some
default values and configurations. You can also replace the filename using the '-f' command
with a Lua script file ending in .lua instead of .pkt. BTW, if the filename ends in anything
other then .lua it is treated as a .pkt file.
 
`./app/pktgen -c 1f -n 3 --proc-type auto --socket-mem 128,128 -- -P -m "[1:3].0, [2:4].1" -f test/set_seq.pkt`

-- test/set_seq.pkt
```
seq 0 all 0000:4455:6677 0000:1234:5678 10.11.0.1 10.10.0.1/16 5 6 ipv4 udp 1 128
set all seqCnt 1
```
The set_seq.pkt command file can also be one of the files in pktgen/test directory,
which are Lua based scripts instead of command line scripts as in set_seq.pkt file.

-- test/set_seq.lua
The Lua version is easier to remember the layout of the agruments if you want to
use that one instead of set_seq.pkt file.

```
./app/pktgen -c 1f -n 3 --proc-type auto --socket-mem 128,128 -- -P -m "[1:3].0, [2:4].1" -f test/set_seq.lua`

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
```
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

Lua Vesrion      : Lua 5.2
Pktgen Version   : 2.8.1
Pktgen Copyright : Copyright (c) `<2010-2015>`, Wind River Systems, Inc.
Pktgen Authors   : Keith Wiles @ Wind River Systems

Hello World!!!!
--------

Here is the program I sent Pktgen:

$ cat test/hello-world.lua 
package.path = package.path ..";?.lua;test/?.lua;app/?.lua;"

printf("Lua Vesrion      : %s\n", pktgen.info.Lua_Version);
printf("Pktgen Version   : %s\n", pktgen.info.Pktgen_Version);
printf("Pktgen Copyright : %s\n", pktgen.info.Pktgen_Copyright);
printf("Pktgen Authors   : %s\n", pktgen.info.Pktgen_Authors);

printf("\nHello World!!!!\n");
-----------

Here is a command from my Mac Book pro laptop, which loads a file from the local
disk where Pktgen is running and then we execute the file with 'f()'.

------------------
$ socat READLINE TCP4:172.25.40.163:22022
f,e = loadfile("test/hello-world.lua")
f()
Lua Version      : Lua 5.2
Pktgen Version   : 2.8.1
Pktgen Copyright : Copyright (c) `<2010-2015>`, Wind River Systems, Inc.
Pktgen Authors   : Keith Wiles @ Wind River Systems

Hello World!!!!
<Control-D>
------------------

You can also just send it commands via echo.

-----------------
$ echo "f,e = loadfile('test/hello-world.lua'); f();"| socat - TCP4:172.25.40.163:22022
Lua Version      : Lua 5.2
Pktgen Version   : 2.8.1
Pktgen Copyright : Copyright (c) `<2010-2015>`, Wind River Systems, Inc.
Pktgen Authors   : Keith Wiles @ Wind River Systems

Hello World!!!!
----------------------

Keith Wiles @ Wind River Systems
