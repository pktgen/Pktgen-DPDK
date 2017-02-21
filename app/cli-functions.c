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
#include <string_fns.h>

#include "copyright_info.h"
#include "pktgen-cmds.h"
#include "pktgen-main.h"
#include "lpktgenlib.h"
#include "pktgen-display.h"
#include "pktgen-random.h"
#include "pktgen-log.h"

/**********************************************************/
static const char *help_info[] = {
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
	"pattern user \"string\"            - A 16 byte string, must set 'pattern user' command",
	"enable|disable <feature>           - Enable or Disable a feature",
	"    Feature - process|mpls|qinq|gre|vlan|garp|random|latency|pcap",
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
	"port <number>                      - Sets the sequence of packets to display for a given port",
	"process <portlist> <state>         - Enable or Disable processing of ARP/ICMP/IPv4/IPv6 packets",
	"garp <portlist> <state>            - Enable or Disable GARP packet processing and update MAC address",
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

/**************************************************************************//**
 *
 * cmd_help_parsed - Display the help screen and pause if needed.
 *
 * DESCRIPTION
 * Display the help and use pause to show screen full of messages.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
help_cmd(struct cli *cli, int argc __rte_unused, char **argv __rte_unused)
{
	int i, paused;

	paused = scrn_is_paused();

	if (!paused)
		scrn_pause();
	scrn_setw(1);
	scrn_cls();

	scrn_pos(0, 0);
	cli_printf(cli, "%s%s\n", help_info[1], copyright_msg());
	scrn_pos(4, 0);
	for (i = 2; help_info[i] != NULL; i++) {
		if (strcmp(help_info[i], "<<PageBreak>>") == 0) {
			if (cli_pause(cli,
					"   <More Help: Press Return to Continue or ESC>",
				 	NULL, NULL) )
				goto leave;
			scrn_cls();
			scrn_pos(0, 0);
			cli_printf(cli,
				       "%s%s\n",
				       help_info[1],
				       copyright_msg());
			scrn_pos(4, 0);
			continue;
		}
		cli_printf(cli, "%s\n", help_info[i]);
	}

	cli_pause(cli, "   <Press Return to Continue or ESC>", NULL, NULL);
leave:
	if (!paused) {
		scrn_setw(pktgen.last_row + 1);
		scrn_resume();
		pktgen_redisplay(1);
	}
	return 0;
}

static struct cli_map theme_map[] = {
	{  0, "theme" },
	{ 10, "theme %|on|off" },
	{ 20, "theme item %s %s %s %s" },
	{ 30, "theme save" },
	{ -1, NULL }
};

static int
theme_cmd(struct cli *cli __rte_unused, int argc, char **argv)
{
	struct cli_map *m;

	m = cli_mapping(cli, theme_map, argc, argv);
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
script_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	lua_State *L = pktgen.L;

	if (L == NULL) {
		pktgen_log_error("Lua is not initialized!");
		return -1;
	}

	if (is_cli_help(argc, argv)) {
		cli_printf(cli, "\nUsage: %s <script-string>\n", argv[0]);
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
exec_lua_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv __rte_unused)
{
	lua_State *L = pktgen.L;

	if (L == NULL) {
		pktgen_log_error("Lua is not initialized!");
		return -1;
	}

	if (is_cli_help(argc, argv)) {
		cli_printf(cli, "\nUsage: %s <script-string>\n", argv[0]);
		return 0;
	}

	if (luaL_dostring(L, cli->gb->buf) != 0)
		pktgen_log_error("%s", lua_tostring(L, -1));
	return 0;
}

/**************************************************************************//**
 *
 * cmd_ping4_parsed - Ping command for IPv4
 *
 * DESCRIPTION
 * Ping command for IPv4 sending a ICMP echo request.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
ping4_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pktgen_ping4(info));
	pktgen_update_display();
	return 0;
}

#ifdef INCLUDE_PING6
/**************************************************************************//**
 *
 * cmd_ping6_parsed - Send a Ping IPv6 command
 *
 * DESCRIPTION
 * Send a ICMP Ping IPv6 request.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
ping6_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pktgen_ping6(info));
	pktgen_update_display();
	return 0;
}
#endif

static struct cli_map range_map[] = {
	{ 10, "range %P %|on|off" },
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

static int
range_cmd(struct cli *cli __rte_unused, int argc, char **argv)
{
	struct cli_map *m;
	uint32_t portlist;
	rte_ipaddr_t ip;

	m = cli_mapping(cli, range_map, argc, argv);
	if (!m)
		return -1;

	rte_parse_portlist(argv[1], &portlist);

	switch(m->index) {
		case 10:
			foreach_port(portlist, enable_range(info, estate(argv[2])));
			break;
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

	/* c_cmd("pattern", 	pattern_user_cmd, "set the user pattern"), */

static int
set_cmd(struct cli *cli __rte_unused, int argc, char **argv)
{
	uint32_t portlist;
	char *what;
	int value, n;
	struct cli_map *m;
	rte_ipaddr_t ip;

	m = cli_mapping(cli, set_map, argc, argv);
	if (!m)
		return -1;

	rte_parse_portlist(argv[1], &portlist);

	what = argv[2];
	value = atoi(argv[3]);

	switch(m->index) {
		case 10:
			n = cli_list_search(cli, m->fmt, argv[2], 2);
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

static int
pcap_cmd(struct cli *cli __rte_unused, int argc, char **argv)
{
	struct cli_map *m;
	pcap_info_t   *pcap;
	uint32_t max_cnt;
	uint32_t value;
	uint32_t portlist;

	m = cli_mapping(cli, pcap_map, argc, argv);
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

/**************************************************************************//**
 *
 * cmd_set_port_parsed - Set the current working port number
 *
 * DESCRIPTION
 * Set the current working port number for sequence configuration.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
port_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{

	pktgen_set_port_number(atoi(argv[1]));
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
 */

static int
seq_set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	char *proto = argv[10];
	char *eth = argv[9];
	int seqnum = atoi(argv[1]);
	uint32_t portlist;
	rte_ipaddr_t dst, src;
	uint32_t teid;

	if ( (proto[0] == 'i') && (eth[3] == '6') ) {
		cli_printf(cli, "Must use IPv4 with ICMP type packets\n");
		return -1;
	}

	if (seqnum >= NUM_SEQ_PKTS)
		return -1;

	teid = strtoul(argv[13], NULL, 10);
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
 * cmd_set_page_parsed - Set which page to display on the screen.
 *
 * DESCRIPTION
 * Set the page to display on the screen.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
page_set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	pktgen_set_page(argv[1]);
	return 0;
}

static struct cli_map start_stop_map[] = {
	{  10, "start %P" },
	{  20, "stop %P" },
	{  30, "%|start|stop %P capture" },
	{  40, "start %P prime" },
	{  50, "start %P arp %|request|gratuitous|req|grat" },
    { -1, NULL }
};

static int
start_stop_cmd(struct cli *cli __rte_unused, int argc, char **argv)
{
	struct cli_map *m;
	uint32_t portlist;

	m = cli_mapping(cli, start_stop_map, argc, argv);
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
		case 30:
			foreach_port(portlist, pktgen_set_capture(info, estate(argv[0])));
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
				"icmp"			/* 14 */

static struct cli_map enable_disable_map[] = {
	{ 10, "enable %P %|" ed_type },
	{ 20, "disable %P %|" ed_type },
    { -1, NULL }
};

static int
enable_disable_cmd(struct cli *cli __rte_unused, int argc, char **argv)
{
	struct cli_map *m;
	uint32_t portlist;
	int n, state;

	m = cli_mapping(cli, enable_disable_map, argc, argv);
	if (!m)
		return -1;

	rte_parse_portlist(argv[1], &portlist);

	switch (m->index) {
		case 10:
		case 20:
			n = cli_list_search(cli, m->fmt, argv[1], 1);

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
	{ 30, "debug mempool dump %P %s" },
    { -1, NULL }
};

static int
debug_cmd(struct cli *cli __rte_unused, int argc, char **argv)
{
	struct cli_map *m;
	uint32_t portlist;

	m = cli_mapping(cli, debug_map, argc, argv);
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
			break;
		case 50:
			break;
		default:
			return -1;
	}
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
	{ 80, "seq %d %P dst %m src %m dst %4 src %4 sport %d dport %d %|ipv4|ipv6 %|udp|tcp|icmp vlan %d size %d teid %d" },
	{ 90, "pdump %P" },
	{ 100, "reset" },
	{ 110, "restart " },
	{ -1, NULL }
};

static int
misc_cmd(struct cli *cli, int argc, char **argv)
{
	struct cli_map *m;
	uint32_t portlist;
	uint16_t rows, cols;
	char *p;

	m = cli_mapping(cli, misc_map, argc, argv);
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
			if (cli_load_cmds(cli, argv[1]) )
				cli_printf(cli, "load command failed for %s\n", argv[1]);
			if (!scrn_is_paused() )
				pktgen_redisplay(0);
			break;
		case 40: script_cmd(cli, argc, argv); break;
		case 50: exec_lua_cmd(cli, argc, argv); break;
		case 60: pktgen_save(argv[1]); break;
		case 70: pktgen_redisplay(1); break;
		case 80: seq_set_cmd(cli, argc, argv); break;
		case 90:
			rte_parse_portlist(argv[1], &portlist);
			foreach_port(portlist, debug_pdump(info));
			pktgen_update_display();
			break;
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
		default:
			return -1;
	}
	return 0;
}

/**********************************************************/
/**********************************************************/
/****** CONTEXT (list of instruction) */

static struct cli_tree default_tree[] = {
	c_dir("/pktgen/bin"),
	c_cmd("help",		help_cmd, 		"help command"),

	c_cmd("clear",		misc_cmd,		"clear stats"),
	c_cmd("geom",		misc_cmd, 		"set the screen geometry"),
	c_cmd("load",		misc_cmd, 		"load command file"),
	c_cmd("script", 	misc_cmd, 		"run a Lua script"),
	c_cmd("lua", 		misc_cmd, 		"execute a Lua string"),
	c_cmd("save", 		misc_cmd, 		"save the current state"),
	c_cmd("redisplay",	misc_cmd,		"redisplay the screen"),
	c_cmd("seq",		misc_cmd,		"sequence command"),
	c_cmd("pdump",		misc_cmd,		"Dump packet on port"),
	c_cmd("reset",		misc_cmd,		"reset pktgen configuration"),
	c_cmd("restart", 	misc_cmd, 		"reset port to default state"),

	c_cmd("theme", 		theme_cmd,		"Set, save, show the theme"),
	c_cmd("range",		range_cmd,		"Range commands"),
	c_cmd("enable",		enable_disable_cmd,	"enable features"),
	c_cmd("disable",	enable_disable_cmd,	"disable features"),
	c_cmd("start",		start_stop_cmd,	"start features"),
	c_cmd("stop",		start_stop_cmd,	"stop features"),
	c_cmd("pcap",		pcap_cmd, 		"pcap commands"),
	c_cmd("set", 		set_cmd, 		"set a number of options"),
	c_cmd("page",		page_set_cmd, 	"change page displays"),
	c_cmd("port", 		port_cmd, 		"Switch between ports"),

	c_cmd("ping4", 		ping4_cmd, 		"Send a ping packet for IPv4"),
#ifdef INCLUDE_PING6
	c_cmd("ping6", 		ping6_cmd,		"Send a ping packet for IPv6"),
#endif
	c_cmd("debug",      debug_cmd,		"debug commands"),

	c_alias("str",		"start all",	"start all ports sending packets"),
	c_alias("stp",		"stop all",		"stop all ports sending packets"),
	c_alias("clr",		"clear all",	"clear all port stats"),

	c_end()
};

static int
init_tree(struct cli *cli)
{
    if (cli_default_tree_init(cli))
        return -1;

    if (cli_add_tree(cli, cli_root_node(cli), default_tree))
        return -1;

    if (cli_add_bin_path(cli, "/pktgen/bin"))
        return -1;

	return 0;
}

static void
my_prompt(struct cli *cli, int cont __rte_unused)
{
    cli_printf(cli, "Pktgen:%s> ", cli_path_string(cli, NULL, NULL));
}

void
pktgen_cli_start(void)
{
    struct cli *cli;

    cli = cli_create(my_prompt,      /* my local prompt routine */
                     init_tree,
                     CLI_DEFAULT_NODES,
                     CLI_DEFAULT_HISTORY);
    if (cli) {
        cli_stdin_setup(cli);

        cli_start(cli, NULL, 1);

        cli_stdin_restore(cli);

        cli_destroy(cli);
    }
}
