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

/**************************************************************************//**
 *
 * cmd_theme_state - Enable or disable the theme
 *
 * DESCRIPTION
 * Enable or disable the theme.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
theme_state_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char ** argv)
{
	pktgen_theme_state(argv[1]);
	pktgen_cls();

	return 0;
}

/**************************************************************************//**
 *
 * cmd_theme - Set a color for a given item
 *
 * DESCRIPTION
 * Set the given item to the give color/attr
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
theme_set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	pktgen_set_theme_item(argv[1], argv[2], argv[3], argv[4]);
	return 0;
}

/**************************************************************************//**
 *
 * cmd_theme_show - show the item names and attributes
 *
 * DESCRIPTION
 * show the item names and attributes
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
theme_show_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv __rte_unused)
{
	pktgen_theme_show();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_theme_save - Save a theme to a file
 *
 * DESCRIPTION
 * Save the current theme to a file.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
theme_save_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	pktgen_theme_save(argv[1]);
	return 0;
}

/**************************************************************************//**
 *
 * cmd_save - Save a configuration
 *
 * DESCRIPTION
 * Save the configuration into a script file.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
save_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	pktgen_save(argv[1]);
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

	if (luaL_dostring(L, cli->gb->buf) != 0)
		pktgen_log_error("%s", lua_tostring(L, -1));
	return 0;
}
/**************************************************************************//**
 *
 * cmd_pdump_parsed - Hex dump of the first packet.
 *
 * DESCRIPTION
 * dump out the first packet in hex.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pdump_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);

	foreach_port(portlist, debug_pdump(info));
	pktgen_update_display();
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

/**************************************************************************//**
 *
 * range_set_cmd - Set the range command options.
 *
 * DESCRIPTION
 * Set the range port options.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_range(info, argv[2]) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_latency_parsed - Set the latency testing.
 *
 * DESCRIPTION
 * Set the latency testing.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
latency_set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_latency(info, argv[2]));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_jitter_parsed - Set the jitter threshold testing.
 *
 * DESCRIPTION
 * Set the jitter threshold testing.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
jitter_set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_jitter(info, strtoull(argv[2], NULL, 0)) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_pattern_parsed - Set the fill pattern per port
 *
 * DESCRIPTION
 * Set the fill pattern per port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pattern_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pattern_set_type(info, argv[2]) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_user_pattern_parsed - Set the user fill pattern per port
 *
 * DESCRIPTION
 * Set the user fill pattern per port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pattern_user_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pattern_set_user_pattern(info, argv[2]) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_rnd_parsed - Set random bitfields.
 *
 * DESCRIPTION
 * Set random bitfields.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
rnd_cmd(struct cli *cli __rte_unused, int argc, char **argv)
{
	uint32_t portlist;

	char mask[33] = { 0 };
	int i, mask_idx = 0;
	char curr_bit;

	if (argc >= 4)
		return -1;

	if (strcmp(argv[4], "off"))
		/* Filter invalid characters from provided mask. This way the user can
		 * more easily enter long bitmasks, using for example '_' as a separator
		 * every 8 bits. */
		for (i = 0;
		     (mask_idx < 32) && ((curr_bit = argv[4][i]) != '\0');
		     ++i)
			if ((curr_bit == '0') || (curr_bit == '1') ||
			    (curr_bit == '.') || (curr_bit == 'X') || (curr_bit == 'x'))
				mask[mask_idx++] = curr_bit;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_random(info, pktgen_set_random_bitfield(info->rnd_bitfields,
									atoi(argv[2]), atoi(argv[3]), mask) ? ENABLE_STATE : DISABLE_STATE));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_geometry_parsed - Set teh display geometry values.
 *
 * DESCRIPTION
 * Set the number of columns and rows for the display.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
geometry_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint16_t rows, cols;

	pktgen_display_get_geometry(&rows, &cols);
	pktgen_log_debug("Old geometry is %dx%d", cols, rows);
	if (strcmp(argv[1], "show") ) {
		char *p;

		p = strchr(argv[1], 'x');
		if (p) {
			rows = strtol(++p, NULL, 10);
			cols = strtol(argv[1], NULL, 10);

			pktgen_display_set_geometry(rows, cols);
			pktgen_cls();
		} else
			pktgen_log_error(
				"Geometry string is invalid (%s) must be CxR format",
				argv[1]);
		pktgen_display_get_geometry(&rows, &cols);
		pktgen_log_debug("New geometry is %dx%d", cols, rows);
	}
	return 0;
}

/**************************************************************************//**
 *
 * cmd_dev_parsed - Display the PCI bus devices.
 *
 * DESCRIPTION
 * Display all of the PCI bus devices.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
dev_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv __rte_unused)
{
	rte_eal_devargs_dump(stdout);
	return 0;
}

/**************************************************************************//**
 *
 * cmd_pci_parsed - Display the PCI bus devices.
 *
 * DESCRIPTION
 * Display all of the PCI bus devices.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pci_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv __rte_unused)
{
	rte_eal_pci_dump(stdout);
	return 0;
}

/**************************************************************************//**s
 *
 * range_mac_dest_cmd - Set the Destination MAC address
 *
 * DESCRIPTION
 * Set the destination MAC address for given port(s).
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_mac_dest_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     range_set_dest_mac(info, argv[1], rte_ether_aton((const char *)argv[3], NULL)));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_src_mac_parsed - Set the source MAC address
 *
 * DESCRIPTION
 * Set the source MAC address values.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
mac_src_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     range_set_src_mac(info, argv[1], rte_ether_aton((const char *)argv[3], NULL)));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_src_ip_parsed - Set the source IP address.
 *
 * DESCRIPTION
 * Set the source IP address.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
ip_src_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;
	rte_ipaddr_t ip;

	rte_parse_portlist(argv[2], &portlist);
	rte_atoip(argv[3], RTE_IPADDR_V4 , &ip, sizeof(ip));
	foreach_port(portlist,
		     range_set_src_ip(info, argv[1], &ip));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_dst_ip_parsed - Set the destination IP address.
 *
 * DESCRIPTION
 * Set the destination IP address.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
ip_dst_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;
	rte_ipaddr_t ip;

	rte_parse_portlist(argv[2], &portlist);
	rte_atoip(argv[3], RTE_IPADDR_V4 , &ip, sizeof(ip));
	foreach_port(portlist,
		     range_set_dst_ip(info, argv[1], &ip));

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_src_port_parsed - Set the source port value.
 *
 * DESCRIPTION
 * Set the port source value.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
src_port_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     range_set_src_port(info, argv[1], atoi(argv[3])) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_ip_proto_parsed - Set IP Protocol type
 *
 * DESCRIPTION
 * Set the IP protocol type
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
ip_proto_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     range_set_proto(info, argv[2][0]) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_dst_ip_port_parsed - Set the destination port value
 *
 * DESCRIPTION
 * Set the destination port value.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
dst_port_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     range_set_dst_port(info, argv[1], atoi(argv[3])) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_vlan_id_parsed - Set the vlan id value
 *
 * DESCRIPTION
 * Set the vlan id value
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
vlan_id_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     range_set_vlan_id(info, argv[1], atoi(argv[3])) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_pkt_size_parsed - Set the PKT Size value
 *
 * DESCRIPTION
 * Set the PKT Size value
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
range_pkt_size_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     range_set_pkt_size(info, argv[1], atoi(argv[3])) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_parsed - Set a value for a set of options.
 *
 * DESCRIPTION
 * Set a value for a given set of variables.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;
	char *what = argv[2];
	int value = atoi(argv[3]);

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist, _do(
			     if (!strcmp(what, "count"))
				     single_set_tx_count(info, value);
			     else if (!strcmp(what, "size"))
				     single_set_pkt_size(info, value);
			     else if (!strcmp(what, "rate"))
				     single_set_tx_rate(info, value);
			     else if (!strcmp(what, "burst"))
				     single_set_tx_burst(info, value);
			     else if (!strcmp(what, "tx_cycles"))
				     debug_set_tx_cycles(info, value);
			     else if (!strcmp(what, "sport"))
				     single_set_port_value(info, what[0], value);
			     else if (!strcmp(what, "dport"))
				     single_set_port_value(info, what[0], value);
			     else if (!strcmp(what, "seqCnt"))
				     pktgen_set_port_seqCnt(info, value);
			     else if (!strcmp(what, "prime"))
				     pktgen_set_port_prime(info, value);
			     else if (!strcmp(what, "dump"))
				     debug_set_port_dump(info, value);
			     else if (!strcmp(what, "vlanid"))
				     single_set_vlan_id(info, value);
			     ) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_pcap_onoff - Enable/Disable PCAP sending on a given port list.
 *
 * DESCRIPTION
 * Enable/Disable PCAP sending of data for a given port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pcap_onoff_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pcap_enable_disable(info, argv[2]) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_pcap_filter - Set PCAP port filtering on a set of ports.
 *
 * DESCRIPTION
 * Compile a filter for a set of ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pcap_filter_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pcap_filter(info, argv[2]) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_pcap_show - Show PCAP information.
 *
 * DESCRIPTION
 * Show PCAP information.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pcap_show_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv __rte_unused)
{
	if (pktgen.info[pktgen.portNum].pcap)
		_pcap_info(pktgen.info[pktgen.portNum].pcap, pktgen.portNum, 1);
	else
		pktgen_log_error(" ** PCAP file is not loaded on port %d",
				 pktgen.portNum);
	return 0;
}

/**************************************************************************//**
 *
 * cmd_pcap_index - Set PCAP index value
 *
 * DESCRIPTION
 * Set PCAP index value.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pcap_index_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	pcap_info_t   *pcap = pktgen.info[pktgen.portNum].pcap;
	uint32_t max_cnt = pcap->pkt_count;
	uint32_t value = strtoul(argv[1], NULL, 10);

	if (pcap) {
		if (value >= max_cnt)
			pcap->pkt_idx = max_cnt - RTE_MIN(PCAP_PAGE_SIZE, (int)max_cnt);
		else
			pcap->pkt_idx = value;
		pktgen.flags |= PRINT_LABELS_FLAG;
	} else
		pktgen_log_error(" ** PCAP file is not loaded on port %d",
				 pktgen.portNum);
	return 0;
}

/**************************************************************************//**
 *
 * cmd_blink_onoff_parsed - Enable/disable blinking a port led.
 *
 * DESCRIPTION
 * Enable/disable blinking a port led.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
blink_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     debug_blink(info, argv[2]) );

	if (pktgen.blinklist)
		pktgen.flags |= BLINK_PORTS_FLAG;
	else
		pktgen.flags &= ~BLINK_PORTS_FLAG;
	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_garp_onoff - Enable/Disable GARP packet processing
 *
 * DESCRIPTION
 * Enable/Disable packet GARP processing for ARP, ICMP and a number of other
 * related processing for a given port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
garp_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_garp(info, argv[2]) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_process_onoff - Enable/Disable input packet processing for ARP, ICMP, ...
 *
 * DESCRIPTION
 * Enable/Disable packet input processing for ARP, ICMP and a number of other
 * related processing for a given port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
process_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_process(info, argv[2]) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_ppp_parsed - Set the number of port per page to display
 *
 * DESCRIPTION
 * Set the number of ports per page to display.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
ppp_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{

	pktgen_set_page_size(atoi(argv[1]));
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
 * cmd_setip_dst_parsed - Set the IP address for the main single packet.
 *
 * DESCRIPTION
 * Set the primary IP address for the single packet type based on src/dst flags.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
ip_dst_set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;
	rte_ipaddr_t ip;

	rte_parse_portlist(argv[3], &portlist);
	rte_atoip(argv[3], RTE_IPADDR_V4, &ip, sizeof(ip));
	foreach_port(portlist,
		     single_set_ipaddr(info, argv[2][0], &ip) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_setip_src_parsed - Set the IP address for the main single packet.
 *
 * DESCRIPTION
 * Set the primary IP address for the single packet type based on src/dst flags.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
ip_src_set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;
	rte_ipaddr_t ip;

	rte_parse_portlist(argv[3], &portlist);
	rte_atoip(argv[3], RTE_IPADDR_V4, &ip, sizeof(ip));
	foreach_port(portlist,
		     single_set_ipaddr(info, argv[2][0], &ip) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_send_arp_parsed - Send a ARP request on a given port list.
 *
 * DESCRIPTION
 * Send an ARP request or gratuitous ARP packet for a given port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
send_arp_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;
	char *what = argv[1];

	rte_parse_portlist(argv[2], &portlist);
	if (what[0] == 'g')
		foreach_port(portlist,
			     pktgen_send_arp_requests(info, GRATUITOUS_ARP) );
	else
		foreach_port(portlist,
			     pktgen_send_arp_requests(info, 0) );
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_proto_parsed - Set the protocol type for a packet.
 *
 * DESCRIPTION
 * Set the protocol type for a set of given ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
set_proto_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     single_set_proto(info, argv[1][0]) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_load_parsed - load a command or script file to be executed.
 *
 * DESCRIPTION
 * Load and execute a set of commands or a script file.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
load_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	if (cli_load_cmds(cli, argv[1]) )
		cli_printf(cli, "load command failed for %s\n", argv[1]);
	if (!scrn_is_paused() )
		pktgen_redisplay(0);
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

/**************************************************************************//**
 *
 * cmd_screen_parsed - Enable or Disable the screen updates.
 *
 * DESCRIPTION
 * Enable or disable screen updates.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
screen_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	pktgen_screen(argv[1]);
	return 0;
}

/**************************************************************************//**
 *
 * cmd_tx_debug_parsed - Toggle TX debug data
 *
 * DESCRIPTION
 * Toggle TX debug data
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
tx_debug_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv __rte_unused)
{
	if ( (pktgen.flags & TX_DEBUG_FLAG) == 0)
		pktgen.flags |= TX_DEBUG_FLAG;
	else
		pktgen.flags &= ~TX_DEBUG_FLAG;
	pktgen_cls();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_l2p_parsed - Display the l2p table information
 *
 * DESCRIPTION
 * Display the l2p table information.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
l2p_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv __rte_unused)
{
	pktgen_l2p_dump();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_mempool_parsed - Display the memory pool information
 *
 * DESCRIPTION
 * Display the memory pool information.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
mempool_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[2], &portlist);
	if (!strcmp(argv[1], "dump") )
		foreach_port(portlist,
			     debug_mempool_dump(info, argv[3]) );
	return 0;
}

/**************************************************************************//**
 *
 * cmd_set_pkt_type_parsed - Set the packet type for a port IPv4 or IPv6
 *
 * DESCRIPTION
 * Set the ports to the given type IPv4 or IPv6.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
pkt_type_set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     single_set_pkt_type(info, argv[1]) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_icmp_echo_parsed - Enable or Disable the processing of ICMP packets
 *
 * DESCRIPTION
 * Enable or disable the processing of ICMP echo requests.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
icmp_echo_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		enable_icmp_echo(info, parseState(argv[1])));
	return 0;
}

/**************************************************************************//**
 *
 * cmd_capture_parsed - Enable or Disable packet capturing
 *
 * DESCRIPTION
 * Enable or Disable packet capturing
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
capture_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pktgen_set_capture(info, parseState(argv[2])) );
	return 0;
}

/**************************************************************************//**
 *
 * cmd_rx_tap_parsed - Enable or Disable the Rx TAP interface option
 *
 * DESCRIPTION
 * Enable or Disable the Rx TAP interface option
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
rx_tap_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_rx_tap(info, parseState(argv[2])) );
	return 0;
}

/**************************************************************************//**
 *
 * cmd_tx_tap_parsed - Enable or Disable the Tx TAP interface option
 *
 * DESCRIPTION
 * Enable or Disable the Tx TAP interface option
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
tx_tap_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_tx_tap(info, parseState(argv[2])) );
	return 0;
}

/**************************************************************************//**
 *
 * cmd_vlan_parsed - Enable or Disable sending VLAN ID on each packet
 *
 * DESCRIPTION
 * Enable or Disable sending the VLAN ID on each packet
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
vlan_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_vlan(info, parseState(argv[2])) );
	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_vlanid_parsed - Set the VLAN ID for a given port
 *
 * DESCRIPTION
 * Set the VLAN ID value for each port given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
vlanid_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     single_set_vlan_id(info, atoi(argv[2])) );
	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_mpls_parsed - Enable or Disable sending mpls ID on each packet
 *
 * DESCRIPTION
 * Enable or Disable sending the mpls ID on each packet
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
mpls_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_mpls(info, parseState(argv[2])) );
	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_mpls_entry_parsed - Set the MPLS entry for a given port
 *
 * DESCRIPTION
 * Set the VLAN ID value for each port given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
mpls_entry_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;
	uint32_t entry = strtoul(argv[2], NULL, 16);

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     range_set_mpls_entry(info, entry) );
	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_qinq_parsed - Enable or Disable sending Q-in-Q tag on each packet
 *
 * DESCRIPTION
 * Enable or Disable sending the Q-in-Q tag on each packet
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
qinq_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_qinq(info, parseState(argv[2])) );
	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_qinqids_parsed - Set the Q-in-Q ID's for a given port
 *
 * DESCRIPTION
 * Set the Q-in-Q ID values for each port given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
qinqids_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     range_set_qinqids(info, atoi(argv[2]), atoi(argv[3])) );
	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_gre_parsed - Enable or Disable GRE with IPv4 payload
 *
 * DESCRIPTION
 * Enable or Disable GRE with IPv4 payload
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
gre_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_gre(info, parseState(argv[2])) );
	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_gre_eth_parsed - Enable or Disable GRE with Ethernet payload
 *
 * DESCRIPTION
 * Enable or Disable GRE with Ethernet payload
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
gre_eth_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     enable_gre_eth(info, parseState(argv[2])) );
	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_gre_key_parsed - Set the GRE key for a given port
 *
 * DESCRIPTION
 * Set the GRE key for each port given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
gre_key_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     range_set_gre_key(info, strtoul(argv[2], NULL, 10)) );
	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_mac_from_arp_parsed - Enable or Disable the ARP packets setting the MAC address
 *
 * DESCRIPTION
 * Enable or disable having ARP packets set the MAC address.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
mac_from_arp_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t onOff = parseState(argv[1]);

	enable_mac_from_arp(onOff);
	return 0;
}
/**************************************************************************//**
 *
 * cmd_delay_parsed - Delay the script for a given number of milli-seconds
 *
 * DESCRIPTION
 * Delay the script processing for a given number of milli-seconds.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
delay_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	_delay_ms(atoi(argv[1]));
	return 0;
}

/**************************************************************************//**
 *
 * cmd_sleep_parsed - Sleep the script for a given number of seconds
 *
 * DESCRIPTION
 * Sleep the script processing for a given number of seconds.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
sleep_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	_sleep(atoi(argv[1]));
	return 0;
}

/**************************************************************************//**
 *
 * cmd_setmac_parsed - Set the single packet MAC address
 *
 * DESCRIPTION
 * Set the single packet MAC address.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
mac_set_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[2], &portlist);
	foreach_port(portlist,
		     single_set_dst_mac(info, rte_ether_aton(argv[3], NULL)) );

	pktgen_update_display();
	return 0;
}

/**************************************************************************//**
 *
 * cmd_start_parsed - Start sending packets in a given port list.
 *
 * DESCRIPTION
 * Start sending packets on a given port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
start_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pktgen_start_transmitting(info) );
	return 0;
}

/**************************************************************************//**
 *
 * cmd_stop_parsed - Stop ports from sending packets on a given port list
 *
 * DESCRIPTION
 * Stop ports from sending packetss on a given port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
stop_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pktgen_stop_transmitting(info) );
	return 0;
}

/**************************************************************************//**
 *
 * cmd_prime_parsed - Send a small number of packets to prime the forwarding tables.
 *
 * DESCRIPTION
 * Send a small number of packets on a given port list to prime the fowarding tables.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
prime_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pktgen_prime_ports(info) );
	return 0;
}

/**************************************************************************//**
 *
 * cmd_clear_parsed - Clear the statistics on all ports.
 *
 * DESCRIPTION
 * Clear all statistics on all ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
clear_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pktgen_clear_stats(info) );
	return 0;
}

/**************************************************************************//**
 *
 * cmd_reset_parsed - Reset Pktgen to the default configuration state.
 *
 * DESCRIPTION
 * Reset Pktgen to the default configuration state.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
reset_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pktgen_reset(info) );
	return 0;
}

/**************************************************************************//**
 *
 * cmd_port_restart_parsed - Port Reset
 *
 * DESCRIPTION
 * Port Reset
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

static int
restart_port_cmd(struct cli *cli __rte_unused, int argc __rte_unused, char **argv)
{
	uint32_t portlist;

	rte_parse_portlist(argv[1], &portlist);
	foreach_port(portlist,
		     pktgen_port_restart(info) );
	return 0;
}

/**********************************************************/
/**********************************************************/
/****** CONTEXT (list of instruction) */

static struct cli_tree default_tree[] = {
	c_dir("/bin"),
	c_cmd("blink", 		blink_cmd,		"blink the leds in the ports"),
	c_cmd("clear",		clear_cmd,		""),
	c_cmd("delay",		delay_cmd, ""),
	c_cmd("geom",		geometry_cmd, ""),
	c_cmd("load",		load_cmd, ""),
	c_cmd("script", 	script_cmd, ""),
	c_cmd("exec.lua", 	exec_lua_cmd, ""),
	c_cmd("save", 		save_cmd, ""),

	c_dir("/pktgen/bin"),
	c_cmd("prime", 		prime_cmd, ""),
	c_cmd("process", 	process_cmd, ""),
	c_cmd("mac_arp",	mac_from_arp_cmd, ""),
	c_cmd("page",		page_set_cmd, ""),
	c_cmd("pdump", 		pdump_cmd, ""),
	c_cmd("help",		help_cmd, 		"help command"),
	c_cmd("set", 		set_cmd, ""),
	c_cmd("reset", 		reset_cmd, ""),
	c_cmd("restart", 	restart_port_cmd, ""),
	c_cmd("screen", 	screen_cmd, ""),
	c_cmd("mpls", mpls_cmd, ""),
	c_cmd("port", port_cmd, ""),
	c_cmd("ppp", ppp_cmd, ""),
	c_cmd("sleep", sleep_cmd, ""),
	c_cmd("start", start_cmd, ""),
	c_cmd("stop", stop_cmd, ""),
	c_cmd("capture", capture_cmd, ""),
	c_cmd("rx_tap", rx_tap_cmd, ""),
	c_cmd("tx_tap", tx_tap_cmd, ""),
	c_cmd("vlan", vlan_cmd, ""),
	c_cmd("garp", garp_cmd, ""),
	c_cmd("rnd", rnd_cmd, ""),
	c_cmd("send.arp", send_arp_cmd, ""),
	c_cmd("seq", seq_set_cmd, ""),
	c_cmd("qinq", qinq_cmd, ""),
	c_cmd("gre", gre_cmd, ""),
	c_cmd("gre_eth", gre_eth_cmd, ""),
	c_cmd("pattern", pattern_cmd, ""),
	c_cmd("pattern.user", pattern_user_cmd, ""),
	c_cmd("latency", latency_set_cmd, ""),
	c_cmd("jitter", jitter_set_cmd, ""),
	c_cmd("icmp.echo", 	icmp_echo_cmd, ""),
	c_cmd("ping4", 		ping4_cmd, ""),
#ifdef INCLUDE_PING6
	c_cmd("ping6", 		ping6_cmd, ""),
#endif

	c_dir("/pktgen/single"),
	c_cmd("pkt.type", 	pkt_type_set_cmd, ""),
	c_cmd("ip", 		ip_dst_cmd, ""),
	c_cmd("port.dst", 	dst_port_cmd, ""),
	c_cmd("set.proto", set_proto_cmd, ""),
	c_cmd("ip.dst", ip_dst_set_cmd, ""),
	c_cmd("ip.src", ip_src_cmd, ""),
	c_cmd("src.ip", ip_src_set_cmd, ""),
	c_cmd("mac", mac_set_cmd, ""),
	c_cmd("vlan.id", vlanid_cmd, ""),

	c_dir("/pktgen/pcap"),
	c_cmd("pcap.inded", pcap_index_cmd, ""),
	c_cmd("pcap", pcap_onoff_cmd, ""),
	c_cmd("pcap.show", pcap_show_cmd, ""),
	c_cmd("pcap.filter", pcap_filter_cmd, ""),

	c_dir("/pktgen/range"),
	c_cmd("range", range_set_cmd, ""),
	c_cmd("range.size", range_pkt_size_cmd, ""),
	c_cmd("mac.dest", 	range_mac_dest_cmd, ""),
	c_cmd("ip_proto", ip_proto_cmd, ""),
	c_cmd("mac.src", mac_src_cmd, ""),
	c_cmd("port.src", src_port_cmd, ""),
	c_cmd("vlan_id", vlan_id_cmd, ""),
	c_cmd("mpls.entry", mpls_entry_cmd, ""),
	c_cmd("qinq.idx", qinqids_cmd, ""),
	c_cmd("gre_key", gre_key_cmd, ""),

	c_dir("/pktgen/debug"),
	c_cmd("l2p", l2p_cmd, ""),
	c_cmd("tx_debug", tx_debug_cmd, ""),
	c_cmd("mempool",	mempool_cmd, ""),
	c_cmd("pci",		pci_cmd, ""),
	c_cmd("dev",		dev_cmd, ""),

	c_dir("/pktgen/theme"),
	c_cmd("theme.save", theme_save_cmd, ""),
	c_cmd("theme.show", theme_show_cmd, ""),
	c_cmd("theme.set", theme_set_cmd, ""),
	c_cmd("theme", theme_state_cmd, ""),

	c_end()
};

static int
init_tree(struct cli *cli)
{
    if (cli_default_tree_init(cli))
        return -1;

    if (cli_add_tree(cli, cli_root_node(cli), default_tree))
        return -1;

    if (cli_add_bin_path(cli, "/bin"))
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
