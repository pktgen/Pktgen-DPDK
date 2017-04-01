/*-
 * Copyright (c) <2010>, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of Intel Corporation nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <stdlib.h>

#include "pktgen-help.h"

/**********************************************************/
const char *help_info[] = {
	"",						/* Leave blank not used */
	"   *** Help Information for Pktgen ***\n",	/* leave newline, Copyright is below this line. */
	"set <portlist> <xxx> value         - Set a few port values",
	"  <portlist>                       - a list of ports as 2,4,6-9,12 or the word 'all'",
	"  <xxx>          count             - number of packets to transmit",
	"                 size              - size of the packet to transmit",
	"                 rate              - Packet rate in percentage",
	"                 burst             - number of packets in a burst",
	"                 sport             - Source port number for TCP",
	"                 dport             - Destination port number for TCP",
	"                 prime             - Set the number of packets to send on prime command",
	"                 seqCnt            - Set the number of packet in the sequence to send",
	"                 dump              - Dump the next <value> received packets to the screen",
	"                 vlanid            - Set the VLAN ID value for the portlist",
	"                 random            - Enable or disable random mode\n",
	"pattern <type>                     - Fill Pattern type",
	"        abc                        - Default pattern of abc string",
	"        none                       - No fill pattern, maybe random data",
	"        zero                       - Fill of zero bytes",
	"        user                       - User supplied string of max 16 bytes",
	"user.pattern \"string\"              - A 16 byte string, must set 'pattern user' command",
	"latency <portlist> <state>         - Enable Latency testing",
	"jitter <portlist> <usec>           - Set the jitter threshold in micro-seconds",
	"seq <seq#> <portlist> dst-Mac src-Mac dst-IP src-IP sport dport ipv4|ipv6 udp|tcp|icmp vlan pktsize",
	"                                   - Set the sequence packet information, make sure the src-IP",
	"                                     has the netmask value eg 1.2.3.4/24",
	"save <path-to-file>                - Save a configuration file using the filename",
	"load <path-to-file>                - Load a command/script file from the given path",
        "script <filename>                  - Execute the Lua script code in file (www.lua.org).",
        "!lua 'lua string'                  - Execute the Lua code in the string needs quotes",
	"ppp [1-6]                          - Set the number of ports displayed per page",
	"icmp.echo <portlist> <state>       - Enable/disable ICMP echo responses per port",
	"send arp req|grat <portlist>       - Send a ARP request or gratuitous ARP on a set of ports",
	"set mac <portlist> etheraddr       - Set MAC addresses 00:11:22:33:44:55",
	"                                     You can use 0011:2233:4455 format as well",
	"",
	"<<PageBreak>>",
	"mac_from_arp <state>               - Set the option to get MAC from ARP request",
	"proto udp|tcp|icmp <portlist>      - Set the packet protocol to UDP or TCP or ICMP per port",
	"type ipv4|ipv6|vlan <portlist>     - Set the packet type to IPv4 or IPv6 or VLAN",
	"set ip src|dst <portlist> ipaddr   - Set IP addresses",
	"geometry <geom>                    - Set the display geometry Columns by Rows (ColxRow)",
	"capture <portlist> <state>         - Enable/disable packet capturing on a portlist",
	"rxtap <portlist> <state>           - Enable/disable Rx tap interface support pg_rxtapN",
	"txtap <portlist> <state>           - Enable/disable Tx tap interface support pg_txtapN",
	"vlan <portlist> <state>            - Enable/disable sending VLAN ID in packets",
	"vlanid <portlist> <vlanid>         - Set the VLAN ID for the portlist, same as 'set 0 vlanid 5'",
	"mpls <portlist> <state>            - Enable/disable sending MPLS entry in packets",
	"mpls_entry <portlist> <entry>      - Set the MPLS entry for the portlist (must be specified in hex)",
	"qinq <portlist> <state>            - Enable/disable sending Q-in-Q header in packets",
	"qinqids <portlist> <id1> <id2>     - Set the Q-in-Q ID's for the portlist",
	"pdump <portlist>                   - Hex dump the first packet to be sent, single packet mode only",
	"",
	"<<PageBreak>>",
	"gre <portlist> <state>             - Enable/disable GRE with IPv4 payload",
	"gre_eth <portlist> <state>         - Enable/disable GRE with Ethernet frame payload",
	"gre_key <portlist> <state>         - Set the GRE key",
	"pcap <portlist> <state>            - Enable or Disable sending pcap packets on a portlist",
	"pcap.show                          - Show the PCAP information",
	"pcap.index                         - Move the PCAP file index to the given packet number,  0 - rewind, -1 - end of file",
	"pcap.filter <portlist> <string>    - PCAP filter string to filter packets on receive",
	"ping4 <portlist>                   - Send a IPv4 ICMP echo request on the given portlist",
#ifdef INCLUDE_PING6
	"ping6 <portlist>                   - Send a IPv6 ICMP echo request on the given portlist",
#endif
	"page <pages>                       - Show the port pages or configuration or sequence page",
	"     [0-7]                         - Page of different ports",
	"     main                          - Display page zero",
	"     range                         - Display the range packet page",
	"     config                        - Display the configuration page (not used)",
	"     pcap                          - Display the pcap page",
	"     cpu                           - Display some information about the CPU system",
	"     next                          - Display next page of PCAP packets.",
	"     sequence | seq                - sequence will display a set of packets for a given port",
	"                                     Note: use the 'port <number>' to display a new port sequence",
	"     rnd                           - Display the random bitfields to packets for a given port",
	"                                     Note: use the 'port <number>' to display a new port sequence",
	"     log                           - Display the log messages page",
	"     latency                       - Display the latency page",
        "     stats                         - Display physical ports stats for all ports",
	"port <number>                      - Sets the sequence of packets to display for a given port",
	"process <portlist> <state>         - Enable or Disable processing of ARP/ICMP/IPv4/IPv6 packets",
	"garp <portlist> <state>            - Enable or Disable GARP packet processing and update MAC address",
	"blink <portlist> <state>           - Blink the link led on the given port list",
	"",
	"<<PageBreak>>",
	"rnd <portlist> <idx> <off> <mask>  - Set random mask for all transmitted packets from portlist",
	"                                     idx: random mask slot",
	"                                     off: offset in packets, where to apply mask",
	"                                     mask: up to 32 bit long mask specification (empty to disable):",
	"                                       0: bit will be 0",
	"                                       1: bit will be 1",
	"                                       .: bit will be ignored (original value is retained)",
	"                                       X: bit will get random value",
	"theme <state>                      - Enable or Disable the theme",
	"theme <item> <fg> <bg> <attr>      - Set color for item with fg/bg color and attribute value",
	"theme.show                         - List the item strings, colors and attributes to the items",
	"theme.save <filename>              - Save the current color theme to a file",
	"start <portlist>                   - Start transmitting packets",
	"stop <portlist>                    - Stop transmitting packets",
	"stp                                - Stop all ports from transmitting",
	"str                                - Start all ports transmitting",
	"",
	"screen stop|start                  - stop/start updating the screen and unlock/lock window",
	"off                                - screen off shortcut",
	"on                                 - screen on shortcut",
	"prime <portlist>                   - Transmit N packets on each port listed. See set prime command above",
	"delay milliseconds                 - Wait a number of milliseconds for scripting commands",
	"sleep seconds                      - Wait a number of seconds for scripting commands",
	"dev.list                           - Show the device whitelist/blacklist/Virtual",
	"pci.list                           - Show all the PCI devices",
	"clear <portlist>                   - Clear the statistics",
	"clr                                - Clear all Statistices",
	"cls                                - Clear the screen",
	"reset <portlist>                   - Reset the configuration to the default",
	"rst                                - Reset the configuration for all ports",
	"port.restart <portlist>            - Restart or stop a ethernet port and restart",
	"help                               - Display this help message",
	"quit                               - Quit the Pktgen program",
	"",
	"<<PageBreak>>",
	"  -- Setup the packet range values --",
	"dst.mac start <portlist> etheraddr - Set destination MAC address start",
	"src.mac start <portlist> etheraddr - Set source MAC address start",
	"src.ip start <portlist> ipaddr     - Set source IP start address",
	"src.ip min <portlist> ipaddr       - Set source IP minimum address",
	"src.ip max <portlist> ipaddr       - Set source IP maximum address",
	"src.ip inc <portlist> ipaddr       - Set source IP increment address",
	"dst.ip start <portlist> ipaddr     - Set destination IP start address",
	"dst.ip min <portlist> ipaddr       - Set destination IP minimum address",
	"dst.ip max <portlist> ipaddr       - Set destination IP maximum address",
	"dst.ip inc <portlist> ipaddr       - Set destination IP increment address",
	"ip.proto <portlist> [tcp|udp]      - Set the IP protocol type (alias range.proto)",
	"src.port start <portlist> value    - Set source port start address",
	"src.port min <portlist> value      - Set source port minimum address",
	"src.port max <portlist> value      - Set source port maximum address",
	"src.port inc <portlist> value      - Set source port increment address",
	"dst.port start <portlist> value    - Set destination port start address",
	"dst.port min <portlist> value      - Set destination port minimum address",
	"dst.port max <portlist> value      - Set destination port maximum address",
	"dst.port inc <portlist> value      - Set destination port increment address",
	"vlan.id start <portlist> value     - Set vlan id start address",
	"vlan.id min <portlist> value       - Set vlan id minimum address",
	"vlan.id max <portlist> value       - Set vlan id maximum address",
	"vlan.id inc <portlist> value       - Set vlan id increment address",
	"pkt.size start <portlist> value    - Set pkt size start address",
	"pkt.size min <portlist> value      - Set pkt size minimum address",
	"pkt.size max <portlist> value      - Set pkt size maximum address",
	"pkt.size inc <portlist> value      - Set pkt size increment address",
	"range <portlist> <state>           - Enable or Disable the given portlist for sending a range of packets",
	"",
	"<<PageBreak>>",
	"       Flags: P---------------- - Promiscuous mode enabled",
	"               E                - ICMP Echo enabled",
	"                A               - Send ARP Request flag",
	"                 G              - Send Gratuitous ARP flag",
	"                  C             - TX Cleanup flag",
	"                   p            - PCAP enabled flag",
	"                    S           - Send Sequence packets enabled",
	"                     R          - Send Range packets enabled",
	"                      D         - DPI Scanning enabled (If Enabled)",
	"                       I        - Process packets on input enabled",
	"                        *       - Using TAP interface for this port can be [-rt*]",
	"                         L      - Send Latency packets"
	"                          V     - Send VLAN ID tag",
	"                          M     - Send MPLS header",
	"                          Q     - Send Q-in-Q tags",
	"                           g    - Process GARP packets",
	"                            g   - Perform GRE with IPv4 payload",
	"                            G   - Perform GRE with Ethernet payload",
	"                             C  - Capture received packets",
	"                              R - Random bitfield(s) are applied",
	"Notes: <state>       - Use enable|disable or on|off to set the state.",
	"       <portlist>    - a list of ports (no spaces) as 2,4,6-9,12 or 3-5,8 or 5 or the word 'all'",
	"       Color best seen on a black background for now",
	"       To see a set of example Lua commands see the files in wr-examples/pktgen/test",
	"",
	NULL
};
