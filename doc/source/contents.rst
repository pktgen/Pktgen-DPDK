
The Pktgen Application
======================

**Pktgen**, (*Packet* *Gen*-erator) is a software based traffic generator
powered by the DPDK fast packet processing framework.

Some of the features of Pktgen are:

* It is capable of generating 10Gbit wire rate traffic with 64 byte frames.
* It can act as a transmitter or receiver at line rate.
* It has a runtime environment to configure, and start and stop traffic flows.
* It can display real time metrics for a number of ports.
* It can generate packets in sequence by iterating source or destination MAC,
  IP addresses or ports.
* It can handle packets with UDP, TCP, ARP, ICMP, GRE, MPLS and
  Queue-in-Queue.
* It can be controlled remotely over a TCP connection.
* It is configurable via Lua and can run command scripts to set up repeatable
  test cases.
* The software is fully available under a BSD licence.


Pktgen was created 2010 by Keith Wiles @ windriver.com, now at intel.com

.. only:: html

   See the sections below for more details.

   Contents:


.. toctree::
   :maxdepth: 1

   getting_started.rst
   running.rst
   usage_eal.rst
   usage_pktgen.rst
   commands.rst
   cli.rst
   cli_lib.rst
   scripts.rst
   lua.rst
   socket.rst
   changes.rst
   copyright.rst
   license.rst
