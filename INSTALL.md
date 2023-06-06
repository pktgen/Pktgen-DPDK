# Pktgen - Traffic Generator powered by DPDK

---
Pktgen is a traffic generator powered by DPDK at wire rate traffic with 64 byte frames.**

## (Pktgen) Sounds like 'Packet-Gen'

---
**Copyright &copy; \<2010-2020\>, Intel Corporation. All rights reserved.**

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

Pktgen: Created 2010-2020 by Keith Wiles @ Intel.com

---

## Installation

INSTALL for setting up Pktgen with DPDK on Ubuntu 18.04 to 20.04 desktop
it should work on most Linux systems as long as the kernel has hugeTLB page support and build DPDK.

### Note

>Tested with Ubuntu 18.04 and up to 20.04 kernel versions
Linux 4.15.0-39-generic #42-Ubuntu SMP Tue Oct 23 15:48:01 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux
Please install the latest version of DPDK from dpdk.org and follow the build instructions
using meson/ninja. Then install DPDK on your system.

## Building Pktgen

Pktgen has been converted to use meson/ninja for configuration and building. **Makefiles have been removed.** This may require DPDK to be built
using meson/ninja. At least a libdpdk.pc file must be present in the system for Pktgen to locate the headers and libraries.

>Please read the DPDK.org documentation to understand more on building DPDK. The following is the minimum set of
instructions to build DPDK. You may need to install meson and ninja, if not already installed.

```console
git clone https://dpdk.org/git/dpdk
sudo rm -fr /usr/local/lib/x86_64-linux-gnu # DPDK changed a number of lib names and need to clean up
cd dpdk
meson build
ninja -C build
sudo ninja -C build install
sudo ldconfig  # make sure ld.so is pointing new DPDK libraries
```

DPDK places the libdpdk.pc (pkg-config file) in a non-standard location and you need to set enviroment variable PKG_CONFIG_PATH to the location of the file. On Ubuntu 20.04 build of DPDK it places the file here /usr/local/lib/x86_64-linux-gnu/pkgconfig/libdpdk.pc

```console
$ export PKG_CONFIG_PATH=/usr/local/lib/x86_64-linux-gnu/pkgconfig
```

Building Pktgen after you have built and installed DPDK. The new build system uses meson/ninja, but Pktgen has a build script
called 'tools/pktgen-build.sh' and uses a very simple Makefile to help build Pktgen without having to fully understand meson/ninja command line.

If you prefer you can still use meson/ninja directly.

```console
git clone http://dpdk.org/git/apps/pktgen-dpdk

cd pktgen-dpdk
make
or
make build    # Same as 'make'
or
make rebuild  # Rebuild Pktgen, which removes the Builddir then builds it again via meson/ninja
or
make rebuildlua # to enable Lua builds
or
make rebuildgui # to enable GUI builds with GTK

# Use 'make help' to read the help message for building.

# DPDK does not add a /etc/ld.so.conf.d/<dpdk-libs> like file. This means you may need to
# edit /etc/ld.so.conf.d/x86_64-linux-gnu.conf file and add /usr/local/lib/x86_64-linux-gnu
# Then do 'sudo ldconfig' to locate the DPDK libraries.

# If you want to use vfio-pci then edit /etc/default/grub and add 'intel_iommu=on' to the LINUX default line
# Then use 'update-grub' command then reboot the system.
```

Editing the meson_options.txt can be done, but normally you should use a meson command line options to enable/disable options.

## Pktgen Information

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

## Setup Pktgen-DPDK

At the pktgen-DPDK level directory we have the 'tools/setup.sh' script,
which needs to be run as root once per boot. The script contains a commands to setup
the environment.

Before you run the script you will need to run:

    # export RTE_SDK=<DPDKinstallDir>
    # export RTE_TARGET=x86_64-native-linux-gcc

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
    # export RTE_TARGET=x86_64-native-linux-gcc

or use clang if you have it installed

    # export RTE_TARGET=x86_64-native-linux-clang

Create the DPDK build tree if you haven't already:

    # cd $RTE_SDK
    # make install T=x86_64-native-linux-gcc -j

The above command will create the x86_64-native-linux-gcc directory in the
top level of the current-dkdp directory. The above command will build the basic
DPDK libraries and build tree.

Next we build pktgen:

    # cd <PktgenInstallDir>
    # make -j

For CentOS and pcap support you may need to try:
For libpcap-devel

    # yum install dnf-plugins-core
    # yum config-manager --set-enabled PowerTools
    # yum repolist

You should now have pktgen built.

To get started, see README.md.
