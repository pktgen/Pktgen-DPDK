.. _usage_eal:


EAL Commandline Options
=======================

Pktgen, like other DPDK applications splits commandline arguments into
arguments for the DPDK Environmental Abstraction Layer (EAL) and arguments for
the application itself. The two sets of arguments are separated using the
standard convention of ``--``::

Pktgen executable is located at ``./app/app/${RTE_TARGET}/pktgen``

   pktgen -l 0-4 -n 3 -- -P -m "[1:3].0, [2:4].1

The usual EAL commandline usage for ``pktgen`` is::

   pktgen -c COREMASK -n NUM \
                [-m NB] \
                [-r NUM] \
                [-b <domain:bus:devid.func>] \
                [--proc-type primary|secondary|auto] -- [pktgen options]

The full list of EAL arguments are::

   EAL options:
     -c COREMASK         : A hexadecimal bitmask of cores to run on
     -n NUM              : Number of memory channels
     -v                  : Display version information on startup
     -d LIB.so           : Add driver (can be used multiple times)
     -m MB               : Memory to allocate (see also --socket-mem)
     -r NUM              : Force number of memory ranks (don't detect)
     --xen-dom0          : Support application running on Xen Domain0 without
                           hugetlbfs
     --syslog            : Set syslog facility
     --socket-mem        : Memory to allocate on specific
                           sockets (use comma separated values)
     --huge-dir          : Directory where hugetlbfs is mounted
     --proc-type         : Type of this process
     --file-prefix       : Prefix for hugepage filenames
     --pci-blacklist, -b : Add a PCI device in black list.
                           Prevent EAL from using this PCI device. The argument
                           format is <domain:bus:devid.func>.
     --pci-whitelist, -w : Add a PCI device in white list.
                           Only use the specified PCI devices. The argument
                           format is <[domain:]bus:devid.func>. This option
                           can be present several times (once per device).
                           NOTE: PCI whitelist cannot be used with -b option
     --vdev              : Add a virtual device.
                           The argument format is <driver><id>[,key=val,...]
                           (ex: --vdev=eth_pcap0,iface=eth2).
     --vmware-tsc-map    : Use VMware TSC map instead of native RDTSC
     --base-virtaddr     : Specify base virtual address
     --vfio-intr         : Specify desired interrupt mode for VFIO
                           (legacy|msi|msix)
     --create-uio-dev    : Create /dev/uioX (usually done by hotplug)

   EAL options for DEBUG use only:
     --no-huge           : Use malloc instead of hugetlbfs
     --no-pci            : Disable pci
     --no-hpet           : Disable hpet
     --no-shconf         : No shared config (mmap'd files)


The ``-c COREMASK`` and ``-n NUM`` arguments are required. The other arguments
are optional.

Pktgen requires 2 logical cores (lcore) in order to run. The first lcore, 0,
is used for the ``pktgen`` commandline, for timers and for displaying the
runtime metrics text on the terminal. The additional lcores ``1-n`` are used
to do the packet receive and transmits along with anything else related to
packets.

You do not need to start at the actual system lcore 0. The application will
use the first lcore in the coremask bitmap.


A more typical commandline to start a ``pktgen`` instance would be::

   pktgen -l 0-4 -n 3 --proc-type auto --socket-mem 256,256
                -b 0000:03:00.0 -b 0000:03:00.1 \
                 --file-prefix pg \
                -- -P -m "[1:3].0, [2:4].1

The coremask ``-c 0x1f`` (0b11111) indicates 5 lcores are used, as the first
lcore is used by Pktgen for display and timers.

The ``--socket-mem 256,256`` DPDK command will allocate 256M from each CPU
(two in this case).

The :ref:`usage_pktgen` are shown in the next section.
