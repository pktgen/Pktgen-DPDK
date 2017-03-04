/*-
 * Copyright (c) <2017>, Intel Corporation
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

/* Created 2017 by Keith Wiles @ intel.com */

#include "cli-functions.h"

#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>

#include <rte_atomic.h>
#include <rte_devargs.h>

#include "pktgen.h"

#include <cli.h>
#include <cli_map.h>
#include <cli_string_fns.h>

#include "copyright_info.h"
#include "pktgen-cmds.h"
#include "pktgen-main.h"
#include "lpktgenlib.h"
#include "pktgen-display.h"
#include "pktgen-random.h"
#include "pktgen-log.h"

/**********************************************************/
static const char *title_help[] = {
	"   *** Help Information for Pktgen ***",
	"",
	NULL
};

CLI_INFO(Title, NULL, title_help);

static const char *status_help[] = {
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

CLI_INFO(Status, NULL, status_help);

static struct cli_map range_map[] = {
	{ 20, "range %P mac dst %|start|min|max|inc %m" },
	{ 21, "range %P mac src %|start|min|max|inc %m" },
	{ 30, "range %P ip dst %|start|min|max|inc %4" },
	{ 31, "range %P ip src %|start|min|max|inc %4" },
	{ 40, "range %P proto %|start|min|max|inc %d" },
	{ 50, "range %P port dst %|start|min|max|inc %d" },
	{ 51, "range %P port src %|start|min|max|inc %d" },
	{ 60, "range %P vlan %|start|min|max|inc %d" },
	{ 70, "range %P size %|start|min|max|inc %d" },
	{ 80, "range %P mpls entry %x" },
	{ 85, "range %P qinq index %d %d" },
	{ 90, "range %P gre key %d" },
    { -1, NULL }
};

static const char *range_help[] = {
	"  -- Setup the packet range values --",
	"                 - SMMI = Start|Min|Max|Inc (Start, Minimum, Maximum, Increment)",
	"range <portlist> mac [dst|src] <etheraddr>    - Set destination/source MAC address",
	"range <portlist> ip [src|dst] <SMMI> <ipaddr> - Set source IP start address",
	"range <portlist> proto [tcp|udp]              - Set the IP protocol type (alias range.proto)",
	"range <portlist> [sport|dport] <SMMI> <value> - Set source port start address",
	"range <portlist> vlan <SMMI> <value>          - Set vlan id start address",
	"range <portlist> size <SMMI> <value>          - Set pkt size start address",
	"range <portlist> teid <SMMI> <value>          - Set TEID value",
	"range <portlist> mpls entry <hex-value>       - Set MPLS entry value",
	"range <portlist> qinq index <val1> <val2>     - Set QinQ index values",
	"range <portlist> gre key <value>              - Set GRE key value",
	CLI_PAUSE,
	NULL
};

CLI_INFO(Range, range_map, range_help);

static int
range_cmd(int argc, char **argv)
{
	struct cli_map *m;
	uint32_t portlist;
	struct rte_ipaddr ip;

	m = cli_mapping(Range_info.map, argc, argv);
	if (!m)
		return -1;

	rte_parse_portlist(argv[1], &portlist);

	switch(m->index) {
		case 20:
			foreach_port(portlist,
			     range_set_dest_mac(info, argv[4],
								rte_ether_aton((const char *)argv[5], NULL)));
			break;
		case 21:
			foreach_port(portlist,
			     range_set_src_mac(info, argv[4],
								rte_ether_aton((const char *)argv[5], NULL)));
			break;
		case 30:
			rte_atoip(argv[5], RTE_IPADDR_V4, &ip, sizeof(ip));
			foreach_port(portlist,
			     range_set_dst_ip(info, argv[4], &ip));
			break;
		case 31:
			rte_atoip(argv[5], RTE_IPADDR_V4, &ip, sizeof(ip));
			foreach_port(portlist,
			     range_set_src_ip(info, argv[4], &ip));
			break;
		case 40:
			foreach_port(portlist,
				range_set_proto(info, argv[4]) );
			break;
		case 50:
			foreach_port(portlist,
				range_set_dst_port(info, argv[4], atoi(argv[5])) );
			break;
		case 51:
			foreach_port(portlist,
				range_set_src_port(info, argv[4], atoi(argv[5])) );
			break;
		case 60:
			foreach_port(portlist,
				range_set_vlan_id(info, argv[3], atoi(argv[4])) );
			break;
		case 70:
			foreach_port(portlist,
				range_set_pkt_size(info, argv[3], atoi(argv[4])) );
			break;
		case 80:
			foreach_port(portlist,
				range_set_mpls_entry(info, strtoul(argv[4], NULL, 16)) );
			break;
		case 85:
			foreach_port(portlist,
				range_set_qinqids(info, atoi(argv[4]), atoi(argv[5])) );
			break;
		case 90:
			foreach_port(portlist,
				range_set_gre_key(info, strtoul(argv[4], NULL, 10)) );
			break;
		default:
			return -1;
	}
	pktgen_update_display();
	return 0;
}

#define set_types	"count|"		/*  0 */ \
					"size|"			/*  1 */ \
					"rate|"			/*  2 */ \
					"burst|"		/*  3 */ \
					"tx_cycles|"	/*  4 */ \
					"sport|"		/*  5 */ \
					"dport|"		/*  6 */ \
					"seqCnt|"		/*  7 */ \
					"prime|"		/*  8 */ \
					"dump|"			/*  9 */ \
					"vlan_id"		/* 10 */

static struct cli_map set_map[] = {
	{ 10, "set %P %|" set_types " %d" },
	{ 11, "set %P jitter %D" },
	{ 20, "set %P type %|arp|ip4|ip6" },
	{ 21, "set %P proto %|udp|tcp|icmp" },
	{ 22, "set %P mac %m" },
	{ 23, "set %P pattern %|abc|none|user|zero" },
	{ 24, "set %P user pattern %s" },
	{ 30, "set %P ip %|src|dst %4" },
	{ 40, "set ports_per_page %d" },
    { -1, NULL }
};

static const char *set_help[] = {
	"set <portlist> <type> value        - Set a few port values",
	"  <portlist>                       - a list of ports as 2,4,6-9,12 or the word 'all'",
	"  <type>         count             - number of packets to transmit",
	"                 size              - size of the packet to transmit",
	"                 rate              - Packet rate in percentage",
	"                 burst             - number of packets in a burst",
	"                 sport             - Source port number for TCP",
	"                 dport             - Destination port number for TCP",
	"                 prime             - Set the number of packets to send on prime command",
	"                 seqCnt            - Set the number of packet in the sequence to send",
	"                 dump              - Dump the next <value> received packets to the screen",
	"                 vlanid            - Set the VLAN ID value for the portlist",
	"                 jitter            - Set the jitter threshold in micro-seconds",
	"                 mpls entry        - Set the MPLS entry for the portlist (must be specified in hex)",
	"                 gre_key           - Set the GRE key",
	"                 mac <etheraddr>   - Set MAC addresses 00:11:22:33:44:55",
	"                                     You can use 0011:2233:4455 format as well",
	"set <portlist> ip src|dst ipaddr   - Set IP addresses",
	"set <portlist> vlanid <vlanid>     - Set the VLAN ID for the portlist, same as 'set 0 vlanid 5'",
	"set <portlist> qinqids <id1> <id2> - Set the Q-in-Q ID's for the portlist",
	"set <portlist> proto udp|tcp|icmp  - Set the packet protocol to UDP or TCP or ICMP per port",
	"set <portlist> type ipv4|ipv6|vlan - Set the packet type to IPv4 or IPv6 or VLAN",
	"set <portlist> pattern <type>      - Set the fill pattern type",
	"     type - abc                    - Default pattern of abc string",
	"            none                   - No fill pattern, maybe random data",
	"            zero                   - Fill of zero bytes",
	"            user                   - User supplied string of max 16 bytes",
	"set <portlist> user pattern <string> - A 16 byte string, must set 'pattern user' command",
	"set <portlist> rnd <idx> <off> <mask> - Set random mask for all transmitted packets from portlist",
	"                                     idx: random mask slot",
	"                                     off: offset in packets, where to apply mask",
	"                                     mask: up to 32 bit long mask specification (empty to disable):",
	"                                       0: bit will be 0",
	"                                       1: bit will be 1",
	"                                       .: bit will be ignored (original value is retained)",
	"                                       X: bit will get random value",
	CLI_PAUSE,
	NULL
};

CLI_INFO(Set, set_map, set_help);

static int
set_cmd(int argc, char **argv)
{
	uint32_t portlist;
	char *what;
	int value, n;
	struct cli_map *m;
	struct rte_ipaddr ip;

	m = cli_mapping(Set_info.map, argc, argv);
	if (!m)
		return -1;

	rte_parse_portlist(argv[1], &portlist);

	what = argv[2];
	value = atoi(argv[3]);

	switch(m->index) {
		case 10:
			n = cli_list_search(m->fmt, argv[2], 2);
			foreach_port(portlist, _do(
				switch(n) {
					case 0: single_set_tx_count(info, value); break;
					case 1: single_set_pkt_size(info, value); break;
					case 2: single_set_tx_rate(info, value); break;
					case 3: single_set_tx_burst(info, value); break;
					case 4: debug_set_tx_cycles(info, value); break;
					case 5: single_set_port_value(info, what[0], value); break;
					case 6: single_set_port_value(info, what[0], value); break;
					case 7: pktgen_set_port_seqCnt(info, value); break;
					case 8: pktgen_set_port_prime(info, value); break;
					case 9: debug_set_port_dump(info, value); break;
					case 10: single_set_vlan_id(info, value); break;
					default:
						return -1;
				}) );
			break;
		case 11:
			foreach_port(portlist, single_set_jitter(info,
										strtoull(argv[3], NULL, 0)));
			break;
		case 20:
			foreach_port(portlist, single_set_pkt_type(info, argv[3]));
			break;
		case 21:
			foreach_port(portlist, single_set_proto(info, argv[3]));
			break;
		case 22:
			foreach_port(portlist, single_set_dst_mac(info,
										rte_ether_aton(argv[3], NULL)));
			break;
		case 23:
			foreach_port(portlist, pattern_set_type(info, argv[3]));
			break;
		case 24:
			foreach_port(portlist,
				     pattern_set_user_pattern(info, argv[3]));
			break;
		case 30:
			rte_atoip(argv[4], RTE_IPADDR_V4, &ip, sizeof(ip));
			foreach_port(portlist, single_set_ipaddr(info, argv[3][0], &ip));
			break;
		case 40:
			pktgen_set_page_size(atoi(argv[2]));
			break;
		default:
			return -1;
	}

	pktgen_update_display();
	return 0;
}

static struct cli_map pcap_map[] = {
	{ 10, "pcap index" },
	{ 20, "pcap show" },
	{ 30, "pcap filter %P %s" },
    { -1, NULL }
};

static const char *pcap_help[] = {
	"pcap show                          - Show PCAP information",
	"pcap index                         - Move the PCAP file index to the given packet number,  0 - rewind, -1 - end of file",
	"pcap filter <portlist> <string>    - PCAP filter string to filter packets on receive",
	"",
	NULL
};

CLI_INFO(PCAP, pcap_map, pcap_help);

static int
pcap_cmd(int argc, char **argv)
{
	struct cli_map *m;
	pcap_info_t   *pcap;
	uint32_t max_cnt;
	uint32_t value;
	uint32_t portlist;

	m = cli_mapping(PCAP_info.map, argc, argv);
	if (!m)
		return -1;

	switch(m->index) {
		case 10:
			pcap = pktgen.info[pktgen.portNum].pcap;
			max_cnt = pcap->pkt_count;
			value = strtoul(argv[1], NULL, 10);

			if (pcap) {
				if (value >= max_cnt)
					pcap->pkt_idx = max_cnt - RTE_MIN(PCAP_PAGE_SIZE, (int)max_cnt);
				else
					pcap->pkt_idx = value;
				pktgen.flags |= PRINT_LABELS_FLAG;
			} else
				pktgen_log_error(" ** PCAP file is not loaded on port %d",
						 pktgen.portNum);
			break;
		case 20:
			if (pktgen.info[pktgen.portNum].pcap)
				_pcap_info(pktgen.info[pktgen.portNum].pcap, pktgen.portNum, 1);
			else
				pktgen_log_error(" ** PCAP file is not loaded on port %d",
						 pktgen.portNum);
			break;
		case 30:
			rte_parse_portlist(argv[2], &portlist);
			foreach_port(portlist,
				pcap_filter(info, argv[3]) );
			break;
		default:
			return -1;
	}
	pktgen_update_display();
	return 0;
}

static struct cli_map start_map[] = {
	{  10, "start %P" },
	{  20, "stop %P" },
	{  40, "start %P prime" },
	{  50, "start %P arp %|request|gratuitous|req|grat" },
    { -1, NULL }
};

static const char *start_help[] = {
	"",
	"start <portlist>                   - Start transmitting packets",
	"stop <portlist>                    - Stop transmitting packets",
	"stp                                - Stop all ports from transmitting",
	"str                                - Start all ports transmitting",
	"start <portlist> prime             - Transmit packets on each port listed. See set prime command above",
	"start <portlist> arp <type>        - Send a ARP type packet",
	"    type - request | gratuitous | req | grat",
	CLI_PAUSE,
	NULL
};

CLI_INFO(Start, start_map, start_help);

static int
start_stop_cmd(int argc, char **argv)
{
	struct cli_map *m;
	uint32_t portlist;

	m = cli_mapping(Start_info.map, argc, argv);
	if (!m)
		return -1;

	rte_parse_portlist(argv[1], &portlist);

	switch (m->index) {
		case 10:
			foreach_port(portlist, pktgen_start_transmitting(info));
			break;
		case 20:
			foreach_port(portlist, pktgen_stop_transmitting(info));
			break;
		case 40:
			foreach_port(portlist, pktgen_prime_ports(info));
			break;
		case 50:
			if (argv[3][0] == 'g')
				foreach_port(portlist,
				     pktgen_send_arp_requests(info, GRATUITOUS_ARP) );
			else
				foreach_port(portlist,
				     pktgen_send_arp_requests(info, 0) );
			break;
		default:
			return -1;
	}
	pktgen_update_display();
	return 0;
}

static struct cli_map theme_map[] = {
	{  0, "theme" },
	{ 10, "theme %|on|off" },
	{ 20, "theme item %s %s %s %s" },
	{ 30, "theme save" },
	{ -1, NULL }
};

static const char *theme_help[] = {
	"theme <item> <fg> <bg> <attr>      - Set color for item with fg/bg color and attribute value",
	"theme show                         - List the item strings, colors and attributes to the items",
	"theme save <filename>              - Save the current color theme to a file",
	CLI_PAUSE,
	NULL
};

CLI_INFO(Theme, theme_map, theme_help);

static int
theme_cmd(int argc, char **argv)
{
	struct cli_map *m;

	m = cli_mapping(Theme_info.map, argc, argv);
	if (!m)
		return -1;

	switch(m->index) {
		case 0:
			pktgen_theme_show();
			break;
		case 10:
			pktgen_theme_state(argv[1]);
			pktgen_cls();
			break;
		case 20:
			pktgen_set_theme_item(argv[2], argv[3], argv[4], argv[5]);
			break;
		case 30:
			pktgen_theme_save(argv[2]);
			break;
		default:
			return -1;
	}
	return 0;
}

#define ed_type	"process|"		/*  0 */	\
				"mpls|" 		/*  1 */	\
				"qinq|" 		/*  2 */	\
				"gre|" 			/*  3 */	\
				"vlan|" 		/*  4 */	\
				"garp|" 		/*  5 */	\
				"random|" 		/*  6 */	\
				"latency|" 		/*  7 */	\
				"pcap|" 		/*  8 */	\
				"screen|" 		/*  9 */	\
				"mac_from_arp|" /* 10 */	\
				"blink|" 		/* 11 */	\
				"rx_tap|" 		/* 12 */	\
				"tx_tap|"		/* 13 */	\
				"icmp|"			/* 14 */	\
				"range|"		/* 15 */	\
				"capture|"		/* 16 */	\
				"gre_eth"		/* 17 */

static struct cli_map enable_map[] = {
	{ 10, "enable %P %|" ed_type },
	{ 20, "disable %P %|" ed_type },
    { -1, NULL }
};

static const char *enable_help[] = {
	"enable|disable <portlist> <features>",
	"    Feature - process              - Enable or Disable processing of ARP/ICMP/IPv4/IPv6 packets",
	"              mpls                 - Enable/disable sending MPLS entry in packets",
    "              qinq                 - Enable/disable sending Q-in-Q header in packets",
	"              gre                  - Enable/disable GRE support",
	"              gre_eth              - Enable/disable GRE with Ethernet frame payload",
	"              vlan                 - Enable/disable VLAN tagging",
	"              garp                 - Enable or Disable GARP packet processing and update MAC address",
	"              random               - Enable/disable Random packet support",
	"              latency              - Enable/disable latency testing",
    "              pcap                 - Enable or Disable sending pcap packets on a portlist",
	"              screen               - Enable/disable updating the screen and unlock/lock window",
	"              mac_from_arp         - Enable/disable MAC address from ARP packet",
	"              blink                - Blink LED on port(s)",
	"              rx_tap               - Enable/Disable RX Tap support",
	"              tx_tap               - Enable/Disable TX Tap support",
	"              icmp                 - Enable/Disable sending ICMP packets",
	"              range                - Enable or Disable the given portlist for sending a range of packets",
	"              capture              - Enable/disable packet capturing on a portlist",
	"              screen               - Enable/disable updating the screen and unlock/lock window",
	"              theme                - Enable or Disable the theme",
	"off                                - screen off shortcut",
	"on                                 - screen on shortcut",
	CLI_PAUSE,
	NULL
};

CLI_INFO(Enable, enable_map, enable_help);

static int
enable_disable_cmd(int argc, char **argv)
{
	struct cli_map *m;
	uint32_t portlist;
	int n, state;

	m = cli_mapping(Enable_info.map, argc, argv);
	if (!m)
		return -1;

	rte_parse_portlist(argv[1], &portlist);

	switch (m->index) {
		case 10:
		case 20:
			n = cli_list_search(m->fmt, argv[1], 1);

			state = estate(argv[0]);

			switch(n) {
				case 0:
					foreach_port(portlist, enable_process(info, state));
					break;
				case 1:
					foreach_port(portlist, enable_mpls(info, state) );
					break;
				case 2:
					foreach_port(portlist, enable_qinq(info, state) );
					break;
				case 3:
					foreach_port(portlist, enable_gre(info, state) );
					break;
				case 4:
					foreach_port(portlist, enable_vlan(info, state) );
					break;
				case 5:
					foreach_port(portlist, enable_garp(info, state) );
					break;
				case 6:
					foreach_port(portlist, enable_random(info, state) );
					break;
				case 7:
					foreach_port(portlist, enable_latency(info, state) );
					break;
				case 8:
					foreach_port(portlist, enable_pcap(info, state) );
					break;
				case 9:
					pktgen_screen(state);
					break;
				case 10:
					enable_mac_from_arp(state);
					break;
				case 11:
					foreach_port(portlist, debug_blink(info, state));

					if (pktgen.blinklist)
						pktgen.flags |= BLINK_PORTS_FLAG;
					else
						pktgen.flags &= ~BLINK_PORTS_FLAG;
					break;
				case 12:
					foreach_port(portlist, enable_rx_tap(info, state));
					break;
				case 13:
					foreach_port(portlist, enable_tx_tap(info, state));
					break;
				case 14:
					foreach_port(portlist, enable_icmp_echo(info, state));
					break;
				case 15:
					foreach_port(portlist, enable_range(info, state));
					break;
				case 16:
					foreach_port(portlist, pktgen_set_capture(info, state));
					break;

				default:
					return -1;
			}
			break;
		default:
			return -1;
	}
	pktgen_update_display();
	return 0;
}

static struct cli_map debug_map[] = {
	{ 10, "debug l2p" },
	{ 20, "debug tx_debug" },
	{ 30, "debug mempool %P %s" },
	{ 40, "debug pdump %P" },
    { -1, NULL }
};

static const char *debug_help[] = {
	"debug l2p                          - Dump out internal lcore to port mapping",
	"debug tx_debug                     - Enable tx debug output",
	"debug mempool <portlist> <type>    - Dump out the mempool info for a given type",
	"debug pdump <portlist>             - Hex dump the first packet to be sent, single packet mode only",
	"",
	NULL
};

CLI_INFO(Debug, debug_map, debug_help);

static int
debug_cmd(int argc, char **argv)
{
	struct cli_map *m;
	uint32_t portlist;

	m = cli_mapping(Debug_info.map, argc, argv);
	if (!m)
		return -1;

	switch(m->index) {
		case 10:
			pktgen_l2p_dump();
			break;
		case 20:
			if ( (pktgen.flags & TX_DEBUG_FLAG) == 0)
				pktgen.flags |= TX_DEBUG_FLAG;
			else
				pktgen.flags &= ~TX_DEBUG_FLAG;
			pktgen_cls();
			break;
		case 30:
			rte_parse_portlist(argv[2], &portlist);
			if (!strcmp(argv[1], "dump") )
				foreach_port(portlist,
					     debug_mempool_dump(info, argv[3]) );
			break;
		case 40:
			rte_parse_portlist(argv[2], &portlist);
			foreach_port(portlist, debug_pdump(info));
			pktgen_update_display();
			break;
		default:
			return -1;
	}
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_seq_parsed - Set a sequence config for given port and slot.
 *
 * DESCRIPTION
 * Set up the sequence packets for a given port and slot.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 * 	"%|seq|sequence %d %P %m %m %4 %4 %d %d %|ipv4|ipv6 %|udp|tcp|icmp %d %d %d"

 */

static int
seq_1_set_cmd(int argc __rte_unused, char **argv)
{
	char *proto = argv[10];
	char *eth = argv[9];
	int seqnum = atoi(argv[1]);
	uint32_t portlist;
	struct rte_ipaddr dst, src;
	uint32_t teid;

	if ( (proto[0] == 'i') && (eth[3] == '6') ) {
		cli_printf("Must use IPv4 with ICMP type packets\n");
		return -1;
	}

	if (seqnum >= NUM_SEQ_PKTS)
		return -1;

	teid = (argc == 11)? strtoul(argv[13], NULL, 10) : 0;
	rte_atoip(argv[5], RTE_IPADDR_V4, &dst, sizeof(dst));
	rte_atoip(argv[6], RTE_IPADDR_V4, &src, sizeof(src));
	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     pktgen_set_seq(info, seqnum,
				    rte_ether_aton(argv[3], NULL), rte_ether_aton(argv[4], NULL),
				    &dst, &src,
				    atoi(argv[7]), atoi(argv[8]), eth[3],
				    proto[0],
				    atoi(argv[11]), atoi(argv[12]),
				    teid) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_seq_parsed - Set a sequence config for given port and slot.
 *
 * DESCRIPTION
 * Set up the sequence packets for a given port and slot.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 * 	"%|seq|sequence %d %P dst %m src %m dst %4 src %4 sport %d dport %d %|ipv4|ipv6 %|udp|tcp|icmp vlan %d size %d teid %d"
 */

static int
seq_2_set_cmd(int argc __rte_unused, char **argv)
{
	char *proto = argv[16];
	char *eth = argv[15];
	int seqnum = atoi(argv[1]);
	uint32_t portlist;
	struct rte_ipaddr dst, src;
	uint32_t teid;

	if ( (proto[0] == 'i') && (eth[3] == '6') ) {
		cli_printf("Must use IPv4 with ICMP type packets\n");
		return -1;
	}

	if (seqnum >= NUM_SEQ_PKTS)
		return -1;

	teid = (argc == 23)? strtoul(argv[22], NULL, 10) : 0;
	rte_atoip(argv[8], RTE_IPADDR_V4, &dst, sizeof(dst));
	rte_atoip(argv[10], RTE_IPADDR_V4, &src, sizeof(src));
	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     pktgen_set_seq(info, seqnum,
				    rte_ether_aton(argv[4], NULL), rte_ether_aton(argv[6], NULL),
				    &dst, &src,
				    atoi(argv[12]), atoi(argv[14]), eth[3],
				    proto[0],
				    atoi(argv[18]), atoi(argv[20]),
				    teid) );

	pktgen_update_display();
	return 0;
}

static struct cli_map seq_map[] = {
	{ 10, "%|seq|sequence %d %P %m %m %4 %4 %d %d %|ipv4|ipv6 %|udp|tcp|icmp %d %d %d" },
	{ 11, "%|seq|sequence %d %P dst %m src %m dst %4 src %4 sport %d dport %d %|ipv4|ipv6 %|udp|tcp|icmp vlan %d size %d" },
	{ 12, "%|seq|sequence %d %P dst %m src %m dst %4 src %4 sport %d dport %d %|ipv4|ipv6 %|udp|tcp|icmp vlan %d size %d teid %d" },
	{ -1, NULL }
};

static const char *seq_help[] = {
	"seq <seq#> <portlist> dst <Mac> src <Mac> dst <IP> src <IP> sport <val> dport <val> ipv4|ipv6 udp|tcp|icmp vlan <val> pktsize <val> teid <val>",
	"seq <seq#> <portlist> <dst-Mac> <src-Mac> <dst-IP> <src-IP> <sport> <dport> ipv4|ipv6 udp|tcp|icmp <vlanid> <pktsize> <teid>",
	"                                   - Set the sequence packet information, make sure the src-IP",
	"                                     has the netmask value eg 1.2.3.4/24",
	"",
	NULL
};

CLI_INFO(Seq, seq_map, seq_help);

static int
seq_cmd(int argc, char **argv)
{
	struct cli_map *m;

	m = cli_mapping(Seq_info.map, argc, argv);
	if (!m)
		return -1;

	switch(m->index) {
		case 10: seq_1_set_cmd(argc, argv); break;
		case 11:
		case 12: seq_2_set_cmd(argc, argv); break;
		default:
			return -1;
	}
	return 0;
}

/**************************************************************************//**
 *
 * cmd_script_parsed - Command to execute a script.
 *
 * DESCRIPTION
 * Load the script file and execute the commands.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
script_cmd(int argc __rte_unused, char **argv)
{
	lua_State *L = pktgen.L;

	if (L == NULL) {
		pktgen_log_error("Lua is not initialized!");
		return -1;
	}

	if (is_cli_help(argc, argv)) {
		cli_printf("\nUsage: %s <script-string>\n", argv[0]);
		return 0;
	}

	if (luaL_dofile(L, argv[1]) != 0)
		pktgen_log_error("%s", lua_tostring(L, -1));
	return 0;
}

/**************************************************************************//**
 *
 * cmd_exec_lua_parsed - Command to execute lua code on command line.
 *
 * DESCRIPTION
 * Execute a string of lua code
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
exec_lua_cmd(int argc __rte_unused, char **argv __rte_unused)
{
	lua_State *L = pktgen.L;

	if (L == NULL) {
		pktgen_log_error("Lua is not initialized!");
		return -1;
	}

	if (is_cli_help(argc, argv)) {
		cli_printf("\nUsage: %s <script-string>\n", argv[0]);
		return 0;
	}

	if (luaL_dostring(L, this_cli->gb->buf) != 0)
		pktgen_log_error("%s", lua_tostring(L, -1));
	return 0;
}

static struct cli_map misc_map[] = {
	{ 10, "clear %P" },
	{ 20, "geom %s" },
	{ 21, "geom" },
	{ 30, "load %s" },
	{ 40, "script %l" },
	{ 50, "lua %l" },
	{ 60, "save %s" },
	{ 70, "redisplay" },
	{ 100, "reset" },
	{ 110, "restart" },
	{ 130, "port %d" },
	{ 135, "ports per page %d" },
	{ 140, "ping4 %P %4" },
#ifdef INCLUDE_PING6
	{ 141, "ping6 %P %6" },
#endif
	{ -1, NULL }
};

static const char *misc_help[] = {
	"save <path-to-file>                - Save a configuration file using the filename",
	"load <path-to-file>                - Load a command/script file from the given path",
	"script <filename>                  - Execute the Lua script code in file (www.lua.org).",
	"lua 'lua string'                   - Execute the Lua code in the string needs quotes",
	"geometry <geom>                    - Set the display geometry Columns by Rows (ColxRow)",
	"clear <portlist>                   - Clear the statistics",
	"clr                                - Clear all Statistices",
	"cls                                - Clear the screen",
	"reset <portlist>                   - Reset the configuration the ports to the default",
	"rst                                - Reset the configuration for all ports",
	"ports per page [1-6]               - Set the number of ports displayed per page",
	"port <number>                      - Sets the sequence packets to display for a given port",
	"restart <portlist>                 - Restart or stop a ethernet port and restart",
	"ping4 <portlist>                   - Send a IPv4 ICMP echo request on the given portlist",
#ifdef INCLUDE_PING6
	"ping6 <portlist>                   - Send a IPv6 ICMP echo request on the given portlist",
#endif
	CLI_PAUSE,
	NULL
};

CLI_INFO(Misc, misc_map, misc_help);

static int
misc_cmd(int argc, char **argv)
{
	struct cli_map *m;
	uint32_t portlist;
	uint16_t rows, cols;
	char *p;

	m = cli_mapping(Misc_info.map, argc, argv);
	if (!m)
		return -1;

	switch(m->index) {
		case 10:
			rte_parse_portlist(argv[1], &portlist);
			foreach_port(portlist,
				     pktgen_clear_stats(info) );
			break;
		case 20:
			p = strchr(argv[1], 'x');
			if (p) {
				rows = strtol(++p, NULL, 10);
				cols = strtol(argv[1], NULL, 10);

				pktgen_display_set_geometry(rows, cols);
				pktgen_cls();
			} else
				return -1;
			/* FALLTHRU */
		case 21:
			pktgen_display_get_geometry(&rows, &cols);
			break;
		case 30:
			if (cli_load_cmds(argv[1]) )
				cli_printf("load command failed for %s\n", argv[1]);
			if (!scrn_is_paused() )
				pktgen_redisplay(0);
			break;
		case 40: script_cmd(argc, argv); break;
		case 50: exec_lua_cmd(argc, argv); break;
		case 60: pktgen_save(argv[1]); break;
		case 70: pktgen_redisplay(1); break;
		case 100:
			rte_parse_portlist(argv[1], &portlist);
			foreach_port(portlist,
				     pktgen_reset(info) );
			break;
		case 110:
			rte_parse_portlist(argv[1], &portlist);
			foreach_port(portlist,
				     pktgen_port_restart(info) );
			break;
		case 120:
		case 130: pktgen_set_port_number(atoi(argv[1])); break;
		case 140:
			rte_parse_portlist(argv[1], &portlist);
			foreach_port(portlist, pktgen_ping4(info));
			pktgen_update_display();
			break;
#ifdef INCLUDE_PING6
		case 141:
			rte_parse_portlist(argv[1], &portlist);
			foreach_port(portlist, pktgen_ping6(info));
			pktgen_update_display();
			break;
#endif
		default:
			return -1;
	}
	return 0;
}

static struct cli_map page_map[] = {
	{ 10, "page %d" },
	{ 11, "page %|main|range|config|cfg|pcap|cpu|next|sequence|seq|rnd|log|latency|stats" },
	{ -1, NULL }
};

static const char *page_help[] = {
	"page <pages>                       - Show the port pages or configuration or sequence page",
	"     [0-7]                         - Page of different ports",
	"     main                          - Display page zero",
	"     range                         - Display the range packet page",
	"     config | cfg                  - Display the configuration page",
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
	CLI_PAUSE,
	NULL
};

CLI_INFO(Page, page_map, page_help);

static int
page_cmd(int argc, char **argv)
{
	struct cli_map *m;

	m = cli_mapping(Page_info.map, argc, argv);
	if (!m)
		return -1;

	switch(m->index) {
		case 10:
		case 11: pktgen_set_page(argv[1]); break;
		default:
			return -1;
	}
	return 0;
}

/**********************************************************/
/**********************************************************/
/****** CONTEXT (list of instruction) */

static int help_cmd(int argc, char **argv);

static struct cli_tree default_tree[] = {
	c_dir("/pktgen/bin"),
	c_cmd("help",		help_cmd, 		"help command"),

	c_cmd("clear",		misc_cmd,		"clear stats"),
	c_cmd("geom",		misc_cmd, 		"set the screen geometry"),
	c_cmd("load",		misc_cmd, 		"load command file"),
	c_cmd("script", 	misc_cmd,		"run a Lua script"),
	c_cmd("lua", 		misc_cmd,		"execute a Lua string"),
	c_cmd("save", 		misc_cmd,		"save the current state"),
	c_cmd("redisplay",	misc_cmd,		"redisplay the screen"),
	c_cmd("reset",		misc_cmd,		"reset pktgen configuration"),
	c_cmd("restart", 	misc_cmd,		"restart port"),
	c_cmd("port", 		misc_cmd, 		"Switch between ports"),
	c_cmd("ping4", 		misc_cmd, 		"Send a ping packet for IPv4"),
#ifdef INCLUDE_PING6
	c_cmd("ping6", 		misc_cmd,		"Send a ping packet for IPv6"),
#endif

	c_cmd("sequeuce",	seq_cmd,		"sequence command"),
	c_alias("seq",		"sequence",		"sequence command"),

	c_cmd("page",		page_cmd,		"change page displays"),
	c_cmd("theme", 		theme_cmd,		"Set, save, show the theme"),
	c_cmd("range",		range_cmd,		"Range commands"),
	c_cmd("enable",		enable_disable_cmd,	"enable features"),
	c_cmd("disable",	enable_disable_cmd,	"disable features"),
	c_cmd("start",		start_stop_cmd,	"start features"),
	c_cmd("stop",		start_stop_cmd,	"stop features"),
	c_cmd("pcap",		pcap_cmd, 		"pcap commands"),
	c_cmd("set", 		set_cmd, 		"set a number of options"),
	c_cmd("debug",      debug_cmd,		"debug commands"),

	c_alias("str",		"start all",	"start all ports sending packets"),
	c_alias("stp",		"stop all",		"stop all ports sending packets"),
	c_alias("clr",		"clear all",	"clear all port stats"),
	c_alias("on",       "enable screen","Enable screen updates"),
	c_alias("off",      "disable screen", "Disable screen updates"),
	c_alias("rst",      "reset all",    "reset all ports"),

	c_end()
};

static int
init_tree(void)
{
	/* Add the system default commands in /sbin directory */
    if (cli_default_tree_init())
        return -1;

	/* Add the Pktgen directory tree */
    if (cli_add_tree(cli_root_node(), default_tree))
        return -1;

	/* Make sure the pktgen commands are executable an in search path */
    if (cli_add_bin_path("/pktgen/bin"))
        return -1;

	return 0;
}

static void
my_prompt(int cont __rte_unused)
{
    cli_printf("Pktgen:%s> ", cli_path_string(NULL, NULL));
}

int
pktgen_cli_create(void)
{
    return cli_create(my_prompt,      /* my local prompt routine */
                     init_tree,
                     CLI_DEFAULT_NODES,
                     CLI_DEFAULT_HISTORY);
}

void
pktgen_cli_start(void)
{
    cli_start(NULL, 1);

    cli_destroy();
}

static struct cli_info *help_data[] = {
	&Title_info,
	&Page_info,
	&Enable_info,
	&Set_info,
	&Range_info,
	&Seq_info,
	&PCAP_info,
	&Start_info,
	&Debug_info,
	&Misc_info,
	&Theme_info,
	&Status_info,
	NULL
};

/**************************************************************************//**
 *
 * Display the help screen and pause if needed.
 *
 * DESCRIPTION
 * Display the help and use pause to show screen full of messages.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
help_cmd(int argc __rte_unused, char **argv __rte_unused)
{
	int paused;

	paused = scrn_is_paused();

	if (!paused)
		scrn_pause();

	scrn_setw(1);
	scrn_cls();
	scrn_pos(0, 0);

	cli_show_help(help_data);

	if (!paused) {
		scrn_setw(pktgen.last_row + 1);
		scrn_resume();
		pktgen_redisplay(1);
	}
	return 0;
}
