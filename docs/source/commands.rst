.. _commands:

Runtime Options and Commands
============================

While the ``pktgen`` application is running you will see a command prompt as
follows::

   Pktgen>

From this you can get help or issue runtime commands::

   Pktgen> help

   set <portlist> <xxx> value    - Set a few port values
   save <path-to-file>           - Save a configuration file using the
                                   filename
   load <path-to-file>           - Load a command/script file from the
                                   given path
   ...


The runtime commands are explained below.

Several commands take common arguments such as:

* ``portlist``: A list of ports such as ``2,4,6-9,12`` or the word ``all``.
* ``state``: This is usually ``on`` or ``off`` but will also accept ``enable``
  or ``disable``.


set
---

The ``set`` command is used to set values for ports::

   set <portlist> <command> value


* ``<portlist>``: A portlist as explained above.
* ``<command>``: one of the following:

  * ``count``: Number of packets to transmit.
  * ``size``: Size of the packet to transmit.
  * ``rate``: Packet rate in percentage.
  * ``burst``: Number of packets in a burst.
  * ``sport``: Source port number for TCP.
  * ``dport``: Destination port number for TCP.
  * ``prime``: Set the number of packets to send on prime command.
  * ``seqCnt``: Set the number of packet in the sequence to send.
  * ``dump``: Dump the next <value> received packets to the screen.

For example::

   Pktgen> set all seqCnt 1


The ``set`` command can also be used to set the MAC address with a format like
``00:11:22:33:44:55`` or ``0011:2233:4455``::

   set mac <portlist> etheraddr

The ``set`` command can also be used to set IP addresses::

   set ip src|dst <portlist> ipaddr


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
* ``src-IP``: The source IP address. Make sure the src-IP has the netmask
  value such as ``1.2.3.4/24``.
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


ppp
---

The ``ppp`` (ports per page) command sets the number of ports displayed per
page::

   ppp [1-6]


icmp.echo
---------

The ``icmp.echo`` command enables or disables ICMP echo responses per port::

   icmp.echo <portlist> <state>

The ``state`` variable is explained above.


send
----

The ``send`` command sends a ARP request or gratuitous ARP on a set of ports::

   send arp req|grat <portlist>


mac_from_arp
------------

The ``mac_from_arp`` command sets the option to get the MAC from an ARP
request::

   mac_from_arp <state>


proto
-----

The ``proto`` command sets the packet protocol to UDP or TCP or ICMP per
port::

   proto udp|tcp|icmp <portlist>


type
----

The ``type`` command sets the packet type to IPv4 or IPv6 or VLAN::

   type ipv4|ipv6|vlan <portlist>


geometry
--------

The ``geometry`` command sets the display geometry in columns by rows
(colxrow)::

   geometry <geom>


capture
-------

The ``capture`` command enables/disables packet capturing on a portlist::

   capture <portlist> <state>


rxtap
-----

The ``rxtap`` command enables/disables the Rx tap interface. It support
pg_rxtapN::

   rxtap <portlist> <state>


txtap
-----

The ``txtap`` command enables/disables Tx the tap interface. It support
pg_txtapN::

   txtap <portlist> <state>


vlan
----

The ``vlan`` command enables/disables sending VLAN ID in packets::

   vlan <portlist> <state>


vlanid
------

The ``vlanid`` command sets the VLAN ID for the portlist::

   vlanid <portlist> <vlanid>


mpls
----

The ``mpls`` command enables/disables sending an MPLS entry in packets::

   mpls <portlist> <state>


mpls_entry
----------

The ``mpls_entry`` command sets the MPLS (Multiprotocol Label Switching) entry
for the portlist (must be specified in hex)::

   mpls_entry <portlist> <entry>


qinq
----

The ``qinq`` command enables/disables sending a Q-in-Q header in packets::

   qinq <portlist> <state>


qinqids
-------

The ``qinqids`` command sets the Q-in-Q ID's for the portlist::

   qinqids <portlist> <id1> <id2>


gre
---

The ``gre`` command enables/disables GRE (Generic Routing Encapsulation) with
IPv4 payload::

   gre <portlist> <state>


gre_eth
-------

The ``gre_eth`` command enables/disables GRE with Ethernet frame payload::

   gre_eth <portlist> <state>


gre_key
-------

The ``gre_key`` command sets the GRE key::

   gre_key <portlist> <state>


pcap
----

The ``pcap`` command enables or disable sending pcap packets on a portlist::

   pcap <portlist> <state>


pcap.show
---------

The ``pcap.show`` command shows the PCAP information::

   pcap.show


pcap.index
----------

The ``pcap.index`` command moves the PCAP file index to the given packet
number::

   pcap.index

Where:

* 0 = rewind.
* -1 = end of file.


pcap.filter
-----------

The ``pcap.filter`` command sets the PCAP filter string to filter packets on
receive::

   pcap.filter <portlist> <string>


script
------

The ``script`` command execute the Lua code in specified file::

   script <filename>

See :ref:`scripts`.


ping4
-----

The ``ping4`` command sends a IPv4 ICMP echo request on the given portlist::

   ping4 <portlist>


page
----

The ``page`` command shows the port pages or configuration or sequence page::

   page [0-7]|main|range|config|seq|pcap|next|cpu|rnd


Where:

* ``[0-7]``: Page of different ports.
* ``main``: Display page zero.
* ``range``: Display the range packet page.
* ``config``: Display the configuration page (reserved, not used).
* ``pcap``: Display the pcap page.
* ``cpu``: Display some information about the system CPU.
* ``next``: Display next page of PCAP packets.
* ``sequence|seq``: Display a set of packets for a given port. Note: use the
  ``port`` command, see below, to display a new port sequence.
* ``rnd``: Display the random bitfields of packets for a given port. Note: use
  the ``port`` command, see below, to display a new port sequence.
* ``log``: Display the log messages page.


port
----

The ``port`` command sets the sequence of packets to display for a given
port::

   port <number>


process
-------

The ``process`` command enables or disables processing of ARP/ICMP/IPv4/IPv6
packets::

   process <portlist> <state>


garp
----

The ``garp`` command enables or disables Gratuitous ARP packet processing and
update MAC address::

   garp <portlist> <state>


blink
-----

The ``blink`` command blinks the link led on the given port list::

   blink <portlist> <state>


rnd
---

The ``rnd`` command sets random mask for all transmitted packets from
portlist::

   rnd <portlist> <idx> <off> <mask>

Where:

* ``idx``: random mask slot.
* ``off``: offset in packets, where to apply mask.
* ``mask``: up to 32 bit long mask specification (empty to disable):

  * ``0``: bit will be 0.
  * ``1``: bit will be 1.
  * ``.``: bit will be ignored (original value is retained).
  * ``X``: bit will get random value.


theme
-----

The ``theme`` command enables or disables the theme::

   theme <state>

It also sets the color for item with foreground (fg) or background (bg) color
and attribute value::

   theme <item> <fg> <bg> <attr>


theme.show
----------

The ``theme.show`` command lists the item strings, colors and attributes to the
items::

   theme.show


theme.save
----------

The ``theme.save`` command saves the current color theme to a file::

   theme.save <filename>


start
-----

The ``start`` command starts transmitting packets::

   start <portlist>


stop
----

The ``stop`` command stops transmitting packets::

   stop <portlist>


str
---

The ``str`` command starts all ports transmitting::

   str

A shortcut for ``start all``.


stp
---

The ``stp`` command stops all ports from transmitting::

   stp

A shortcut for ``stop all``.


screen
------

The ``screen`` command stops/starts updating the screen and unlocks/locks the
window::

   screen stop|start


off
---

The ``off`` command is a ``screen off`` shortcut::

   off


on
--

The ``on`` command ``screen on`` shortcut::

   on


prime
-----

The ``prime`` command transmits N packets on each port listed. See set prime
command above::

   prime <portlist>


delay
-----

The ``delay`` command waits a number of milliseconds before reading or
executing scripting commands::

   delay milliseconds


sleep
-----

The ``sleep`` command waits a number of seconds before reading or executing
scripting commands::

   sleep seconds


dev.list
--------

The ``dev.list`` command shows the whitelist/blacklist/Virtual devices::

   dev.list


pci.list
--------

The ``pci.list`` command shows all the PCI devices::

   pci.list


clear
-----

The ``clear`` command clears the statistics::

   clear <portlist>


clr
---

The ``clr`` command clears all statistics::

   clr

A shortcut for ``clear all``.


cls
---

The ``cls`` command clears the screen::

   cls

A shortcut for ``clear all``.


reset
-----

The ``reset`` command resets the configuration to the default::

   reset <portlist>


rst
---

The ``rst`` command resets the configuration for all ports::

   rst

A shortcut for ``reset all``.


help
----

The ``help`` command displays this help for runtime commands::

   help


quit
----

The ``quit`` command quits the Pktgen program::

   quit


dst.mac
-------

The ``dst.mac`` command sets the destination MAC address start::

   dst.mac start <portlist> etheraddr


src.mac
-------

The ``src.mac`` command sets the source MAC address start::

   src.mac start <portlist> etheraddr


src.ip
------

The ``src.ip`` command sets the source IP address properties:

* ``start``: The start of the range.
* ``min``: The minimum value in range.
* ``max`` The maximum value in range
* ``inc``: The increment.


For example::

   src.ip start <portlist> ipaddr
   src.ip min <portlist> ipaddr
   src.ip max <portlist> ipaddr
   src.ip inc <portlist> ipaddr


dst.ip
------

The ``dst.ip`` command sets the destination IP address properties:

* ``start``: The start of the range.
* ``min``: The minimum value in range.
* ``max`` The maximum value in range
* ``inc``: The increment.


For example::

   dst.ip start <portlist> ipaddr
   dst.ip min <portlist> ipaddr
   dst.ip max <portlist> ipaddr
   dst.ip inc <portlist> ipaddr


src.port
--------

The ``src.port`` command sets the source port address properties:

* ``start``: The start of the range.
* ``min``: The minimum value in range.
* ``max`` The maximum value in range
* ``inc``: The increment.


For example::

   src.port start <portlist> value
   src.port min <portlist> value
   src.port max <portlist> value
   src.port inc <portlist> value


dst.port
--------

The ``dst.port`` command sets the source port address properties:

* ``start``: The start of the range.
* ``min``: The minimum value in range.
* ``max`` The maximum value in range
* ``inc``: The increment.


For example::

   dst.port start <portlist> value
   dst.port min <portlist> value
   dst.port max <portlist> value
   dst.port inc <portlist> value


vlan.id
-------

The ``vlan.id`` command sets the vlan id address properties:

* ``start``: The start of the range.
* ``min``: The minimum value in range.
* ``max`` The maximum value in range
* ``inc``: The increment.


For example::

   vlan.id start <portlist> value
   vlan.id min <portlist> value
   vlan.id max <portlist> value
   vlan.id inc <portlist> value


pkt.size
--------

The ``pkt.size`` command sets the packet size properties:


* ``start``: The start of the range.
* ``min``: The minimum value in range.
* ``max`` The maximum value in range
* ``inc``: The increment.


For example::


   pkt.size start <portlist> value
   pkt.size min <portlist> value
   pkt.size max <portlist> value
   pkt.size inc <portlist> value


range
-----

The ``range`` command enables or disables the given portlist for sending a
range of packets::

   range <portlist> <state>
