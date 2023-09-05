.. _commands:

``*** Pktgen ***``
Copyright &copy \<2015-2020\>, Intel Corporation.

README for setting up Pktgen with DPDK on Ubuntu 10.04 to 20.04 desktop, it
should work on most Linux systems as long as the kernel has hugeTLB page support.

Note: Tested with Ubuntu 18.04 and up to 20.04 versions
Linux 3.5.0-25-generic #39-Ubuntu SMP Mon Feb 25 18:26:58 UTC 2013 x86_64

I am using Ubuntu 16.10 x86_64 (64 bit support) for running Pktgen-DPDK on a
Crownpass Dual socket board running at 2.4GHz with 32GB of ram 16GB per socket.
The current kernel version is 4.4.0-66-generic (as of 2018-04-01) support, but should
work on just about any new Linux kernel version.

Currently using as of 2020-05 Ubuntu 20.04 Kernel 5.6.0+ system.

To get hugeTLB page support your Linux kernel must be at least 2.6.33 and in the
DPDK documents it talks about how you can upgrade your Linux kernel.

Here is another document on how to upgrade your Linux kernel.
Ubuntu 10.04 is 2.6.32 by default so upgraded to kernel 2.6.34 using this HOWTO:
http://usablesoftware.wordpress.com/2010/05/26/switch-to-a-newer-kernel-in-ubuntu-10-04/

The pktgen output display needs 132 columns and about 42 lines to display
currently. I am using an xterm of 132x42, but you can have a larger display
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


Pktgen command line directory format
====================================

-- Pktgen Ver: 3.2.x (DPDK 17.05.0-rc0)  Powered by DPDK ---------------

Show the commands inside the ``pktgen/bin`` directory::

	Pktgen:/> ls
	[pktgen]        [sbin]          dpdk-version    copyright
    Pktgen:/> ls pktgen/bin
    off             on              rate            plugin          dbg
    set             pcap            stp             str             stop
    start           disable         enable          range           theme
    page            seq             sequence        ping4           port
    restart         rst             reset           cls             redisplay
    save            load            clr             clear           help

Showin the ``1s`` command at root::

	Pktgen:/> ls
	[pktgen]        [sbin]          copyright
    Pktgen:/> ls sbin
    version         echo            script          env             path
    hugepages       cmap            more            history         quit
    screen.clear    pwd             cd              ls              rm
    mkdir           chelp           sleep           delay

The case of using ``ls -l`` in a subdirectory::

    Pktgen:/> cd sbin
    Pktgen:/sbin/> ls -l
    version          Command : Display version information
    echo             Command : simple echo a string to the screen
    script           Command : load and process cli command files
    env              Command : Show/del/get/set environment variables
    path             Command : display the execution path for commands
    hugepages        Command : hugepages # display hugepage info
    cmap             Command : cmap # display the core mapping
    more             Command : more <file> # display a file content
    history          Command : history # display the current history
    quit             Command : quit # quit the application
    screen.clear     Command : screen.clear # clear the screen
    pwd              Command : pwd # display current working directory
    cd               Command : cd <dir> # change working directory
    ls               Command : ls [-lr] <dir> # list current directory
    rm               Command : remove a file or directory
    mkdir            Command : create a directory
    chelp            Command : CLI help - display information for DPDK
    sleep            Command : delay a number of seconds
    delay            Command : delay a number of milliseconds

Show help using ``ls -l`` command in pktgen directory::

    Pktgen:/sbin/> cd ../pktgen/bin
    Pktgen:/pktgen/bin/> ls -l
    off              Alias : disable screen
    on               Alias : enable screen
    rate             Command : Rate setup commands
    plugin           Command : Plugin a shared object file
    dbg              Command : debug commands
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
    load             Command : load command file
    clr              Alias : clear all stats
    clear            Command : clear stats, ...
    help             Command : help command

    Pktgen:/pktgen/bin/>

Runtime Options and Commands
============================

While the ``pktgen`` application is running you will see a command prompt as
follows::

   Pktgen:/>

From this you can get help or issue runtime commands::

   Pktgen:/> help

   set <portlist> <xxx> value    - Set a few port values
   save <path-to-file>           - Save a configuration file using the
                                   filename
   load <path-to-file>           - Load a command/script file from the
                                   given path
   ...


The ``page`` commands to show different screens::

        ** Pktgen Help Information **

    page [0-7]                         - Show the port pages or configuration or sequence page
    page main                          - Display page zero
    page range                         - Display the range packet page
    page config | cfg                  - Display the configuration page
    page pcap                          - Display the pcap page
    page cpu                           - Display some information about the CPU system
    page next                          - Display next page of PCAP packets.
    page sequence | seq                - sequence will display a set of packets for a given port
                                        Note: use the 'port <number>' to display a new port sequence
    page rnd                           - Display the random bitfields to packets for a given port
                                        Note: use the 'port <number>' to display a new port sequence
    page log                           - Display the log messages page
    page latency                       - Display the latency page
    page stats                         - Display physical ports stats for all ports
    page xstats                        - Display port XSTATS values
    page rate                          - Display Rate Pacing values

List of the ``enable/disable`` commands::

    enable|disable <portlist> process  - Enable or Disable processing of ARP/ICMP/IPv4/IPv6 packets
    enable|disable <portlist> mpls     - Enable/disable sending MPLS entry in packets
    enable|disable <portlist> qinq     - Enable/disable sending Q-in-Q header in packets
    enable|disable <portlist> gre      - Enable/disable GRE support
    enable|disable <portlist> gre_eth  - Enable/disable GRE with Ethernet frame payload
    enable|disable <portlist> vlan     - Enable/disable VLAN tagging
    enable|disable <portlist> garp     - Enable or Disable Gratuitous ARP packet processing
    enable|disable <portlist> random   - Enable/disable Random packet support
    enable|disable <portlist> latency  - Enable/disable latency testing
    enable|disable <portlist> pcap     - Enable or Disable sending pcap packets on a portlist
    enable|disable <portlist> blink    - Blink LED on port(s)
    enable|disable <portlist> rx_tap   - Enable/Disable RX Tap support
    enable|disable <portlist> tx_tap   - Enable/Disable TX Tap support
    enable|disable <portlist> icmp     - Enable/Disable sending ICMP packets
    enable|disable <portlist> range    - Enable or Disable the given portlist for sending a range of packets
    enable|disable <portlist> capture  - Enable/disable packet capturing on a portlist, disable to save capture
                                        Disable capture on a port to save the data into the currect working directory.
    enable|disable <portlist> bonding  - Enable call TX with zero packets for bonding driver
    enable|disable <portlist> vxlan    - Send VxLAN packets
    enable|disable <portlist> rate     - Enable/Disable Rate Packing on given ports
    enable|disable mac_from_arp        - Enable/disable MAC address from ARP packet
    enable|disable screen              - Enable/disable updating the screen and unlock/lock window
        off                            - screen off shortcut
        on                             - screen on shortcut

List of the ``set`` commands::

        note: <portlist>               - a list of ports (no spaces) e.g. 2,4,6-9,12 or the word 'all'
    set <portlist> count <value>       - number of packets to transmit
    set <portlist> size <value>        - size of the packet to transmit
    set <portlist> rate <percent>      - Packet rate in percentage
    set <portlist> txburst <value>     - number of packets in a Tx burst
    set <portlist> rxburst <value>     - number of packets in a Rx burst
    set <portlist> tx_cycles <value>   - DEBUG to set the number of cycles per TX burst
    set <portlist> sport <value>       - Source port number for TCP
    set <portlist> dport <value>       - Destination port number for TCP
    set <portlist> ttl <value>         - Set the TTL value for the single port more
    set <portlist> seq_cnt|seqcnt|seqCnt <value>
                                    - Set the number of packet in the sequence to send [0-16]
    set <portlist> prime <value>       - Set the number of packets to send on prime command
    set <portlist> dump <value>        - Dump the next 1-32 received packets to the screen
                                        Dumped packets are in the log, use 'page log' to view
    set <portlist> vlan|vlanid <value> - Set the VLAN ID value for the portlist
    set <portlist> jitter <value>      - Set the jitter threshold in micro-seconds
    set <portlist> src|dst mac <addr>  - Set MAC addresses 00:11:22:33:44:55 or 0011:2233:4455 format
    set <portlist> type ipv4|ipv6|vlan|arp - Set the packet type to IPv4 or IPv6 or VLAN
    set <portlist> proto udp|tcp|icmp  - Set the packet protocol to UDP or TCP or ICMP per port
    set <portlist> pattern <type>      - Set the fill pattern type
                    type - abc        - Default pattern of abc string
                            none       - No fill pattern, maybe random data
                            zero       - Fill of zero bytes
                            user       - User supplied string of max 16 bytes
    set <portlist> user pattern <string> - A 16 byte string, must set 'pattern user' command
    set <portlist> [src|dst] ip ipaddr - Set IP addresses, Source must include network mask e.g. 10.1.2.3/24
    set <portlist> qinqids <id1> <id2> - Set the Q-in-Q ID's for the portlist
    set <portlist> rnd <idx> <off> <mask> - Set random mask for all transmitted packets from portlist
        idx: random mask index slot
        off: offset in bytes to apply mask value
        mask: up to 32 bit long mask specification (empty to disable):
            0: bit will be 0
            1: bit will be 1
            .: bit will be ignored (original value is retained)
            X: bit will get random value
    set <portlist> cos <value>         - Set the CoS value for the portlist
    set <portlist> tos <value>         - Set the ToS value for the portlist
    set <portlist> vxlan <flags> <group id> <vxlan_id> - Set the vxlan values
    set ports_per_page <value>         - Set ports per page value 1 - 6

The ``range`` commands::

  -- Setup the packet range values --
     note: SMMI = start|min|max|inc (start, minimum, maximum, increment)

    range <portlist> src|dst mac <SMMI> <etheraddr> - Set destination/source MAC address
        e.g: range 0 src mac start 00:00:00:00:00:00
            range 0 dst mac max 00:12:34:56:78:90
        or  range 0 src mac 00:00:00:00:00:00 00:00:00:00:00:00 00:12:34:56:78:90 00:00:00:01:01:01
    range <portlist> src|dst ip <SMMI> <ipaddr>   - Set source IP start address
        e.g: range 0 dst ip start 0.0.0.0
            range 0 dst ip min 0.0.0.0
            range 0 dst ip max 1.2.3.4
            range 0 dst ip inc 0.0.1.0
        or  range 0 dst ip 0.0.0.0 0.0.0.0 1.2.3.4 0.0.1.0
    range <portlist> proto tcp|udp                - Set the IP protocol type
    range <portlist> src|dst port <SMMI> <value>  - Set UDP/TCP source/dest port number
        or  range <portlist> src|dst port <start> <min> <max> <inc>
    range <portlist> vlan <SMMI> <value>          - Set vlan id start address
        or  range <portlist> vlan <start> <min> <max> <inc>
    range <portlist> size <SMMI> <value>          - Set pkt size start address
        or  range <portlist> size <start> <min> <max> <inc>
    range <portlist> teid <SMMI> <value>          - Set TEID value
        or  range <portlist> teid <start> <min> <max> <inc>
    range <portlist> mpls entry <hex-value>       - Set MPLS entry value
    range <portlist> qinq index <val1> <val2>     - Set QinQ index values
    range <portlist> gre key <value>              - Set GRE key value
    range <portlist> cos <SMMI> <value>           - Set cos value
    range <portlist> tos <SMMI> <value>           - Set tos value

The ``sequence`` commands::

    sequence <seq#> <portlist> dst <Mac> src <Mac> dst <IP> src <IP> sport <val> dport <val> ipv4|ipv6 udp|tcp|icmp vlan <val> size <val> [teid <val>]
    sequence <seq#> <portlist> <dst-Mac> <src-Mac> <dst-IP> <src-IP> <sport> <dport> ipv4|ipv6 udp|tcp|icmp <vlanid> <pktsize> [<teid>]
    sequence <seq#> <portlist> cos <cos> tos <tos>
    sequence <seq#> <portlist> vxlan <flags> gid <group_id> vid <vxlan_id>
                                    - Set the sequence packet information, make sure the src-IP
                                        has the netmask value eg 1.2.3.4/24


The ``pcap`` commands::

    pcap show                          - Show PCAP information
    pcap index                         - Move the PCAP file index to the given packet number,  0 - rewind, -1 - end of file
    pcap filter <portlist> <string>    - PCAP filter string to filter packets on receive

The ``start|stop`` commands::

    start <portlist>                   - Start transmitting packets
    stop <portlist>                    - Stop transmitting packets
    stp                                - Stop all ports from transmitting
    str                                - Start all ports transmitting
    start <portlist> prime             - Transmit packets on each port listed. See set prime command above
    start <portlist> arp <type>        - Send a ARP type packet
       type - request | gratuitous | req | grat

The ``debug`` commands::
    dbg l2p                            - Dump out internal lcore to port mapping
    dbg tx_dbg                         - Enable tx debug output
    dbg mempool <portlist> <type>      - Dump out the mempool info for a given type
    dbg pdump <portlist>               - Hex dump the first packet to be sent, single packet mode only
    dbg memzone                        - List all of the current memzones
    dbg memseg                         - List all of the current memsegs
    dbg hexdump <addr> <len>           - hex dump memory at given address
    dbg break                          - break into the debugger
    dbg memcpy [loop-cnt KBytes]       - run a memcpy test

The odd or special commands::

    save <path-to-file>                - Save a configuration file using the filename
    load <path-to-file>                - Load a command/script file from the given path
    script <filename>                  - Execute the Lua script code in file (www.lua.org). (if Lua is enabled)
    lua 'lua string'                   - Execute the Lua code in the string needs quotes (if Lua is enabled)
    clear <portlist> stats             - Clear the statistics
    clr                                - Clear all Statistices
    reset <portlist>                   - Reset the configuration the ports to the default
    rst                                - Reset the configuration for all ports
    ports per page [1-6]               - Set the number of ports displayed per page
    port <number>                      - Sets the sequence packets to display for a given port
    restart <portlist>                 - Restart or stop a ethernet port and restart
    ping4 <portlist>                   - Send a IPv4 ICMP echo request on the given portlist

The ``theme`` commands::
    theme <item> <fg> <bg> <attr>      - Set color for item with fg/bg color and attribute value
    theme show                         - List the item strings, colors and attributes to the items
    theme save <filename>              - Save the current color theme to a file

The ``plugin`` commands::
    plugin                             - Show the plugins currently installed
    plugin load <filename>             - Load a plugin file
    plugin load <filename> <path>      - Load a plugin file at path
    plugin rm|delete <plugin>          - Remove or delete a plugin

The ``rate` commands for packet pacing::

    rate <portlist> count <value>        - number of packets to transmit
    rate <portlist> size <value>         - size of the packet to transmit
    rate <portlist> rate <percent>       - Packet rate in percentage
    rate <portlist> burst <value>        - number of packets in a burst
    rate <portlist> sport <value>        - Source port number for TCP
    rate <portlist> dport <value>        - Destination port number for TCP
    rate <portlist> ttl <value>          - Set the TTL value for the single port more
    rate <portlist> src|dst mac <addr>   - Set MAC addresses 00:11:22:33:44:55 or 0011:2233:4455 format
    rate <portlist> type ipv4|ipv6|vlan|arp - Set the packet type to IPv4 or IPv6 or VLAN
    rate <portlist> proto udp|tcp|icmp   - Set the packet protocol to UDP or TCP or ICMP per port
    rate <portlist> [src|dst] ip ipaddr  - Set IP addresses, Source must include network mask e.g. 10.1.2.3/24
    rate <portlist> fps <value>          - Set the frame per second value e.g. 60fps
    rate <portlist> lines <value>        - Set the number of video lines, e.g. 720
    rate <portlist> pixels <value>       - Set the number of pixels per line, e.g. 1280
    rate <portlist> color bits <value>   - Set the color bit size 8, 16, 24, ...
    rate <portlist> payload size <value> - Set the payload size
    rate <portlist> overhead <value>     - Set the packet overhead + payload = total packet size

The flags::

        Flags: P------------------ - Promiscuous mode enabled
                E                  - ICMP Echo enabled
                 B                 - Bonding enabled LACP 802.3ad
                  I                - Process packets on input enabled
                   *               - Using TAP interface for this port can be [-rt*]
                    g              - Process GARP packets
                     C             - Capture received packets
                      ------       - Modes Single, pcap, sequence, latency, random, Rate
                            ------ - Modes VLAN, VxLAN, MPLS, QnQ, GRE IPv4, GRE ETH
    Notes: <state>    - Use enable|disable or on|off to set the state.
        <portlist>    - a list of ports (no spaces) as 2,4,6-9,12 or 3-5,8 or 5 or the word 'all'
        Colors best seen on a black background for now


Several commands take common arguments such as:

* ``portlist``: A list of ports such as ``2,4,6-9,12`` or the word ``all``.
* ``state``: This is usually ``on`` or ``off`` but will also accept ``enable``
  or ``disable``.

For example::

   Pktgen:/> set all seq_cnt 1


The ``set`` command can also be used to set the MAC address with a format like
``00:11:22:33:44:55`` or ``0011:2233:4455``::

   set <portlist> src|dst mac etheraddr

The ``set`` command can also be used to set IP addresses::

   set <portlist> src|dst ip ipaddr


seq
---

The ``seq`` command sets the flow parameters for a sequence of packets::

   seq <seq#> <portlist> dst-Mac src-Mac dst-IP src-IP
                         sport dport ipv4|ipv6|vlan udp|tcp|icmp vid pktsize

Where the arguments are:

  * ``<seq#>``: The packet sequence number.
  * ``<portlist>``: A portlist as explained above.
  * ``dst-Mac``: The destination MAC address.
  * ``src-Mac``: The source MAC address.
  * ``dst-IP``: The destination IP address.
  * ``src-IP``: The source IP address. Make sure the src-IP has the netmask value such as ``1.2.3.4/24``.
  * ``sport``: The source port.
  * ``dport``: The destination port.
  * ``IP``: The IP layer. One of ``ipv4|ipv6|vlan``.
  * ``Transport``: The transport. One of ``udp|tcp|icmp``.
  * ``vid``: The VLAN ID.
  * ``pktsize``: The packet size.


save
----

The ``save`` command saves the current configuration of a file::

   save <path-to-file>


load
----

The ``load`` command loads a configuration from a file::

   load <path-to-file>

The is most often used with a configuration file written with the ``save``
command, see above.


ports per page
--------------

The ``ports per page`` (ports per page) command sets the number of ports displayed per
page::

   ports per page [1-6]


script
------

The ``script`` command execute the Lua code in specified file::

   script <filename>

See :ref:`scripts`.


pages
-----

The Random or rnd page.
::

  Port 0           <Random bitfield Page>  Copyright(c) <2010-2023>, Intel Corporation
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
       10       0      No   00000000 00000000 00000000 00000000
       11       0      No   00000000 00000000 00000000 00000000
       12       0      No   00000000 00000000 00000000 00000000
       13       0      No   00000000 00000000 00000000 00000000
       14       0      No   00000000 00000000 00000000 00000000
       15       0      No   00000000 00000000 00000000 00000000
       16       0      No   00000000 00000000 00000000 00000000
       17       0      No   00000000 00000000 00000000 00000000
       18       0      No   00000000 00000000 00000000 00000000
       19       0      No   00000000 00000000 00000000 00000000
       20       0      No   00000000 00000000 00000000 00000000
       21       0      No   00000000 00000000 00000000 00000000
       22       0      No   00000000 00000000 00000000 00000000
       23       0      No   00000000 00000000 00000000 00000000
       24       0      No   00000000 00000000 00000000 00000000
       25       0      No   00000000 00000000 00000000 00000000
       26       0      No   00000000 00000000 00000000 00000000
       27       0      No   00000000 00000000 00000000 00000000
       28       0      No   00000000 00000000 00000000 00000000
       29       0      No   00000000 00000000 00000000 00000000
       30       0      No   00000000 00000000 00000000 00000000
       31       0      No   00000000 00000000 00000000 00000000
       -- Pktgen Ver: 3.2.4 (DPDK 17.05.0-rc0)  Powered by DPDK -----

The sequence or seq page.
::

	<Sequence Page>  Copyright(c) <2010-2023>, Intel Corporation
	  Port   :  0, Sequence Count:  8 of 16                                                                            GTPu
	    * Seq:            Dst MAC           Src MAC          Dst IP            Src IP    Port S/D Protocol:VLAN  Size  TEID
	    *   0:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
	    *   1:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
	    *   2:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
	    *   3:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
	    *   4:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
	    *   5:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
	    *   6:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0
	    *   7:  3c:fd:fe:9c:5c:d9 3c:fd:fe:9c:5c:d8     192.168.1.1    192.168.0.1/24   1234/5678 IPv4/TCP:0001   64     0

	    -- Pktgen Ver: 3.2.4 (DPDK 17.05.0-rc0)  Powered by DPDK ---------------

The CPU information page.
::

	<CPU Page>  Copyright(c) <2010-2023>, Intel Corporation

	Kernel: Linux rkwiles-DESK1.intel.com 4.4.0-66-generic #87-Ubuntu SMP Fri Mar 3 15:29:05 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux

	Model Name: Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz
	CPU Speed : 1201.031
	Cache Size: 46080 KB

	CPU Flags : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid dca sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm epb tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid cqm xsaveopt cqm_llc cqm_occup_llc dtherm ida arat pln pts
	  2 sockets, 18 cores per socket and 2 threads per core.
	  Socket   :    0         1
	  Core   0 : [ 0,36]   [18,54]
	  Core   1 : [ 1,37]   [19,55]
	  Core   2 : [ 2,38]   [20,56]
	  Core   3 : [ 3,39]   [21,57]
	  Core   4 : [ 4,40]   [22,58]
	  Core   5 : [ 5,41]   [23,59]
	  Core   6 : [ 6,42]   [24,60]
	  Core   7 : [ 7,43]   [25,61]
	  Core   8 : [ 8,44]   [26,62]
	  Core   9 : [ 9,45]   [27,63]
	  Core  10 : [10,46]   [28,64]
	  Core  11 : [11,47]   [29,65]
	  Core  12 : [12,48]   [30,66]
	  Core  13 : [13,49]   [31,67]
	  Core  14 : [14,50]   [32,68]
	  Core  15 : [15,51]   [33,69]
	  Core  16 : [16,52]   [34,70]
	  Core  17 : [17,53]   [35,71]

The latency page.
::

	-- Ports 0-3 of 8   <Main Page>  Copyright(c) <2010-2023>, Intel Corporation
		Flags:Port         :   P----S---------:0   P--------------:1   P--------------:2   P--------------:3
		Link State         :       <UP-10000-FD>       <UP-10000-FD>       <UP-10000-FD>       <UP-10000-FD>     ----TotalRate----
		Pkts/s Max/Rx      :                 0/0                 0/0                 0/0                 0/0                   0/0
		       Max/Tx      :                 0/0                 0/0                 0/0                 0/0                   0/0
		MBits/s Rx/Tx      :                 0/0                 0/0                 0/0                 0/0                   0/0
		                   :
		Latency usec       :                   0                   0                   0                   0
		Jitter Threshold   :                  50                  50                  50                  50
		Jitter count       :                   0                   0                   0                   0
		Total Rx pkts      :                   0                   0                   0                   0
		Jitter percent     :                   0                   0                   0                   0
		                   :
		Pattern Type       :             abcd...             abcd...             abcd...             abcd...
		Tx Count/% Rate    :       Forever /100%       Forever /100%       Forever /100%       Forever /100%
		PktSize/Rx:Tx Burst:         64 / 32: 64           64 /   32           64 /   32           64 /   32
		Src/Dest Port      :         1234 / 5678         1234 / 5678         1234 / 5678         1234 / 5678
		Pkt Type:VLAN ID   :     IPv4 / TCP:0001     IPv4 / TCP:0001     IPv4 / TCP:0001     IPv4 / TCP:0001
		Dst  IP Address    :         192.168.1.1         192.168.0.1         192.168.3.1         192.168.2.1
		Src  IP Address    :      192.168.0.1/24      192.168.1.1/24      192.168.2.1/24      192.168.3.1/24
		Dst MAC Address    :   3c:fd:fe:9c:5c:d9   3c:fd:fe:9c:5c:d8   3c:fd:fe:9c:5c:db   3c:fd:fe:9c:5c:da
		Src MAC Address    :   3c:fd:fe:9c:5c:d8   3c:fd:fe:9c:5c:d9   3c:fd:fe:9c:5c:da   3c:fd:fe:9c:5c:db
		VendID/PCI Addr    :   8086:1572/04:00.0   8086:1572/04:00.1   8086:1572/04:00.2   8086:1572/04:00.3

		-- Pktgen Ver: 3.2.4 (DPDK 17.05.0-rc0)  Powered by DPDK ---------------

The config or cfg page.
::

	<CPU Page>  Copyright(c) <2010-2023>, Intel Corporation
	 2 sockets, 18 cores, 2 threads
	  Socket   :    0         1      Port description
	  Core   0 : [ 0,36]   [18,54]   0000:04:00.0 : Intel Corporation X710 for 10GbE SFP+ (rev 01)
	  Core   1 : [ 1,37]   [19,55]   0000:04:00.1 : Intel Corporation X710 for 10GbE SFP+ (rev 01)
	  Core   2 : [ 2,38]   [20,56]   0000:04:00.2 : Intel Corporation X710 for 10GbE SFP+ (rev 01)
	  Core   3 : [ 3,39]   [21,57]   0000:04:00.3 : Intel Corporation X710 for 10GbE SFP+ (rev 01)
	  Core   4 : [ 4,40]   [22,58]   0000:05:00.0 : Intel Corporation I350 Gigabit Network Connection (rev 01)
	  Core   5 : [ 5,41]   [23,59]   0000:05:00.1 : Intel Corporation I350 Gigabit Network Connection (rev 01)
	  Core   6 : [ 6,42]   [24,60]   0000:81:00.0 : Intel Corporation X710 for 10GbE SFP+ (rev 01)
	  Core   7 : [ 7,43]   [25,61]   0000:81:00.1 : Intel Corporation X710 for 10GbE SFP+ (rev 01)
	  Core   8 : [ 8,44]   [26,62]   0000:81:00.2 : Intel Corporation X710 for 10GbE SFP+ (rev 01)
	  Core   9 : [ 9,45]   [27,63]   0000:81:00.3 : Intel Corporation X710 for 10GbE SFP+ (rev 01)
	  Core  10 : [10,46]   [28,64]   0000:82:00.0 : Intel Corporation XL710 for 40GbE QSFP+ (rev 02)
	  Core  11 : [11,47]   [29,65]   0000:83:00.0 : Intel Corporation XL710 for 40GbE QSFP+ (rev 02)
	  Core  12 : [12,48]   [30,66]
	  Core  13 : [13,49]   [31,67]
	  Core  14 : [14,50]   [32,68]
	  Core  15 : [15,51]   [33,69]
	  Core  16 : [16,52]   [34,70]
	  Core  17 : [17,53]   [35,71]

	  -- Pktgen Ver: 3.2.4 (DPDK 17.05.0-rc0)  Powered by DPDK ---------------


Here is the ``page range`` screen.
::

	    Port #                           Port-0              Port-1              Port-2              Port-3
	    dst.ip            :         192.168.1.1         192.168.2.1         192.168.3.1         192.168.4.1
	        inc           :             0.0.0.1             0.0.0.1             0.0.0.1             0.0.0.1
	        min           :         192.168.1.1         192.168.2.1         192.168.3.1         192.168.4.1
	        max           :       192.168.1.254       192.168.2.254       192.168.3.254       192.168.4.254
	                      :
	    src.ip            :         192.168.0.1         192.168.1.1         192.168.2.1         192.168.3.1
	        inc           :             0.0.0.0             0.0.0.0             0.0.0.0             0.0.0.0
	        min           :         192.168.0.1         192.168.1.1         192.168.2.1         192.168.3.1
	        max           :       192.168.0.254       192.168.1.254       192.168.2.254       192.168.3.254
	                      :
	    ip_proto          :                 TCP                 TCP                 TCP                 TCP
	                      :
	    dst.port / inc    :             0/    1           256/    1           512/    1           768/    1
	         min / max    :             0/  254           256/  510           512/  766           768/ 1022
	                      :
	    src.port / inc    :             0/    1           256/    1           512/    1           768/    1
	         min / max    :             0/  254           256/  510           512/  766           768/ 1022
	                      :
	    vlan.id / inc     :              1/   0              1/   0              1/   0              1/   0
	        min / max     :              1/4095              1/4095              1/4095              1/4095
	                      :
	    pkt.size / inc    :             64/   0             64/   0             64/   0             64/   0
	         min / max    :             64/1518             64/1518             64/1518             64/1518
	                      :
	    dst.mac           :   3c:fd:fe:9c:5c:d9   3c:fd:fe:9c:5c:d8   3c:fd:fe:9c:5c:db   3c:fd:fe:9c:5c:da
	        inc           :   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00
	        min           :   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00
	        max           :   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00
	                      :
	    src.mac           :   3c:fd:fe:9c:5c:d8   3c:fd:fe:9c:5c:d9   3c:fd:fe:9c:5c:da   3c:fd:fe:9c:5c:db
	        inc           :   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00
	        min           :   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00
	        max           :   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00   00:00:00:00:00:00
	                      :
	    gtpu.teid / inc   :             0/    0             0/    0             0/    0             0/    0
	          min / max   :             0/    0             0/    0             0/    0             0/    0
	    -- Pktgen Ver: 3.2.4 (DPDK 17.05.0-rc0)  Powered by DPDK ---------------

	    Pktgen:/>

s
