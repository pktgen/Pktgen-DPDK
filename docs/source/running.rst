.. _running:

Running Pktgen
==============


A sample commandline to start a ``pktgen`` instance would look something like
the following, which you may need 'sudo -E' added to the front if not superuser.
The -E option of sudo passes environment variables to sudo shell as the scripts
need the RTE_SDK and RTE_TARGET variables::

   ./app/pktgen -c 0x1f -n 3 -- -P -m "[1:3].0, [2:4].1

Pktgen, like other DPDK applications splits its commandline arguments into
arguments for the DPDK Environmental Abstraction Layer (EAL) and arguments for
the application itself. The two sets of arguments are separated using the
standard convention of ``--`` as shown above.

These commandline arguments are explained in the :ref:`usage_eal` and
:ref:`usage_pktgen`.

The output when running ``pktgen`` will look something like the following::

   -----------------------

   Copyright notices

   -----------------------
   EAL: Detected lcore 0 as core 0 on socket 0
   EAL: Detected lcore 1 as core 1 on socket 0
   ...
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
       1, type  RX , rx_cnt  1, tx_cnt  0, RX (pid:qid): ( 0: 0) , TX (pid:qid):
       2, type  RX , rx_cnt  1, tx_cnt  0, RX (pid:qid): ( 1: 0) , TX (pid:qid):
       3, type  TX , rx_cnt  0, tx_cnt  1, RX (pid:qid): , TX (pid:qid): ( 0: 0)
       4, type  TX , rx_cnt  0, tx_cnt  1, RX (pid:qid): , TX (pid:qid): ( 1: 0)

   Port :
       0, nb_lcores  2, private 0x7d08d8, lcores:  1  3
       1, nb_lcores  2, private 0x7d1c48, lcores:  2  4


   Initialize Port 0 -- TxQ 1, RxQ 1,  Src MAC 90:e2:ba:5a:f7:90
       Create: Default RX  0:0 - Mem (MBUFs 2048 x (1984 + 64)) + 790720 = 4869 KB
       Create: Default TX  0:0 - Mem (MBUFs 2048 x (1984 + 64)) + 790720 = 4869 KB
       Create: Range TX    0:0 - Mem (MBUFs 2048 x (1984 + 64)) + 790720 = 4869 KB
       Create: Sequence TX 0:0 - Mem (MBUFs 2048 x (1984 + 64)) + 790720 = 4869 KB
       Create: Special TX  0:0 - Mem (MBUFs   64 x (1984 + 64)) + 790720 =  901 KB

                                                      Port memory used =  20373 KB

   Initialize Port 1 -- TxQ 1, RxQ 1,  Src MAC 90:e2:ba:5a:f7:91
       Create: Default RX  1:0 - Mem (MBUFs 2048 x (1984 + 64)) + 790720 = 4869 KB
       Create: Default TX  1:0 - Mem (MBUFs 2048 x (1984 + 64)) + 790720 = 4869 KB
       Create: Range TX    1:0 - Mem (MBUFs 2048 x (1984 + 64)) + 790720 = 4869 KB
       Create: Sequence TX 1:0 - Mem (MBUFs 2048 x (1984 + 64)) + 790720 = 4869 KB
       Create: Special TX  1:0 - Mem (MBUFs   64 x (1984 + 64)) + 790720 =  901 KB

                                                      Port memory used =  20373 KB
                                                     Total memory used =  40746 KB

   Port  0: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>
   Port  1: Link Up - speed 10000 Mbps - full-duplex <Enable promiscuous mode>


   === Display processing on lcore 0
   === RX processing on lcore  1, rxcnt 1, port/qid, 0/0
   === RX processing on lcore  2, rxcnt 1, port/qid, 1/0
   === TX processing on lcore  3, txcnt 1, port/qid, 0/0
   === TX processing on lcore  4, txcnt 1, port/qid, 1/0
   ...


Once ``pktgen`` is running you will see an output like the following::

   - Ports 0-3 of 6   ** Main Page **  Copyright
     Flags:Port    :   P-------------:0   P-------------:1
   Link State      :      <UP-10000-FD>      <UP-10000-FD>  ---TotalRate---
   Pkts/s  Rx      :                  0                  0                0
           Tx      :                  0                  0                0
   MBits/s Rx/Tx   :                0/0                0/0              0/0
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

   Pktgen>


The flags displayed on the top line for each port are::

   P--------------- - Promiscuous mode enabled
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

The ``pktgen`` default colors and theme work best on a black background. If
required, it is possible to set other color themes, (see :ref:`commands`).
