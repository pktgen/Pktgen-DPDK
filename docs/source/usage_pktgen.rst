.. _usage_pktgen:

Pktgen Commandline Options
==========================

The Pktgen commandline usage is::

   ./app/pktgen [EAL options] -- \
                -p PORTMASK [-h] [-P] [-G] [-f cmd_file] \
                [-l log_file] [-s P:PCAP_file] [-m <string>]

The :ref:`usage_eal` were shown in the previous section.

The ``pktgen`` arguments are::

   Usage:
     -h           Display the help information
     -p PORTMASK  hexadecimal bitmask of ports to configure
     -s P:file    PCAP packet stream file, 'P' is the port number
     -f filename  Command file (.pkt) to execute or a Lua script (.lua) file
     -l filename  Write log to filename
     -P           Enable PROMISCUOUS mode on all ports
     -G           Enable socket support using default server: localhost:0x5606
     -g address   Like -g with Optional IP address and port.
     -N           Enable NUMA support
     -m <string>  Matrix for mapping ports to logical cores.


Where the options are:

* ``-h``: Display the usage/help information shown above.

* ``-p PORTMASK``: The hexadecimal bitmask of the ports to configure. To
  determine the Ethernet ports in your system use::

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
     1.[0-2], 2.3, ...             - core 1 handles ports 0,1,2 rx/tx,
                                     core 2 handles port 3 rx/tx
     [0-1].0, [2/4-5].1, ...       - core 0-1 handle port 0 rx/tx,
                                     core 2,4,5 handle port 1 rx/tx
     [1:2].0, [4:6].1, ...         - core 1 handles port 0 rx,
                                     core 2 handles port 0 tx,
     [1:2].[0-1], [4:6].[2/3], ... - core 1 handles port 0 & 1 rx,
                                     core 2 handles port  0 & 1 tx
     [1:2-3].0, [4:5-6].1, ...     - core  1   handles port 0 rx,
                                     cores 2,3 handle  port 0 tx
                                     core  4   handles port 1 rx
                                     core  5,6 handles port 1 tx
     [1-2:3].0, [4-5:6].1, ...     - core 1,2 handles port 0 rx
                                     core 3   handles port 0 tx
                                     core 4,5 handles port 1 rx
                                     core 6   handles port 1 tx
     [1-2:3-5].0, [4-5:6/8].1, ... - core 1,2   handles port 0 rx
                                     core 3,4,5 handles port 0 tx
                                     core 4,5   handles port 1 rx
                                     core 6,8   handles port 1 tx
     [1:2].[0:0-7], [3:4].[1:0-7], - core 1 handles port  0   rx
                                     core 2 handles ports 0-7 tx
                                     core 3 handles port  1   rx
                                     core 4 handles port  0-7 tx

Grouping can use ``{}`` instead of ``[]`` if required.


Pktgen can also be configured using the :ref:`commands`.
