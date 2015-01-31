.. _getting_started:

Getting Started with Pktgen
===========================

This section contains instructions on how to get up and running with `DPDK
<http://dpdk.org/>`_ and the ``pktgen`` traffic generator application.

These instructions relate to setting up DPDK and ``pktgen`` on an Ubuntu
desktop system. However, the should work on any recent Linux system with
kernel support for hugeTLB/hugepages.


System requirements
-------------------

The main system requirement is that the DPDK packet processing framework is
supported.

The `DPDK Linux Getting Started Guide
<http://www.dpdk.org/doc/guides/linux_gsg/index.html>`_ has a section on the
`System Requirements
<http://www.dpdk.org/doc/guides/linux_gsg/sys_reqs.html>`_ that explains the
BIOS, System and Toolchain requirements to compile and run a DPDK based
application such as ``pktgen``. Ensure that your system meets those requirements
before proceeding.

You will also need a `DPDK supported NIC <http://www.dpdk.org/doc/nics>`_.

The current version of ``pktgen`` was developed and tested using Ubuntu 13.10
x86_64, kernel version 3.5.0-25, on a Westmere Dual socket board running at
2.4GHz with 12GB of ram 6GB per socket.


Setting up hugeTLB/hugepage support
-----------------------------------

To get hugeTLB/hugepage support your Linux kernel must be at least 2.6.33 and
the ``HUGETLBFS`` kernel option must be enabled.

The DPDK Linux Getting Started Guide has a section on the `Use of Hugepages in
the Linux Environment
<http://www.dpdk.org/doc/guides/linux_gsg/sys_reqs.html#use-of-hugepages-in-the-linux-environment>`_.

Once you have made the required changed make sure you have HUGE TLB support in the kernel with the following commands::

   $ grep -i huge /boot/config-2.6.35-24-generic
   CONFIG_HUGETLBFS=y
   CONFIG_HUGETLB_PAGE=y

   $ grep -i huge /proc/meminfo

   HugePages_Total:      128
   HugePages_Free:       128
   HugePages_Rsvd:        0
   HugePages_Surp:        0
   Hugepagesize:       2048 kB


The values in Total and Free may be different depending on your system.

You will need to edit the ``/etc/sysctl.conf`` file to setup the hugepages
size::

   $ sudo vi /etc/sysctl.conf
   Add to the bottom of the file:
   vm.nr_hugepages=256

You can configure the ``vm.nr_hugepages=256`` as required. In some cases
making it too small will effect the performance of pktgen or cause it to
terminate on startup.

You will also need to edit the ``/etc/fstab`` file to mount the hugepages at
startup::

   $ sudo vi /etc/fstab
   Add to the bottom of the file:
   huge /mnt/huge hugetlbfs defaults 0 0

   $ sudo mkdir /mnt/huge
   $ sudo chmod 777 /mnt/huge

You should also reboot your machine as the huge pages must be setup just after
boot to make sure there is enough contiguous memory for the 2MB pages.

.. Note::

   If you start an application that makes extensive use of hugepages, such as
   Eclipse or WR Workbench, before starting ``pktgen`` for the first time
   after reboot, ``pktgen`` may fail to load. In this case you should close
   the other application that is using hugepages.



BIOS settings
-------------

In the BIOS make sure that the HPET High Precision Event Timer is
enabled. Also make sure hyper-threading is enabled. See the DPDK documentation
on `enabling additional BIOS functionality
<http://www.dpdk.org/doc/guides/linux_gsg/enable_func.html#enabling-additional-functionality>`_
for more details.


Terminal display
----------------

The ``pktgen`` output display requires 132 columns and about 42 lines to
display correctly. The author uses an xterm of 132x42, but you can also have a
larger display and maybe a bit smaller. If you are displaying more then 4-6
ports then you will need a wider display.

Pktgen allows you to view a set ports via the ``page`` runtime command if they
do not all fit on the screen at one time, see :ref:`commands`.

Pktgen uses VT100 control codes display its output screens, which means your
terminal must support VT100.

It is also best to set your terminal background to black when working with the
default ``pktgen`` color scheme.



Get the source code
-------------------

Pktgen requires the DPDK source code to build.

The main ``dpdk`` and ``pktgen`` git repositories are hosted on `dpdk.org
<http://www.dpdk.org/browse/>`_.

The ``dpdk`` code can be cloned as follows::

   git clone git://dpdk.org/dpdk
   # or:
   git clone http://dpdk.org/git/dpdk

The ``pktgen`` code can be cloned as follows::

   git clone git://dpdk.org/apps/pktgen-dpdk
   # or:
   git clone http://dpdk.org/git/apps/pktgen-dpdk

In the instructions below the repository close directories are referred to as
``DPDKInstallDir`` and ``PktgenInstallDir``.

You will also require the Linux kernel headers to allow DPDK to build its
kernel modules. On Ubuntu you can install them as follows (where the version
matches the kernel version)::

   $ sudo apt-get install linux-headers-3.5.0-32-generic

DPDK can also work with a ``libpcap`` driver which is sometimes useful for
testing without a real NIC or for low speed packet capture. Install the
``libpcap`` development libs using your package manage. For example::

    $ sudo apt-get install libpcap-dev


Build DPDK and Pktgen
---------------------

Set up the environmental variables required by DPDK::

   export RTE_SDK=<DPDKInstallDir>
   export RTE_TARGET=x86_64-native-linuxapp-gcc

   # or use clang if you have it installed:
   export RTE_TARGET=x86_64-native-linuxapp-clang

Create the DPDK build tree::

   $ cd $RTE_SDK
   $ make install T=x86_64-native-linuxapp-gcc

This above command will create the `x86_64-pktgen-linuxapp-gcc` directory in
the top level of the ``$RTE_SDK`` directory. It will also build the basic DPDK
libraries, kernel modules and build tree.

Pktgen can then be built as follows::

   $ cd <PktgenInstallDir>
   $ make


Setting up your environment
---------------------------

In the ``PktgenInstallDir`` level directory there is ``setup.sh`` script,
which should be run once per boot. The script contains the commands required
to set up the environment::

   $ cd <PktgenInstallDir>
   $ sudo ./setup.sh

The setup script is a bash script and tries to configure the system to run a
DPDK application. You will probably have to change the script to match your
system.

The ``modprobe uio`` command, in the setup script, loads the UIO support
module into the kernel as well as loafing the igb-uio.ko module.

The two echo commands, in the setup script, set up the huge pages for a two
socket system. If you only have a single socket system then remove the second
echo command. The last command in the script is used to display the hugepage
setup.

You may also wish to edit your ``.bashrc``, ``.profile`` or ``.cshrc`` files to
permanently add the environment variables that you set up above::

   export RTE_SDK=<DPDKInstallDir>
   export RTE_TARGET=x86_64-native-linuxapp-gcc


Running the application
-----------------------

Once the above steps have been completed and the ``pktgen`` application has
been compiled you can run it using the commands shown in the :ref:`running`
section.
