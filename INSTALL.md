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

# Installation

## Environment

INSTALL for setting up Pktgen with DPDK on Ubuntu 10.04 to 16.10 desktop and CentOS,
it should work on most Linux systems as long as the kernel has hugeTLB page support.

**Note:**

Tested with Ubuntu 13.10 and up to 18.04 kernel versions
Linux 4.15.0-39-generic #42-Ubuntu SMP Tue Oct 23 15:48:01 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux

Tested with CentOS 7.5.1804
Linux 3.10.0-862.14.4.el7 Sat 24 Nov 04:07:00 UTC 2018 x86_64

I am using Ubuntu 18.04 x86_64 (64 bit support) for running Pktgen-DPDK on a
Broadwell Dual socket board running at 2.4GHz with 32GB of ram 16GB per socket.
The current kernel version is 4.15.0-39-generic (as of 2018-11-26) support, but should
work on just about any new Linux kernel version.

To get hugeTLB page support your Linux kernel must be at least 2.6.33 and in the
DPDK documents it talks about how you can upgrade your Linux kernel.

The pktgen output display needs 132 columns and about 42 lines to display
correctly. I am using an xterm of 132x42, but you can have a larger display
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

In the BIOS make sure the hyper-threading is enabled.

## Install prerequisites
Make sure you have the Linux kernel headers installed as DPDK requires them to build
the kernel modules. On Ubuntu I run the following:

    // On Debian based systems
    # sudo apt-get install linux-headers-$(uname -r) libpcap-dev

Lua is required to be installed on Ubuntu it is 'apt-get install liblua-dev' on some systems they mayb use liblua5.3-dev or some version appened to the name.

    // On Red Hat systems
    # yum install -y make gcc kernel-devel elfutils-libelf-devel patch libasan libpcap-devel numactl-devel glibc-devel readline-devel pciutils git epel-release gcc-c++ autoconf automake libtool wget python ncurses-devel zlib-devel libjpeg-devel openssl-devel sqlite-devel libcurl-devel libxml2-devel libidn-devel e2fsprogs-devel pcre-devel speex-devel ldns-devel libedit-devel libyuv-devel opus-devel libvpx-devel unbound-devel libuuid-devel libsndfile-devel

On Red Hat-based systems the additional dependencies are due to the fact that you
will have to build LUA manually.

**Note:** On Red Hat proper you will need to install epel manually as it is not available
in the repositories.

**Note:** You will need to adjust the version number to match your current kernel
version. If you upgrade your system or kernel version you will need to install the
correct headers and rebuild the RTE_TARGET directory.

Be aware that most often there are mismatches when you update the kernel, but have
not yet rebooted. We suggested updating and then running the above if you haven't
already.

## Getting the Code

Get the DPDK and pktgen source code from github.com or dpdk.org repo via:

    # cd <InstallDir>
    # git clone git://dpdk.org/dpdk.git
    # git clone git://dpdk.org/pktgen-dpdk.git

**NOTE:** While on the dpdk site you must also pull down the dpdk SDK itself. git://dpdk.org/dpdk

## Use the Quick Start Script to Install dpdk Toolkit and Configure Host

You can setup all relevant settings manually, but I recommend you use the quick
start setup script detailed [here](https://doc.dpdk.org/guides/linux_gsg/quick_start.html).

## Manually Configure dpdk Toolkit

If you did not use the quick start script to configure huge pages you can follow
the below procedure.

**Note:** You will still need to manually compile the dpdk toolset before you follow
the below procedure.

The commands above created a directory called Pktgen-DPDK in the current directory
location. You now have the dpdk source.

Make sure you have HUGE TLB support in the kernel with the following commands:

For more information in huge pages see [here](https://www.kernel.org/doc/Documentation/vm/hugetlbpage.txt).

    # grep -i huge /boot/config-2.6.35-24-generic
    CONFIG_HUGETLBFS=y
    CONFIG_HUGETLB_PAGE=y

    # grep -i huge /proc/meminfo
    HugePages_Total:128
    HugePages_Free: 128
    HugePages_Rsvd:0
    HugePages_Surp:0
    Hugepagesize: 2048 kB

**Note:** The values in Total and Free maybe different until you reboot the machine.

Two files in /etc must be setup to support huge TLBs. If you do not have
hugeTLB support then you most likely need a newer kernel.

    # vi /etc/sysctl.conf

Add the below to the bottom of the file to enable 2M hugepages:

    vm.nr_hugepages=256

If you need more or less hugeTLB pages then you can change the value to a
number you need it to be. In some cases pktgen needs a fair number of pages
and making it too small will effect performance or pktgen will terminate on
startup looking for more pages.

**Note:** On Ubuntu 15.10 I noticed mounting /mnt/huge is not required as /dev/hugepages
is already mounted. Check your system and verify that /mnt/huge is required.

    # vi /etc/fstab

Add the below to the bottom of the file:

    huge /mnt/huge hugetlbfs defaults 0 0

    # mkdir /mnt/huge
    # chmod 777 /mnt/huge

Reboot your machine as the huge pages must be setup just after boot to make
sure you have contiguous memory for the 2Meg pages, setting up 1G pages can
also be done.

**Note:** If you startup Eclipse or WR Workbench before starting pktgen the first
 time after reboot, pktgen will fail to load because it can not get all of the
 huge pages as eclipse has consumed some of the huge pages. If you did start eclipse
 or WR Workbench then you need to close that application first.

This is my current machine you will have a few different numbers depending on
how your system was booted and if you had hugeTLB support enabled.

## Setup Prerequisites on Red Hat-based Systems

Red Hat does not provide an up-to-date LUA package so you will need to build it
yourself. You can do this by following the below procedure.

1) Make sure you have run a `yum update -y` prior to continuing. We also recommend
   restarting after you have updated particularly if there were kernel updates.


2) Run `yum remove -y lua-devel` to make sure you don't already have an outdated
   LUA package installed.

   **Note:** This was tested with LUA 5.3.2.

3) Run the following commands:

        # cd /usr/local/src
        # curl -R -O http://www.lua.org/ftp/lua-5.3.2.tar.gz
        # tar zxf lua-5.3.2.tar.gz
        # cd /usr/local/src/lua-5.3.2
        # vi Makefile

        // Change: TO_LIB= liblua.a
        // To be: TO_LIB= liblua.a liblua.so
        // Save and close (<Esc> :wq!)

        # cd /usr/local/src/lua-5.3.2/src
        # vi Makefile

        // Change: CFLAGS= -O2 -Wall -Wextra -DLUA_COMPAT_5_2 $(SYSCFLAGS) $(MYCFLAGS)
        // To be: CFLAGS= -O2 -Wall -Wextra -DLUA_COMPAT_5_2 $(SYSCFLAGS) $(MYCFLAGS) -fPIC
        // Add under: LUA_A= liblua.a
        // The line: LUA_SO= liblua.so
        // Change: ALL_T= $(LUA_A) $(LUA_T) $(LUAC_T)
        // To be: ALL_T= $(LUA_A) $(LUA_T) $(LUAC_T) $(LUA_SO)
        // Add under: $(LUAC_T): $(LUAC_O) $(LUA_A)
        // $(CC) -o $@ $(LDFLAGS) $(LUAC_O) $(LUA_A) $(LIBS)
        // The lines: $(LUA_SO): $(CORE_O) $(LIB_O)
        // $(CC) -o $@ -shared $?
        // Save and close (<Esc> :wq!)

4) Export the LUA directory to your C include path so that gcc can find your headers

        # export C_INCLUDE_PATH=/usr/local/src/lua-5.3.2/src

5) Now we need to manually add the package configuration files. Run `vi /usr/lib64/pkgconfig/lua-5.3.pc`
   and add the below text. You will need to update the version appropriately if you
   are not using LUA 5.3.2. When you are finished save with `wq!`

        V= 5.3
        R= 5.3.2
        prefix= /usr
        exec_prefix=${prefix}
        libdir= /usr/lib64
        includedir=${prefix}/include

        Name: Lua
        Description: An Extensible Extension Language
        Version: ${R}
        Requires:
        Libs: -llua-${V} -lm -ldl
        Cflags: -I${includedir}/lua-${V}

6) Now run `vi /usr/lib64/pkgconfig/lua5.3.pc` and add the below text.

        V=5.3
        R=5.3.2

        prefix=/usr
        INSTALL_BIN=${prefix}/bin
        INSTALL_INC=${prefix}/include
        INSTALL_LIB=${prefix}/lib
        INSTALL_MAN=${prefix}/share/man/man1
        INSTALL_LMOD=${prefix}/share/lua/${V}
        INSTALL_CMOD=${prefix}/lib/lua/${V}
        exec_prefix=${prefix}
        libdir=${exec_prefix}/lib
        includedir=${prefix}/include

        Name: Lua
        Description: An Extensible Extension Language
        Version: ${R}
        Requires:
        Libs: -L${libdir} -llua -lm -ldl
        Cflags: -I${includedir}

7) Move to the lua-5.3.2/src directory and run `make linux`.

8) Once the make has completed without errors copy your newly built libraries over
   to your lib64 folder with:

        # cp /usr/local/src/lua-5.3.2/src/liblua.so /usr/lib64/liblua-5.3.so
        # cp /usr/local/src/lua-5.3.2/src/liblua.a /usr/lib64/liblua-5.3.a

## Setup pktgen-DPDK

At the pktgen-DPDK level directory we have the 'tools/setup.sh' script,
which needs to be run as root once per boot. The script contains a commands to setup
the environment.

Before you run the script you will need to run:

    # export RTE_SDK=<DPDKinstallDir>
    # export RTE_TARGET=x86_64-native-linuxapp-gcc

Make sure you run the setup script as root via `./tools/setup.sh`. The setup
script is a bash script and tries to setup the system correctly, but you may have to
change the script to match your number of huge pages you configured above and ports.

The `modprobe uio` command, in the setup script, loads the UIO support module into the
kernel plus it loads the igb-uio.ko module into the kernel. The two echo commands,
in the setup script, finish setting up the huge pages one for each socket. If you
only have a single socket system then comment out the second echo command. The last
command is to display the huge TLB setup.

Edit your .bashrc or .profile or .cshrc to add the environment variables.
I am using bash: `# vi ~/.bashrc`

Add the following lines: Change the $RTE_SDK to the location of the DPDK version
directory. Your SDK directory maybe named differently, but should point to the DPDK
SDK directory.

    # export RTE_SDK=<DPDKinstallDir>
    # export RTE_TARGET=x86_64-native-linuxapp-gcc

or use clang if you have it installed

    # export RTE_TARGET=x86_64-native-linuxapp-clang

Create the DPDK build tree if you haven't already:

    # cd $RTE_SDK
    # make install T=x86_64-native-linuxapp-gcc -j

The above command will create the x86_64-native-linuxapp-gcc directory in the
top level of the current-dkdp directory. The above command will build the basic
DPDK libraries and build tree.

Next we build pktgen:

    # cd <PktgenInstallDir>
    # make -j

You should now have pktgen built.

To get started, see README.md.
