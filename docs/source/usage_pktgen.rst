.. _usage_pktgen:

Pktgen Commandline Options
==========================

The Pktgen commandline usage is::

   ./app/app/``$(target}``/pktgen [EAL options] -- \
				[-h] [-P] [-G] [-T] [-f cmd_file] \
				[-l log_file] [-s P:PCAP_file] [-m <string>]

The :ref:`usage_eal` were shown in the previous section.

The ``pktgen`` arguments are::

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
  -h           Display the help information


Where the options are:

* ``-h``: Display the usage/help information shown above::

     lspci | grep Ethernet

  This shows a list of all ports in the system. Some ports may not be usable
  by DPDK/Pktgen.  The first port listed is bit 0 or least signification bit
  in the ``-c`` EAL coremask. Another method is to compile and run the DPDK
  sample application ``testpmd`` to list out the ports DPDK is able to use::

     ./test_pmd -c 0x3 -n 2

* ``-s P:file``: The PCAP packet file to stream. ``P`` is the port number.

* ``-f filename``: The script command file (.pkt) to execute or a Lua script
  (.lua) file. See :ref:`scripts`.

* ``-l filename``: The filename to write a log to.

* ``-P``: Enable PROMISCUOUS mode on all ports.

* ``-G``: Enable socket support using default server values of
  localhost:0x5606. See :ref:`socket`.

* ``-g address``: Same as ``-G`` but with an optional IP address and port
  number. See :ref:`socket`.

* ``-T``: Enable color terminal output in VT100

* ``-N``: Enable NUMA support.

* ``-m <string>``: Matrix for mapping ports to logical cores. The format of the
  port mapping string is defined with a BNF-like grammar as follows::

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

  For example::

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

Grouping can use ``{}`` instead of ``[]`` if required.

Multiple Instances of Pktgen or other application
=================================================

One possible solution I use and if you have enough ports available to use.
Lets say you need two ports for your application, but you have 4 ports in
your system. I physically loop back the cables to have port 0 connect to
port 2 and port 1 connected to port 3. Now I can give two ports to my
application and two ports to Pktgen.

Setup if pktgen and your application you have to startup each one a bit
differently to make sure they share the resources like memory and the
ports. I will use two Pktgen running on the same machine, which just means
you have to setup your application as one of the applications.

In my machine I have 8 10G ports and 72 lcores between 2 sockets. Plus I
have 1024 hugepages per socket for a total of 2048.

  Example commands::

     # lspci | grep Ether
     06:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
     06:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
     08:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
     08:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
     09:00.0 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
     09:00.1 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
     83:00.1 Ethernet controller: Intel Corporation DH8900CC Null Device (rev 21)
     87:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
     87:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
     89:00.0 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)
     89:00.1 Ethernet controller: Intel Corporation Ethernet Converged Network Adapter X520-Q1 (rev 01)

     ./app/app/${target}/pktgen -l 2-11 -n 3 --proc-type auto \
		--socket-mem 512,512 --file-prefix pg1 \
		-b 09:00.0 -b 09:00.1 -b 83:00.1 -b 06:00.0 \
		-b 06:00.1 -b 08:00.0 -b 08:00.1 -- \
		-T -P -m "[4:6].0, [5:7].1, [8:10].2, [9:11].3" \
		-f themes/black-yellow.theme

     ./app/app/${target}/pktgen -l 2,4-11 -n 3 --proc-type auto \
		--socket-mem 512,512 --file-prefix pg2 \
		-b 09:00.0 -b 09:00.1 -b 83:00.1 -b 87:00.0 \
		-b 87:00.1 -b 89:00.0 -b 89:00.1 -- \
		-T -P -m "[12:16].0, [13:17].1, [14:18].2, [15:19].3" \
		-f themes/black-yellow.theme

Notice I black list the three onboard devices and then black list the
other 4 ports I will not be using for each of the pktgen instances.

I need 8+1 lcores for each instance for Pktgen use. The -c option of ff2
and FF004 lcores, the ff value are used for port handling and the 2/4 is
used because pktgen needs the first lcore for display and timers.

The -m option then assigns lcores to the ports.

The information from above is taken from two new files pktgen-master.sh
and pktgen-slave.sh, have a look at them and adjust as you need.

Pktgen can also be configured using the :ref:`commands`.
