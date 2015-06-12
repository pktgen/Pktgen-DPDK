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

/*
 * Copyright (c) 2009, Olivier MATZ <zer0@droids-corp.org>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Copyright (c) <2010-2014>, Wind River Systems, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3) Neither the name of Wind River Systems nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 4) The screens displayed by the application must contain the copyright notice as defined
 * above and can not be removed without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Created 2010 by Keith Wiles @ intel.com */


#include "commands.h"

#include <stdio.h>
#include <termios.h>	// cmdline.h uses struct termios
#include <string.h>
#include <unistd.h>

#include <cmdline_rdline.h>
#include <cmdline_parse_string.h>
#include <cmdline.h>
#include <rte_atomic.h>
#include <rte_devargs.h>

#include "wr_copyright_info.h"
#include "pktgen-cmds.h"
#include "pktgen-main.h"
#include "lpktgenlib.h"
#include "pktgen-display.h"
#include "pktgen-random.h"
#include "pktgen-log.h"

#include "pktgen.h"


cmdline_parse_ctx_t main_ctx[];


/**************************************************************************//**
*
* cmd_port_display - Create a string based on the port list.
*
* DESCRIPTION
* Create a string based on the port list and use the short hand format.
*
* RETURNS: Null if ERROR or a string pointer to the port list.
*
* SEE ALSO:
*/

char *
cmd_port_display(char * buff, uint32_t len, uint64_t portlist) {

	char	  * p = buff;
	uint32_t	bit = 0, first, comma = 0;

	buff[0] = '\0';
	while( bit < (sizeof(portlist) << 3) ) {
		if ( (portlist & (1ULL << bit++)) == 0 )
			continue;
		first = (bit-1);
		while( (portlist & (1ULL << bit)) )
			bit++;
		if ( first == (bit-1) )
			snprintf(p, len - strlen(buff), "%s%d", (comma)?",":"", first);
		else
			snprintf(p, len - strlen(buff), "%s%d-%d", (comma)?",":"", first, (bit-1));

		p = buff + strlen(buff);
		if ( strlen(buff) > (len - 5) )
			break;
		comma = 1;
	}
	return buff;
}

 /**************************************************************************//**
 *
 * cmdline_pause - Pause the screen from scrolling and wait for a key.
 *
 * DESCRIPTION
 * Display a message and wait for a response.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
cmdline_pause(struct cmdline *cl, const char * msg)
{
	char c;
	int	n;

	cmdline_printf(cl, "%s", msg);
	n = read(cl->s_in, &c, 1);
	if ( n < 0 )
		return;
	cmdline_printf(cl, "\r");
	wr_scrn_eol();
}

/**********************************************************/
const char * help_info[] = {
		"", /* Leave blank not used */
		"   *** Help Information for Pktgen ***         %s\n",
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
		"seq <seq#> <portlist> dst-Mac src-Mac dst-IP src-IP sport dport ipv4|ipv6 udp|tcp|icmp vlan pktsize",
		"                                   - Set the sequence packet information, make sure the src-IP",
		"                                     has the netmask value eg 1.2.3.4/24",
		"save <path-to-file>                - Save a configuration file using the filename",
		"load <path-to-file>                - Load a command/script file from the given path",
		"ppp [1-6]                          - Set the number of ports displayed per page",
		"icmp.echo <portlist> <state>       - Enable/disable ICMP echo responses per port",
		"send arp req|grat <portlist>       - Send a ARP request or gratuitous ARP on a set of ports",
		"set mac <portlist> etheraddr       - Set MAC addresses 00:11:22:33:44:55",
		"                                     You can use 0011:2233:4455 format as well",
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
		"",
		"<<PageBreak>>",
		"gre <portlist> <state>             - Enable/disable GRE with IPv4 payload",
		"gre_eth <portlist> <state>         - Enable/disable GRE with Ethernet frame payload",
		"gre_key <portlist> <state>         - Set the GRE key",
		"pcap <portlist> <state>            - Enable or Disable sending pcap packets on a portlist",
		"pcap.show                          - Show the PCAP information",
		"pcap.index                         - Move the PCAP file index to the given packet number,  0 - rewind, -1 - end of file",
		"pcap.filter <portlist> <string>    - PCAP filter string to filter packets on receive",
		"script <filename>                  - Execute the Lua script code in file (www.lua.org).",
		"ping4 <portlist>                   - Send a IPv4 ICMP echo request on the given portlist",
#ifdef INCLUDE_PING6
		"ping6 <portlist>                   - Send a IPv6 ICMP echo request on the given portlist",
#endif
		"page [0-7]|main|range|config|seq|pcap|next|cpu|rnd- Show the port pages or configuration or sequence page",
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
		"       Flags: P--------------- - Promiscuous mode enabled",
		"               E               - ICMP Echo enabled",
		"                A              - Send ARP Request flag",
		"                 G             - Send Gratuitous ARP flag",
		"                  C            - TX Cleanup flag",
		"                   p           - PCAP enabled flag",
		"                    S          - Send Sequence packets enabled",
		"                     R         - Send Range packets enabled",
		"                      D        - DPI Scanning enabled (If Enabled)",
		"                       I       - Process packets on input enabled",
		"                        T      - Using TAP interface for this port",
		"                         V     - Send VLAN ID tag",
		"                         M     - Send MPLS header",
		"                         Q     - Send Q-in-Q tags",
		"                          g    - Process GARP packets",
		"                           g   - Perform GRE with IPv4 payload",
		"                           G   - Perform GRE with Ethernet payload",
		"                            C  - Capture received packets",
		"                             R - Random bitfield(s) are applied",
		"Notes: <state>       - Use enable|disable or on|off to set the state.",
		"       <portlist>    - a list of ports (no spaces) as 2,4,6-9,12 or 3-5,8 or 5 or the word 'all'",
        "       Color best seen on a black background for now",
		"       To see a set of example Lua commands see the files in wr-examples/pktgen/test",
		"",
		NULL
};

struct cmd_help_result {
	cmdline_fixed_string_t help;
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

static void cmd_help_parsed(__attribute__((unused)) void *parsed_result,
			    struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	int		i, paused;

	paused = wr_scrn_is_paused();

	if ( ! paused )
		wr_scrn_pause();
	wr_scrn_setw(1);
	wr_scrn_cls();

	wr_scrn_pos(0,0);
	cmdline_printf(cl, help_info[1], wr_copyright_msg());
	wr_scrn_pos(3,0);
	for(i=2; help_info[i] != NULL; i++ ) {
		if ( strcmp(help_info[i], "<<PageBreak>>") == 0 ) {
			cmdline_pause(cl, "   <More Help: Press Return to Continue>");
			wr_scrn_cls();
			wr_scrn_pos(0,0);
			cmdline_printf(cl, help_info[1], wr_copyright_msg());
			wr_scrn_pos(3,0);
			continue;
		}
		cmdline_printf(cl, "%s\n", help_info[i]);
	}

	cmdline_pause(cl, "   <Press Return to Continue>");

	if ( !paused ) {
		wr_scrn_setw(pktgen.last_row+1);
		wr_scrn_resume();
		pktgen_redisplay(1);
	}
}

cmdline_parse_token_string_t cmd_help_help =
	TOKEN_STRING_INITIALIZER(struct cmd_help_result, help, "help");

cmdline_parse_inst_t cmd_help = {
	.f = cmd_help_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "show help",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_help,
		NULL,
	},
};

/**********************************************************/

struct cmd_theme_state_result {
	cmdline_fixed_string_t theme;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_theme_state_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_theme_state_result *res = parsed_result;

	pktgen_theme_state(res->onOff);
	pktgen_cls();
}

cmdline_parse_token_string_t cmd_theme_theme =
	TOKEN_STRING_INITIALIZER(struct cmd_theme_state_result, theme, "theme");
cmdline_parse_token_string_t cmd_theme_onOff =
	TOKEN_STRING_INITIALIZER(struct cmd_theme_state_result, onOff, "stop#start#off#on#enable#disable");

cmdline_parse_inst_t cmd_theme_state = {
	.f = cmd_theme_state_parsed,
	.data = NULL,
	.help_str = "theme <state>",
	.tokens = {
		(void *)&cmd_theme_theme,
		(void *)&cmd_theme_onOff,
		NULL,
	},
};

/**********************************************************/

struct cmd_theme_result {
	cmdline_fixed_string_t theme;
	cmdline_fixed_string_t item;
	cmdline_fixed_string_t fg;
	cmdline_fixed_string_t bg;
	cmdline_fixed_string_t attr;
};

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

static void cmd_theme_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_theme_result *res = parsed_result;

	pktgen_set_theme_item(res->item, res->fg, res->bg, res->attr);
}

cmdline_parse_token_string_t cmd_theme_data =
	TOKEN_STRING_INITIALIZER(struct cmd_theme_result, theme, "theme");
cmdline_parse_token_string_t cmd_theme_item =
	TOKEN_STRING_INITIALIZER(struct cmd_theme_result, item, NULL);
cmdline_parse_token_string_t cmd_theme_fg =
	TOKEN_STRING_INITIALIZER(struct cmd_theme_result, fg, NULL);
cmdline_parse_token_string_t cmd_theme_bg =
	TOKEN_STRING_INITIALIZER(struct cmd_theme_result, bg, NULL);
cmdline_parse_token_string_t cmd_theme_attr =
	TOKEN_STRING_INITIALIZER(struct cmd_theme_result, attr, NULL);

cmdline_parse_inst_t cmd_theme = {
	.f = cmd_theme_parsed,
	.data = NULL,
	.help_str = "theme <item> <fg> <bg> <attr>",
	.tokens = {
		(void *)&cmd_theme_data,
		(void *)&cmd_theme_item,
		(void *)&cmd_theme_fg,
		(void *)&cmd_theme_bg,
		(void *)&cmd_theme_attr,
		NULL,
	},
};

/**********************************************************/

struct cmd_theme_show_result {
	cmdline_fixed_string_t theme_show;
};

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

static void cmd_theme_show_parsed(__attribute__((unused)) void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	pktgen_theme_show();
}

cmdline_parse_token_string_t cmd_theme_show_data =
	TOKEN_STRING_INITIALIZER(struct cmd_theme_show_result, theme_show, "theme.show");

cmdline_parse_inst_t cmd_theme_show = {
	.f = cmd_theme_show_parsed,
	.data = NULL,
	.help_str = "theme.show",
	.tokens = {
		(void *)&cmd_theme_show_data,
		NULL,
	},
};

/**********************************************************/

struct cmd_theme_save_result {
	cmdline_fixed_string_t theme_save;
	cmdline_fixed_string_t filename;
};

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

static void cmd_theme_save_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_theme_save_result *res = parsed_result;

	pktgen_theme_save(res->filename);
}

cmdline_parse_token_string_t cmd_theme_save_data =
	TOKEN_STRING_INITIALIZER(struct cmd_theme_save_result, theme_save, "theme.save");
cmdline_parse_token_string_t cmd_theme_save_filename =
	TOKEN_STRING_INITIALIZER(struct cmd_theme_save_result, filename, NULL);

cmdline_parse_inst_t cmd_theme_save = {
	.f = cmd_theme_save_parsed,
	.data = NULL,
	.help_str = "theme.save filename",
	.tokens = {
		(void *)&cmd_theme_save_data,
		(void *)&cmd_theme_save_filename,
		NULL,
	},
};

/**********************************************************/

struct cmd_save_result {
	cmdline_fixed_string_t save;
	cmdline_fixed_string_t filename;
};

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

static void cmd_save_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_save_result *res = parsed_result;

	pktgen_save(res->filename);
}

cmdline_parse_token_string_t cmd_save_data =
	TOKEN_STRING_INITIALIZER(struct cmd_save_result, save, "save");
cmdline_parse_token_string_t cmd_save_filename =
	TOKEN_STRING_INITIALIZER(struct cmd_save_result, filename, NULL);

cmdline_parse_inst_t cmd_save = {
	.f = cmd_save_parsed,
	.data = NULL,
	.help_str = "save filename",
	.tokens = {
		(void *)&cmd_save_data,
		(void *)&cmd_save_filename,
		NULL,
	},
};

/**********************************************************/

struct cmd_scripting_result {
	cmdline_fixed_string_t script;
	cmdline_fixed_string_t filename;
};

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

static void cmd_script_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_scripting_result *res = parsed_result;
	lua_State *L = pktgen.L;

	if ( L == NULL ) {
		pktgen_log_error("Lua is not initialized!");
		return;
	}

	if ( luaL_dofile(L, res->filename) != 0 )
		pktgen_log_error("%s", lua_tostring(L,-1));
}

cmdline_parse_token_string_t cmd_script_cmds =
	TOKEN_STRING_INITIALIZER(struct cmd_scripting_result, script, "script");
cmdline_parse_token_string_t cmd_script_filename =
		TOKEN_STRING_INITIALIZER(struct cmd_scripting_result, filename, NULL);

cmdline_parse_inst_t cmd_script = {
	.f = cmd_script_parsed,
	.data = NULL,
	.help_str = "script <filename>",
	.tokens = {
		(void *)&cmd_script_cmds,
		(void *)&cmd_script_filename,
		NULL,
	},
};

/**********************************************************/

struct cmd_ping4_result {
	cmdline_fixed_string_t ping4;
	cmdline_portlist_t portlist;
};

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

static void cmd_ping4_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_ping4_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_ping4(info) );
	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_ping4 =
	TOKEN_STRING_INITIALIZER(struct cmd_ping4_result, ping4, "ping4");
cmdline_parse_token_portlist_t cmd_ping4_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_ping4_result, portlist);

cmdline_parse_inst_t cmd_ping4 = {
	.f = cmd_ping4_parsed,
	.data = NULL,
	.help_str = "ping4 <portlist>",
	.tokens = {
		(void *)&cmd_set_ping4,
		(void *)&cmd_ping4_portlist,
		NULL,
	},
};

#ifdef INCLUDE_PING6
/**********************************************************/

struct cmd_ping6_result {
	cmdline_fixed_string_t ping6;
	cmdline_portlist_t portlist;
};

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

static void cmd_ping6_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_ping6_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_ping6(info) );
	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_ping6 =
	TOKEN_STRING_INITIALIZER(struct cmd_ping6_result, ping6, "ping6");
cmdline_parse_token_portlist_t cmd_ping6_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_ping6_result, portlist);

cmdline_parse_inst_t cmd_ping6 = {
	.f = cmd_ping6_parsed,
	.data = NULL,
	.help_str = "ping6 <portlist>",
	.tokens = {
		(void *)&cmd_set_ping6,
		(void *)&cmd_ping6_portlist,
		NULL,
	},
};
#endif

/**********************************************************/

struct cmd_set_range_result {
	cmdline_fixed_string_t range;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t what;
};

/**************************************************************************//**
*
* cmd_set_range_parsed - Set the range command options.
*
* DESCRIPTION
* Set the range port options.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void cmd_set_range_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_set_range_result *res = parsed_result;


	foreach_port( res->portlist.map,
		pktgen_range_enable_disable(info, res->what) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_range =
	TOKEN_STRING_INITIALIZER(struct cmd_set_range_result, range, "range");
cmdline_parse_token_portlist_t cmd_set_range_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_set_range_result, portlist);
cmdline_parse_token_string_t cmd_set_range_what =
	TOKEN_STRING_INITIALIZER(struct cmd_set_range_result, what, "enable#disable#on#off");

cmdline_parse_inst_t cmd_range = {
	.f = cmd_set_range_parsed,
	.data = NULL,
	.help_str = "range <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_range,
		(void *)&cmd_set_range_portlist,
		(void *)&cmd_set_range_what,
		NULL,
	},
};


/**********************************************************/

struct cmd_rnd_result {
	cmdline_fixed_string_t rnd;
	cmdline_portlist_t portlist;
	uint8_t idx;
	uint8_t off;
	cmdline_fixed_string_t mask;
};

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

static void cmd_rnd_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_rnd_result *res = parsed_result;

	char mask[33] = { 0 };
	int i, mask_idx = 0;
	char curr_bit;

	if (strcmp(res->mask, "off")) {
		/* Filter invalid characters from provided mask. This way the user can
		 * more easily enter long bitmasks, using for example '_' as a separator
		 * every 8 bits. */
		for (i = 0; (mask_idx < 32) && ((curr_bit = res->mask[i]) != '\0'); ++i) {
			if ((curr_bit == '0') || (curr_bit == '1') || (curr_bit == '.') || (curr_bit == 'X')) {
				mask[mask_idx++] = curr_bit;
			}
		}
	}

	foreach_port( res->portlist.map,
			pktgen_set_random(info,
				pktgen_set_random_bitfield(info->rnd_bitfields, res->idx, res->off, mask) ? ENABLE_STATE : DISABLE_STATE)
			);

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_rnd_rnd =
	TOKEN_STRING_INITIALIZER(struct cmd_rnd_result, rnd, "rnd");
cmdline_parse_token_portlist_t cmd_rnd_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_rnd_result, portlist);
cmdline_parse_token_num_t cmd_rnd_idx =
	TOKEN_NUM_INITIALIZER(struct cmd_rnd_result, idx, UINT8);
cmdline_parse_token_num_t cmd_rnd_off =
	TOKEN_NUM_INITIALIZER(struct cmd_rnd_result, off, UINT8);
cmdline_parse_token_string_t cmd_rnd_mask =
	TOKEN_STRING_INITIALIZER(struct cmd_rnd_result, mask, NULL);

cmdline_parse_inst_t cmd_rnd = {
	.f = cmd_rnd_parsed,
	.data = NULL,
	.help_str = "rnd <portlist> <idx> <off> <mask>",
	.tokens = {
		(void *)&cmd_rnd_rnd,
		(void *)&cmd_rnd_portlist,
		(void *)&cmd_rnd_idx,
		(void *)&cmd_rnd_off,
		(void *)&cmd_rnd_mask,
		NULL,
	},
};

/**********************************************************/

struct cmd_set_geometry_result {
	cmdline_fixed_string_t geometry;
	cmdline_fixed_string_t what;
};

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

static void cmd_set_geometry_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_set_geometry_result *res = parsed_result;

	uint16_t rows, cols;
	pktgen_display_get_geometry(&rows, &cols);
	pktgen_log_debug("Old geometry is %dx%d", cols, rows);
	if ( strcmp(res->what, "show") ) {
		char * p;

		p = strchr(res->what, 'x');
		if ( p ) {
			rows = strtol(++p, NULL, 10);
			cols = strtol(res->what, NULL, 10);

			pktgen_display_set_geometry(rows, cols);
			pktgen_cls();
		} else {
			pktgen_log_error("Geometry string is invalid (%s) must be CxR format", res->what);
		}
		pktgen_display_get_geometry(&rows, &cols);
		pktgen_log_debug("New geometry is %dx%d", cols, rows);
	}
}

cmdline_parse_token_string_t cmd_set_geometry =
	TOKEN_STRING_INITIALIZER(struct cmd_set_geometry_result, geometry, "geometry");
cmdline_parse_token_string_t cmd_set_geometry_what =
	TOKEN_STRING_INITIALIZER(struct cmd_set_geometry_result, what, NULL);

cmdline_parse_inst_t cmd_geometry = {
	.f = cmd_set_geometry_parsed,
	.data = NULL,
	.help_str = "geometry [<geom>|show]",
	.tokens = {
		(void *)&cmd_set_geometry,
		(void *)&cmd_set_geometry_what,
		NULL,
	},
};

/**********************************************************/

struct cmd_dev_result {
	cmdline_fixed_string_t dev;
};

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

static void cmd_dev_parsed(__attribute__((unused)) void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	rte_eal_devargs_dump(stdout);
}

cmdline_parse_token_string_t cmd_dev_cmds =
	TOKEN_STRING_INITIALIZER(struct cmd_dev_result, dev, "dev.list");

cmdline_parse_inst_t cmd_dev = {
	.f = cmd_dev_parsed,
	.data = NULL,
	.help_str = "dev.list",
	.tokens = {
		(void *)&cmd_dev_cmds,
		NULL,
	},
};

/**********************************************************/

struct cmd_pci_result {
	cmdline_fixed_string_t pci;
};

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

static void cmd_pci_parsed(__attribute__((unused)) void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	rte_eal_pci_dump(stdout);
}

cmdline_parse_token_string_t cmd_pci_cmds =
	TOKEN_STRING_INITIALIZER(struct cmd_pci_result, pci, "pci.list");

cmdline_parse_inst_t cmd_pci = {
	.f = cmd_pci_parsed,
	.data = NULL,
	.help_str = "pci.list",
	.tokens = {
		(void *)&cmd_pci_cmds,
		NULL,
	},
};

/**********************************************************/

struct cmd_dest_mac_result {
	cmdline_fixed_string_t dst_mac;
	cmdline_fixed_string_t what;
	cmdline_portlist_t portlist;
	cmdline_etheraddr_t addr;
};

/**************************************************************************//**
*
* cmd_dest_mac_parsed - Set the Destination MAC address
*
* DESCRIPTION
* Set the destination MAC address for given port(s).
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void cmd_dest_mac_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_dest_mac_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_dest_mac(info, res->what, &res->addr) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_dest_mac =
	TOKEN_STRING_INITIALIZER(struct cmd_dest_mac_result, dst_mac, "dst.mac");
cmdline_parse_token_string_t cmd_dest_mac_what =
	TOKEN_STRING_INITIALIZER(struct cmd_dest_mac_result, what, "start#min#max#inc");
cmdline_parse_token_portlist_t cmd_dest_mac_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_dest_mac_result, portlist);
cmdline_parse_token_etheraddr_t cmd_dest_mac_addr =
	TOKEN_ETHERADDR_INITIALIZER(struct cmd_dest_mac_result, addr);

cmdline_parse_inst_t cmd_dest_mac = {
	.f = cmd_dest_mac_parsed,
	.data = NULL,
	.help_str = "dst.mac start|min|max|inc <portlist> ethaddr",
	.tokens = {
		(void *)&cmd_set_dest_mac,
		(void *)&cmd_dest_mac_what,
		(void *)&cmd_dest_mac_portlist,
		(void *)&cmd_dest_mac_addr,
		NULL,
	},
};

/**********************************************************/

struct cmd_src_mac_result {
	cmdline_fixed_string_t src_mac;
	cmdline_fixed_string_t what;
	cmdline_portlist_t portlist;
	cmdline_etheraddr_t addr;
};

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

static void cmd_src_mac_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_src_mac_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_src_mac(info, res->what, &res->addr) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_src_mac =
	TOKEN_STRING_INITIALIZER(struct cmd_src_mac_result, src_mac, "src.mac");
cmdline_parse_token_string_t cmd_src_mac_what =
	TOKEN_STRING_INITIALIZER(struct cmd_src_mac_result, what, "start#min#max#inc");
cmdline_parse_token_portlist_t cmd_src_mac_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_src_mac_result, portlist);
cmdline_parse_token_etheraddr_t cmd_src_mac_addr =
	TOKEN_ETHERADDR_INITIALIZER(struct cmd_src_mac_result, addr);

cmdline_parse_inst_t cmd_src_mac = {
	.f = cmd_src_mac_parsed,
	.data = NULL,
	.help_str = "src.mac start|min|max|inc <portlist> ethaddr",
	.tokens = {
		(void *)&cmd_set_src_mac,
		(void *)&cmd_src_mac_what,
		(void *)&cmd_src_mac_portlist,
		(void *)&cmd_src_mac_addr,
		NULL,
	},
};

/**********************************************************/

struct cmd_src_ip_result {
	cmdline_fixed_string_t src_ip;
	cmdline_fixed_string_t what;
	cmdline_portlist_t portlist;
	cmdline_ipaddr_t ipaddr;
};

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

static void cmd_src_ip_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_src_ip_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_src_ip(info, res->what, &res->ipaddr) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_src_ip =
	TOKEN_STRING_INITIALIZER(struct cmd_src_ip_result, src_ip, "src.ip");
cmdline_parse_token_string_t cmd_src_ip_what =
	TOKEN_STRING_INITIALIZER(struct cmd_src_ip_result, what, "start#min#max#inc");
cmdline_parse_token_portlist_t cmd_src_ip_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_src_ip_result, portlist);
cmdline_parse_token_ipaddr_t cmd_src_ip_addr =
	TOKEN_IPV4_INITIALIZER(struct cmd_src_ip_result, ipaddr);

cmdline_parse_inst_t cmd_src_ip = {
	.f = cmd_src_ip_parsed,
	.data = NULL,
	.help_str = "src.ip start|min|max|inc <portlist> ipaddr",
	.tokens = {
		(void *)&cmd_set_src_ip,
		(void *)&cmd_src_ip_what,
		(void *)&cmd_src_ip_portlist,
		(void *)&cmd_src_ip_addr,
		NULL,
	},
};

/**********************************************************/

struct cmd_dst_ip_result {
	cmdline_fixed_string_t dst_ip;
	cmdline_fixed_string_t what;
	cmdline_portlist_t portlist;
	cmdline_ipaddr_t ipaddr;
};

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

static void cmd_dst_ip_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_dst_ip_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_dst_ip(info, res->what, &res->ipaddr) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_dst_ip =
	TOKEN_STRING_INITIALIZER(struct cmd_dst_ip_result, dst_ip, "dst.ip");
cmdline_parse_token_string_t cmd_dst_ip_what =
	TOKEN_STRING_INITIALIZER(struct cmd_dst_ip_result, what, "start#min#max#inc");
cmdline_parse_token_portlist_t cmd_dst_ip_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_dst_ip_result, portlist);
cmdline_parse_token_ipaddr_t cmd_dst_ip_addr =
	TOKEN_IPV4_INITIALIZER(struct cmd_dst_ip_result, ipaddr);

cmdline_parse_inst_t cmd_dst_ip = {
	.f = cmd_dst_ip_parsed,
	.data = NULL,
	.help_str = "dst.ip start|min|max|inc <portlist> ipaddr",
	.tokens = {
		(void *)&cmd_set_dst_ip,
		(void *)&cmd_dst_ip_what,
		(void *)&cmd_dst_ip_portlist,
		(void *)&cmd_dst_ip_addr,
		NULL,
	},
};

/**********************************************************/

struct cmd_src_port_result {
	cmdline_fixed_string_t src_port;
	cmdline_fixed_string_t what;
	cmdline_portlist_t portlist;
	uint16_t port;
};

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

static void cmd_src_port_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_src_port_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_src_port(info, res->what, res->port) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_src_port =
	TOKEN_STRING_INITIALIZER(struct cmd_src_port_result, src_port, "src.port");
cmdline_parse_token_string_t cmd_src_port_what =
	TOKEN_STRING_INITIALIZER(struct cmd_src_port_result, what, "start#min#max#inc");
cmdline_parse_token_portlist_t cmd_src_port_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_src_port_result, portlist);
cmdline_parse_token_num_t cmd_src_port_addr =
	TOKEN_NUM_INITIALIZER(struct cmd_src_port_result, port, UINT16);

cmdline_parse_inst_t cmd_src_port = {
	.f = cmd_src_port_parsed,
	.data = NULL,
	.help_str = "src.port start|min|max|inc <portlist> port",
	.tokens = {
		(void *)&cmd_set_src_port,
		(void *)&cmd_src_port_what,
		(void *)&cmd_src_port_portlist,
		(void *)&cmd_src_port_addr,
		NULL,
	},
};

/**********************************************************/

struct cmd_dst_port_result {
	cmdline_fixed_string_t dst_port;
	cmdline_fixed_string_t what;
	cmdline_portlist_t portlist;
	uint16_t port;
};

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

static void cmd_dst_port_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_dst_port_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_dst_port(info, res->what, res->port) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_dst_port =
	TOKEN_STRING_INITIALIZER(struct cmd_dst_port_result, dst_port, "dst.port");
cmdline_parse_token_string_t cmd_dst_port_what =
	TOKEN_STRING_INITIALIZER(struct cmd_dst_port_result, what, "start#min#max#inc");
cmdline_parse_token_portlist_t cmd_dst_port_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_dst_port_result, portlist);
cmdline_parse_token_num_t cmd_dst_port_addr =
	TOKEN_NUM_INITIALIZER(struct cmd_dst_port_result, port, UINT16);

cmdline_parse_inst_t cmd_dst_port = {
	.f = cmd_dst_port_parsed,
	.data = NULL,
	.help_str = "dst.port start|min|max|inc <portlist> port",
	.tokens = {
		(void *)&cmd_set_dst_port,
		(void *)&cmd_dst_port_what,
		(void *)&cmd_dst_port_portlist,
		(void *)&cmd_dst_port_addr,
		NULL,
	},
};

/**********************************************************/

struct cmd_vlan_id_result {
	cmdline_fixed_string_t vlan_id;
	cmdline_fixed_string_t what;
	cmdline_portlist_t portlist;
	uint16_t id;
};

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

static void cmd_vlan_id_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_vlan_id_result *res = parsed_result;

	foreach_port( res->portlist.map,
			pktgen_set_vlan_id(info, res->what, res->id) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_vlan_id =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_id_result, vlan_id, "vlan.id");
cmdline_parse_token_string_t cmd_vlan_id_what =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_id_result, what, "start#min#max#inc");
cmdline_parse_token_portlist_t cmd_vlan_id_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_vlan_id_result, portlist);
cmdline_parse_token_num_t cmd_vlan_id_addr =
	TOKEN_NUM_INITIALIZER(struct cmd_vlan_id_result, id, UINT16);

cmdline_parse_inst_t cmd_vlan_id = {
	.f = cmd_vlan_id_parsed,
	.data = NULL,
	.help_str = "vlan.id start|min|max|inc <portlist> id",
	.tokens = {
		(void *)&cmd_set_vlan_id,
		(void *)&cmd_vlan_id_what,
		(void *)&cmd_vlan_id_portlist,
		(void *)&cmd_vlan_id_addr,
		NULL,
	},
};

/**********************************************************/

/**********************************************************/

struct cmd_pkt_size_result {
	cmdline_fixed_string_t pkt_size;
	cmdline_fixed_string_t what;
	cmdline_portlist_t portlist;
	uint16_t size;
};

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

static void cmd_pkt_size_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_pkt_size_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_range_pkt_size(info, res->what, res->size) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_range_pkt_size =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_size_result, pkt_size, "pkt.size");
cmdline_parse_token_string_t cmd_pkt_size_what =
	TOKEN_STRING_INITIALIZER(struct cmd_pkt_size_result, what, "start#min#max#inc");
cmdline_parse_token_portlist_t cmd_pkt_size_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_pkt_size_result, portlist);
cmdline_parse_token_num_t cmd_pkt_size_addr =
	TOKEN_NUM_INITIALIZER(struct cmd_pkt_size_result, size, UINT16);

cmdline_parse_inst_t cmd_pkt_size = {
	.f = cmd_pkt_size_parsed,
	.data = NULL,
	.help_str = " start|min|max|inc <portlist> id",
	.tokens = {
		(void *)&cmd_set_range_pkt_size,
		(void *)&cmd_pkt_size_what,
		(void *)&cmd_pkt_size_portlist,
		(void *)&cmd_pkt_size_addr,
		NULL,
	},
};

/**********************************************************/

struct cmd_set_result {
	cmdline_fixed_string_t set;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t what;
	uint32_t value;
};

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

static void cmd_set_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_set_result *res = parsed_result;

	foreach_port( res->portlist.map, _do(
		if (!strcmp(res->what, "count"))
			pktgen_set_tx_count(info, res->value);
		else if (!strcmp(res->what, "size"))
			pktgen_set_pkt_size(info, res->value);
		else if (!strcmp(res->what, "rate"))
			pktgen_set_tx_rate(info, res->value);
		else if (!strcmp(res->what, "burst"))
			pktgen_set_tx_burst(info, res->value);
		else if (!strcmp(res->what, "tx_cycles"))
			pktgen_set_tx_cycles(info, res->value);
		else if (!strcmp(res->what, "sport"))
			pktgen_set_port_value(info, res->what[0], res->value);
		else if (!strcmp(res->what, "dport"))
			pktgen_set_port_value(info, res->what[0], res->value);
		else if (!strcmp(res->what, "seqCnt"))
			pktgen_set_port_seqCnt(info, res->value);
		else if (!strcmp(res->what, "prime"))
			pktgen_set_port_prime(info, res->value);
		else if (!strcmp(res->what, "dump"))
			pktgen_set_port_dump(info, res->value);
		else if (!strcmp(res->what, "vlanid"))
			pktgen_set_vlanid(info, res->value);
	) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_set =
	TOKEN_STRING_INITIALIZER(struct cmd_set_result, set, "set");
cmdline_parse_token_portlist_t cmd_set_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_set_result, portlist);
cmdline_parse_token_string_t cmd_set_what =
	TOKEN_STRING_INITIALIZER(struct cmd_set_result, what,
				 "count#size#rate#burst#tx_cycles#sport#dport#seqCnt#prime#dump#vlanid");
cmdline_parse_token_num_t cmd_set_value =
	TOKEN_NUM_INITIALIZER(struct cmd_set_result, value, UINT32);

cmdline_parse_inst_t cmd_set = {
	.f = cmd_set_parsed,
	.data = NULL,
	.help_str = "set <portlist> count|size|rate|burst|tx_cycles|sport|dport|seqCnt|prime|dump|vlanid value",
	.tokens = {
		(void *)&cmd_set_set,
		(void *)&cmd_set_portlist,
		(void *)&cmd_set_what,
		(void *)&cmd_set_value,
		NULL,
	},
};

/**********************************************************/

struct cmd_pcap_onoff_result {
	cmdline_fixed_string_t pcap;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t what;
};

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

static void cmd_pcap_onoff_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_pcap_onoff_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_pcap_enable_disable(info, res->what) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_pcap_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_pcap_onoff_result, pcap, "pcap");
cmdline_parse_token_portlist_t cmd_pcap_onoff_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_pcap_onoff_result, portlist);
cmdline_parse_token_string_t cmd_pcap_onoff_what =
	TOKEN_STRING_INITIALIZER(struct cmd_pcap_onoff_result, what, "enable#disable#on#off");

cmdline_parse_inst_t cmd_pcap_onoff = {
	.f = cmd_pcap_onoff_parsed,
	.data = NULL,
	.help_str = "pcap <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_pcap_onoff,
		(void *)&cmd_pcap_onoff_portlist,
		(void *)&cmd_pcap_onoff_what,
		NULL,
	},
};

/**********************************************************/

struct cmd_pcap_filter_result {
	cmdline_fixed_string_t pcap_filter;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t filter_string;
};

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

static void cmd_pcap_filter_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_pcap_filter_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_pcap_filter(info, res->filter_string) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_pcap_filter =
	TOKEN_STRING_INITIALIZER(struct cmd_pcap_filter_result, pcap_filter, "pcap.filter");
cmdline_parse_token_portlist_t cmd_pcap_filter_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_pcap_filter_result, portlist);
cmdline_parse_token_string_t cmd_pcap_filter_string =
	TOKEN_STRING_INITIALIZER(struct cmd_pcap_filter_result, filter_string, NULL);

cmdline_parse_inst_t cmd_pcap_filter = {
	.f = cmd_pcap_filter_parsed,
	.data = NULL,
	.help_str = "pcap.filter <portlist> <filter-string>",
	.tokens = {
		(void *)&cmd_set_pcap_filter,
		(void *)&cmd_pcap_filter_portlist,
		(void *)&cmd_pcap_filter_string,
		NULL,
	},
};

/**********************************************************/

struct cmd_pcap_show_result {
	cmdline_fixed_string_t pcap_show;
};

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

static void cmd_pcap_show_parsed(__attribute__((unused)) void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	if ( pktgen.info[pktgen.portNum].pcap )
		wr_pcap_info(pktgen.info[pktgen.portNum].pcap, pktgen.portNum, 1);
	else
		pktgen_log_error(" ** PCAP file is not loaded on port %d", pktgen.portNum);
}

cmdline_parse_token_string_t cmd_set_pcap_show =
	TOKEN_STRING_INITIALIZER(struct cmd_pcap_show_result, pcap_show, "pcap.show");

cmdline_parse_inst_t cmd_pcap_show = {
	.f = cmd_pcap_show_parsed,
	.data = NULL,
	.help_str = "pcap.show - show the pcap information",
	.tokens = {
		(void *)&cmd_set_pcap_show,
		NULL,
	},
};

/**********************************************************/

struct cmd_pcap_index_result {
	cmdline_fixed_string_t pcap_index;
	uint32_t value;
};

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

static void cmd_pcap_index_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_pcap_index_result *res = parsed_result;
	pcap_info_t	  * pcap = pktgen.info[pktgen.portNum].pcap;
	uint32_t	max_cnt = pcap->pkt_count;

	if ( pcap ) {
		if ( res->value >= max_cnt )
			pcap->pkt_idx = max_cnt - RTE_MIN(PCAP_PAGE_SIZE, (int)max_cnt) ;
		else
			pcap->pkt_idx = res->value;
		pktgen.flags |= PRINT_LABELS_FLAG;
	} else
		pktgen_log_error(" ** PCAP file is not loaded on port %d", pktgen.portNum);
}

cmdline_parse_token_string_t cmd_set_pcap_index =
	TOKEN_STRING_INITIALIZER(struct cmd_pcap_index_result, pcap_index, "pcap.index");
cmdline_parse_token_num_t cmd_set_pcap_value =
	TOKEN_NUM_INITIALIZER(struct cmd_pcap_index_result, value, UINT32);

cmdline_parse_inst_t cmd_pcap_index = {
	.f = cmd_pcap_index_parsed,
	.data = NULL,
	.help_str = "pcap.index - Set the PCAP pakcet index value",
	.tokens = {
		(void *)&cmd_set_pcap_index,
		(void *)&cmd_set_pcap_value,
		NULL,
	},
};

/**********************************************************/

struct cmd_blink_onoff_result {
	cmdline_fixed_string_t blink;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t what;
};

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

static void cmd_blink_onoff_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_blink_onoff_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_blink_enable_disable(info, res->what) );

	if ( pktgen.blinklist )
		pktgen.flags |= BLINK_PORTS_FLAG;
	else
		pktgen.flags &= ~BLINK_PORTS_FLAG;
	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_blink_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_blink_onoff_result, blink, "blink");
cmdline_parse_token_portlist_t cmd_blink_onoff_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_blink_onoff_result, portlist);
cmdline_parse_token_string_t cmd_blink_onoff_what =
	TOKEN_STRING_INITIALIZER(struct cmd_blink_onoff_result, what, "enable#disable#on#off");

cmdline_parse_inst_t cmd_blink_onoff = {
	.f = cmd_blink_onoff_parsed,
	.data = NULL,
	.help_str = "blink <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_blink_onoff,
		(void *)&cmd_blink_onoff_portlist,
		(void *)&cmd_blink_onoff_what,
		NULL,
	},
};

/**********************************************************/

struct cmd_garp_onoff_result {
	cmdline_fixed_string_t garp;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t what;
};

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

static void cmd_garp_onoff_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_garp_onoff_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_garp_enable_disable(info, res->what) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_garp_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_garp_onoff_result, garp, "garp");
cmdline_parse_token_portlist_t cmd_garp_onoff_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_garp_onoff_result, portlist);
cmdline_parse_token_string_t cmd_garp_onoff_what =
	TOKEN_STRING_INITIALIZER(struct cmd_garp_onoff_result, what, "enable#disable#on#off");

cmdline_parse_inst_t cmd_garp_onoff = {
	.f = cmd_garp_onoff_parsed,
	.data = NULL,
	.help_str = "garp <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_garp_onoff,
		(void *)&cmd_garp_onoff_portlist,
		(void *)&cmd_garp_onoff_what,
		NULL,
	},
};

/**********************************************************/

struct cmd_process_onoff_result {
	cmdline_fixed_string_t process;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t what;
};

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

static void cmd_process_onoff_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_process_onoff_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_process_enable_disable(info, res->what) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_process_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_process_onoff_result, process, "process");
cmdline_parse_token_portlist_t cmd_process_onoff_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_process_onoff_result, portlist);
cmdline_parse_token_string_t cmd_process_onoff_what =
	TOKEN_STRING_INITIALIZER(struct cmd_process_onoff_result, what, "enable#disable#on#off");

cmdline_parse_inst_t cmd_process_onoff = {
	.f = cmd_process_onoff_parsed,
	.data = NULL,
	.help_str = "process <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_process_onoff,
		(void *)&cmd_process_onoff_portlist,
		(void *)&cmd_process_onoff_what,
		NULL,
	},
};

/**********************************************************/

struct cmd_set_ppp_result {
	cmdline_fixed_string_t ppp;
	uint32_t value;
};

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

static void cmd_set_ppp_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_set_ppp_result *res = parsed_result;

	pktgen_set_page_size(res->value);
}

cmdline_parse_token_string_t cmd_set_ppp =
	TOKEN_STRING_INITIALIZER(struct cmd_set_ppp_result, ppp, "ppp");
cmdline_parse_token_num_t cmd_set_ppp_value =
	TOKEN_NUM_INITIALIZER(struct cmd_set_ppp_result, value, UINT32);

cmdline_parse_inst_t cmd_set_pppp = {
	.f = cmd_set_ppp_parsed,
	.data = NULL,
	.help_str = "ppp <size>",
	.tokens = {
		(void *)&cmd_set_ppp,
		(void *)&cmd_set_ppp_value,
		NULL,
	},
};

/**********************************************************/

struct cmd_set_port_result {
	cmdline_fixed_string_t port;
	uint32_t value;
};

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

static void cmd_set_port_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_set_port_result *res = parsed_result;

	pktgen_set_port_number(res->value);
}

cmdline_parse_token_string_t cmd_set_port_num =
	TOKEN_STRING_INITIALIZER(struct cmd_set_port_result, port, "port");
cmdline_parse_token_num_t cmd_set_port_value =
	TOKEN_NUM_INITIALIZER(struct cmd_set_port_result, value, UINT32);

cmdline_parse_inst_t cmd_set_port_number = {
	.f = cmd_set_port_parsed,
	.data = NULL,
	.help_str = "port <Number>",
	.tokens = {
		(void *)&cmd_set_port_num,
		(void *)&cmd_set_port_value,
		NULL,
	},
};

/**********************************************************/

struct cmd_set_seq_result {
	cmdline_fixed_string_t seq;
	uint32_t			seqnum;
	cmdline_portlist_t portlist;
	cmdline_etheraddr_t daddr;
	cmdline_etheraddr_t saddr;
	cmdline_ipaddr_t ip_daddr;
	cmdline_ipaddr_t ip_saddr;
	uint32_t			sport;
	uint32_t			dport;
	cmdline_fixed_string_t eth;
	cmdline_fixed_string_t proto;
	uint16_t			vlanid;
	uint32_t			pktsize;
};

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

static void cmd_set_seq_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_set_seq_result *res = parsed_result;

	if ( (res->proto[0] == 'i') && (res->eth[3] == '6') ) {
		cmdline_printf(cl, "Must use IPv4 with ICMP type packets\n");
		return;
	}

	if ( res->seqnum >= NUM_SEQ_PKTS )
		return;

	foreach_port( res->portlist.map,
		pktgen_set_seq(info, res->seqnum,
				&res->daddr, &res->saddr, &res->ip_daddr, &res->ip_saddr,
				res->sport, res->dport, res->eth[3], res->proto[0],
				res->vlanid, res->pktsize) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_set_seq =
	TOKEN_STRING_INITIALIZER(struct cmd_set_seq_result, seq, "seq");
cmdline_parse_token_num_t cmd_set_seqnum =
	TOKEN_NUM_INITIALIZER(struct cmd_set_seq_result, seqnum, UINT32);
cmdline_parse_token_portlist_t cmd_set_seq_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_set_seq_result, portlist);
cmdline_parse_token_etheraddr_t cmd_set_seq_daddr =
	TOKEN_ETHERADDR_INITIALIZER(struct cmd_set_seq_result, daddr);
cmdline_parse_token_etheraddr_t cmd_set_seq_saddr =
	TOKEN_ETHERADDR_INITIALIZER(struct cmd_set_seq_result, saddr);
cmdline_parse_token_ipaddr_t cmd_set_seq_ip_daddr =
	TOKEN_IPV4_INITIALIZER(struct cmd_set_seq_result, ip_daddr);
cmdline_parse_token_ipaddr_t cmd_set_seq_ip_saddr =
	TOKEN_IPV4NET_INITIALIZER(struct cmd_set_seq_result, ip_saddr);
cmdline_parse_token_num_t cmd_set_seq_sport =
	TOKEN_NUM_INITIALIZER(struct cmd_set_seq_result, sport, UINT32);
cmdline_parse_token_num_t cmd_set_seq_dport =
	TOKEN_NUM_INITIALIZER(struct cmd_set_seq_result, dport, UINT32);
cmdline_parse_token_string_t cmd_set_seq_eth =
	TOKEN_STRING_INITIALIZER(struct cmd_set_seq_result, eth, "ipv4#ipv6");
cmdline_parse_token_string_t cmd_set_seq_proto =
	TOKEN_STRING_INITIALIZER(struct cmd_set_seq_result, proto, "udp#tcp#icmp");
cmdline_parse_token_num_t cmd_set_seq_vlanid =
	TOKEN_NUM_INITIALIZER(struct cmd_set_seq_result, vlanid, UINT16);
cmdline_parse_token_num_t cmd_set_seq_pktsize =
	TOKEN_NUM_INITIALIZER(struct cmd_set_seq_result, pktsize, UINT32);

cmdline_parse_inst_t cmd_seq = {
	.f = cmd_set_seq_parsed,
	.data = NULL,
	.help_str = "seq <seqN> <portlist> dst-MAC src-MAC dst-IP src-IP sport dport type proto vlanid size",
	.tokens = {
		(void *)&cmd_set_set_seq,
		(void *)&cmd_set_seqnum,
		(void *)&cmd_set_seq_portlist,
		(void *)&cmd_set_seq_daddr,
		(void *)&cmd_set_seq_saddr,
		(void *)&cmd_set_seq_ip_daddr,
		(void *)&cmd_set_seq_ip_saddr,
		(void *)&cmd_set_seq_sport,
		(void *)&cmd_set_seq_dport,
		(void *)&cmd_set_seq_eth,
		(void *)&cmd_set_seq_proto,
		(void *)&cmd_set_seq_vlanid,
		(void *)&cmd_set_seq_pktsize,
		NULL,
	},
};

/**********************************************************/

struct cmd_setip_dst_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t ip;
	cmdline_fixed_string_t iptype;
	cmdline_portlist_t portlist;
	cmdline_ipaddr_t ipaddr;
};

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

static void cmd_setip_dst_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_setip_dst_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_ipaddr(info, res->iptype[0], &res->ipaddr) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_setip_dst =
	TOKEN_STRING_INITIALIZER(struct cmd_setip_dst_result, set, "set");
cmdline_parse_token_string_t cmd_set_ip_dst =
	TOKEN_STRING_INITIALIZER(struct cmd_setip_dst_result, ip, "ip");
cmdline_parse_token_string_t cmd_set_iptype_dst =
	TOKEN_STRING_INITIALIZER(struct cmd_setip_dst_result, iptype, "dst");
cmdline_parse_token_portlist_t cmd_set_ip_dst_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_setip_dst_result, portlist);
cmdline_parse_token_ipaddr_t cmd_set_dst_ipaddr =
	TOKEN_IPV4_INITIALIZER(struct cmd_setip_dst_result, ipaddr);

cmdline_parse_inst_t cmd_setip_dst = {
	.f = cmd_setip_dst_parsed,
	.data = NULL,
	.help_str = "set ip dst <portlist> ipaddr",
	.tokens = {
		(void *)&cmd_set_setip_dst,
		(void *)&cmd_set_ip_dst,
		(void *)&cmd_set_iptype_dst,
		(void *)&cmd_set_ip_dst_portlist,
		(void *)&cmd_set_dst_ipaddr,
		NULL,
	},
};

/**********************************************************/

struct cmd_setip_src_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t ip;
	cmdline_fixed_string_t iptype;
	cmdline_portlist_t portlist;
	cmdline_ipaddr_t ipaddr;
};

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

static void cmd_setip_src_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_setip_src_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_ipaddr(info, res->iptype[0], &res->ipaddr) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_setip_src =
	TOKEN_STRING_INITIALIZER(struct cmd_setip_src_result, set, "set");
cmdline_parse_token_string_t cmd_set_ip_src =
	TOKEN_STRING_INITIALIZER(struct cmd_setip_src_result, ip, "ip");
cmdline_parse_token_string_t cmd_set_iptype_src =
	TOKEN_STRING_INITIALIZER(struct cmd_setip_src_result, iptype, "src");
cmdline_parse_token_portlist_t cmd_set_ip_src_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_setip_src_result, portlist);
cmdline_parse_token_ipaddr_t cmd_set_src_ipaddr =
	TOKEN_IPV4NET_INITIALIZER(struct cmd_setip_src_result, ipaddr);

cmdline_parse_inst_t cmd_setip_src = {
	.f = cmd_setip_src_parsed,
	.data = NULL,
	.help_str = "set ip src <portlist> ipaddr",
	.tokens = {
		(void *)&cmd_set_setip_src,
		(void *)&cmd_set_ip_src,
		(void *)&cmd_set_iptype_src,
		(void *)&cmd_set_ip_src_portlist,
		(void *)&cmd_set_src_ipaddr,
		NULL,
	},
};

/**********************************************************/

struct cmd_send_arp_result {
	cmdline_fixed_string_t send;
	cmdline_fixed_string_t arp;
	cmdline_fixed_string_t what;
	cmdline_portlist_t portlist;
};

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

static void cmd_send_arp_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_send_arp_result *res = parsed_result;

	if (res->what[0] == 'g') {
		foreach_port( res->portlist.map,
			pktgen_send_arp_requests(info, GRATUITOUS_ARP) );
	} else {
		foreach_port( res->portlist.map,
			pktgen_send_arp_requests(info, 0) );
    }
}

cmdline_parse_token_string_t cmd_set_send_arp =
	TOKEN_STRING_INITIALIZER(struct cmd_send_arp_result, send, "send");
cmdline_parse_token_string_t cmd_set_arp =
	TOKEN_STRING_INITIALIZER(struct cmd_send_arp_result, arp, "arp");
cmdline_parse_token_string_t cmd_set_arp_what =
	TOKEN_STRING_INITIALIZER(struct cmd_send_arp_result, what, "grat#req");
cmdline_parse_token_portlist_t cmd_set_arp_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_send_arp_result, portlist);

cmdline_parse_inst_t cmd_send_arp = {
	.f = cmd_send_arp_parsed,
	.data = NULL,
	.help_str = "send arp grat|req <portlist>",
	.tokens = {
		(void *)&cmd_set_send_arp,
		(void *)&cmd_set_arp,
		(void *)&cmd_set_arp_what,
		(void *)&cmd_set_arp_portlist,
		NULL,
	},
};

/**********************************************************/

struct cmd_set_proto_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t type;
	cmdline_portlist_t portlist;
};

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

static void cmd_set_proto_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_set_proto_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_proto(info, res->type[0]) );

    pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_set_proto =
	TOKEN_STRING_INITIALIZER(struct cmd_set_proto_result, set, "proto");
cmdline_parse_token_string_t cmd_set_type =
	TOKEN_STRING_INITIALIZER(struct cmd_set_proto_result, type, "udp#tcp#icmp");
cmdline_parse_token_portlist_t cmd_set_proto_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_set_proto_result, portlist);

cmdline_parse_inst_t cmd_proto = {
	.f = cmd_set_proto_parsed,
	.data = NULL,
	.help_str = "proto udp|tcp|icmp <portlist>",
	.tokens = {
		(void *)&cmd_set_set_proto,
		(void *)&cmd_set_type,
		(void *)&cmd_set_proto_portlist,
		NULL,
	},
};

/**********************************************************/

struct cmd_load_result {
	cmdline_fixed_string_t load;
	cmdline_fixed_string_t path;
};

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

static void cmd_set_load_parsed(void *parsed_result,
			   struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_load_result *res = parsed_result;

	if ( pktgen_load_cmds(res->path) )
		cmdline_printf(cl, "load command failed for %s\n", res->path);
	if ( ! wr_scrn_is_paused() )
		pktgen_redisplay(0);
}

cmdline_parse_token_string_t cmd_set_load =
	TOKEN_STRING_INITIALIZER(struct cmd_load_result, load, "load");
cmdline_parse_token_string_t cmd_set_path =
	TOKEN_STRING_INITIALIZER(struct cmd_load_result, path, NULL);

cmdline_parse_inst_t cmd_load = {
	.f = cmd_set_load_parsed,
	.data = NULL,
	.help_str = "load <path>",
	.tokens = {
		(void *)&cmd_set_load,
		(void *)&cmd_set_path,
		NULL,
	},
};

/**********************************************************/

struct cmd_page_result {
	cmdline_fixed_string_t page;
	cmdline_fixed_string_t pageType;
};

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

static void cmd_set_page_parsed(void *parsed_result,
		__attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_page_result *res = parsed_result;

	pktgen_set_page(res->pageType);
}

cmdline_parse_token_string_t cmd_set_page =
	TOKEN_STRING_INITIALIZER(struct cmd_page_result, page, "page");
cmdline_parse_token_string_t cmd_set_pageType =
	TOKEN_STRING_INITIALIZER(struct cmd_page_result, pageType, "0#1#2#3#4#5#6#7#main#range#config#sequence#seq#pcap#next#cpu#rnd#log");

cmdline_parse_inst_t cmd_page = {
	.f = cmd_set_page_parsed,
	.data = NULL,
	.help_str = "page [0-7]|main|range|config|sequence|seq|pcap|next|cpu|rnd|log",
	.tokens = {
		(void *)&cmd_set_page,
		(void *)&cmd_set_pageType,
		NULL,
	},
};

/**********************************************************/

struct cmd_screen_result {
	cmdline_fixed_string_t screen;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_screen_parsed(void *parsed_result,
		__attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_screen_result *res = parsed_result;

	pktgen_screen(res->onOff);
}

cmdline_parse_token_string_t cmd_set_screen =
	TOKEN_STRING_INITIALIZER(struct cmd_screen_result, screen, "screen");
cmdline_parse_token_string_t cmd_screen_onOff =
	TOKEN_STRING_INITIALIZER(struct cmd_screen_result, onOff, "stop#start#off#on#enable#disable");

cmdline_parse_inst_t cmd_screen = {
	.f = cmd_screen_parsed,
	.data = NULL,
	.help_str = "screen stop|start|on|off|enable|disable",
	.tokens = {
		(void *)&cmd_set_screen,
		(void *)&cmd_screen_onOff,
		NULL,
	},
};

/**********************************************************/

struct cmd_off_result {
	cmdline_fixed_string_t off;
};

/**************************************************************************//**
*
* cmd_off_parsed - Enable or Disable the screen updates.
*
* DESCRIPTION
* Enable or disable screen updates.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void cmd_off_parsed(__attribute__ ((unused))void *parsed_result,
		__attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{

	pktgen_screen("off");
}

cmdline_parse_token_string_t cmd_set_off =
	TOKEN_STRING_INITIALIZER(struct cmd_off_result, off, "off");

cmdline_parse_inst_t cmd_off = {
	.f = cmd_off_parsed,
	.data = NULL,
	.help_str = "off - disable the screen",
	.tokens = {
		(void *)&cmd_set_off,
		NULL,
	},
};

/**********************************************************/

struct cmd_on_result {
	cmdline_fixed_string_t on;
};

/**************************************************************************//**
*
* cmd_on_parsed - Enable or Disable the screen updates.
*
* DESCRIPTION
* Enable or disable screen updates.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void cmd_on_parsed(__attribute__ ((unused))void *parsed_result,
		__attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{

	pktgen_screen("on");
}

cmdline_parse_token_string_t cmd_set_on =
	TOKEN_STRING_INITIALIZER(struct cmd_on_result, on, "on");

cmdline_parse_inst_t cmd_on = {
	.f = cmd_on_parsed,
	.data = NULL,
	.help_str = "on - enable the screen",
	.tokens = {
		(void *)&cmd_set_on,
		NULL,
	},
};

/**********************************************************/

struct cmd_tx_debug_result {
	cmdline_fixed_string_t tx_debug;
};

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

static void cmd_tx_debug_parsed(__attribute__ ((unused))void *parsed_result,
		__attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{

	if ( (pktgen.flags & TX_DEBUG_FLAG) == 0 )
		pktgen.flags |= TX_DEBUG_FLAG;
	else
		pktgen.flags &= ~TX_DEBUG_FLAG;
	pktgen_cls();
}

cmdline_parse_token_string_t cmd_set_tx_debug =
	TOKEN_STRING_INITIALIZER(struct cmd_tx_debug_result, tx_debug, "tx_debug");

cmdline_parse_inst_t cmd_tx_debug = {
	.f = cmd_tx_debug_parsed,
	.data = NULL,
	.help_str = "Toggle TX debug",
	.tokens = {
		(void *)&cmd_set_tx_debug,
		NULL,
	},
};

/**********************************************************/

struct cmd_l2p_result {
	cmdline_fixed_string_t l2p;
};

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

static void cmd_l2p_parsed(__attribute__ ((unused))void *parsed_result,
		__attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{

	pktgen_l2p_dump();
}

cmdline_parse_token_string_t cmd_set_l2p =
	TOKEN_STRING_INITIALIZER(struct cmd_l2p_result, l2p, "l2p");

cmdline_parse_inst_t cmd_l2p = {
	.f = cmd_l2p_parsed,
	.data = NULL,
	.help_str = "l2p dump the information",
	.tokens = {
		(void *)&cmd_set_l2p,
		NULL,
	},
};

/**********************************************************/

struct cmd_mempool_result {
	cmdline_fixed_string_t mempool;
	cmdline_fixed_string_t dump;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t name;
};

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

static void cmd_mempool_parsed(void *parsed_result,
		__attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_mempool_result *res = parsed_result;

	if ( ! strcmp(res->dump, "dump") )
		foreach_port( res->portlist.map,
				pktgen_mempool_dump(info, res->name) );
}

cmdline_parse_token_string_t cmd_set_mempool =
	TOKEN_STRING_INITIALIZER(struct cmd_mempool_result, mempool, "mempool");
cmdline_parse_token_string_t cmd_mempool_dump =
	TOKEN_STRING_INITIALIZER(struct cmd_mempool_result, dump, "dump");
cmdline_parse_token_portlist_t cmd_mempool_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_mempool_result, portlist);
cmdline_parse_token_string_t cmd_mempool_name =
	TOKEN_STRING_INITIALIZER(struct cmd_mempool_result, name, "rx#tx#range#seq#arp#pcap#all");

cmdline_parse_inst_t cmd_mempool = {
	.f = cmd_mempool_parsed,
	.data = NULL,
	.help_str = "mempool dump <portlist> [all|rx|tx|range|seq|arp|pcap]",
	.tokens = {
		(void *)&cmd_set_mempool,
		(void *)&cmd_mempool_dump,
		(void *)&cmd_mempool_portlist,
		(void *)&cmd_mempool_name,
		NULL,
	},
};

/**********************************************************/

struct cmd_set_pkt_type_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t type;
	cmdline_portlist_t portlist;
};

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

static void cmd_set_pkt_type_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_set_proto_result *res = parsed_result;

	foreach_port( res->portlist.map,
			pktgen_set_pkt_type(info, res->type) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_set_pkt_type =
	TOKEN_STRING_INITIALIZER(struct cmd_set_pkt_type_result, set, "type");
cmdline_parse_token_string_t cmd_set_pkt_type =
	TOKEN_STRING_INITIALIZER(struct cmd_set_pkt_type_result, type, "ipv4#ipv6#arp");
cmdline_parse_token_portlist_t cmd_set_pkt_type_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_set_pkt_type_result, portlist);

cmdline_parse_inst_t cmd_pkt_type = {
	.f = cmd_set_pkt_type_parsed,
	.data = NULL,
	.help_str = "type ipv4|ipv6|arp <portlist>",
	.tokens = {
		(void *)&cmd_set_set_pkt_type,
		(void *)&cmd_set_pkt_type,
		(void *)&cmd_set_pkt_type_portlist,
		NULL,
	},
};

/**********************************************************/

struct cmd_icmp_echo_result {
	cmdline_fixed_string_t icmp_echo;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_icmp_echo_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_icmp_echo_result *res = parsed_result;

	foreach_port(res->portlist.map,
			pktgen_set_icmp_echo(info, parseState(res->onOff)) );
}

cmdline_parse_token_string_t cmd_set_icmp =
	TOKEN_STRING_INITIALIZER(struct cmd_icmp_echo_result, icmp_echo, "icmp.echo");
cmdline_parse_token_portlist_t cmd_set_icmp_echo_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_icmp_echo_result, portlist);
cmdline_parse_token_string_t cmd_set_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_icmp_echo_result, onOff, "on#off#enable#disable");

cmdline_parse_inst_t cmd_icmp_echo = {
	.f = cmd_icmp_echo_parsed,
	.data = NULL,
	.help_str = "icmp.echo <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_icmp,
		(void *)&cmd_set_icmp_echo_portlist,
		(void *)&cmd_set_onoff,
		NULL,
	},
};

/**********************************************************/

struct cmd_capture_result {
	cmdline_fixed_string_t capture;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_capture_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_capture_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_capture(info, parseState(res->onOff)) );
}

cmdline_parse_token_string_t cmd_set_capture =
	TOKEN_STRING_INITIALIZER(struct cmd_capture_result, capture, "capture");
cmdline_parse_token_portlist_t cmd_set_capture_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_capture_result, portlist);
cmdline_parse_token_string_t cmd_set_capture_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_capture_result, onOff, "on#off#enable#disable");

cmdline_parse_inst_t cmd_capture = {
	.f = cmd_capture_parsed,
	.data = NULL,
	.help_str = "capture <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_capture,
		(void *)&cmd_set_capture_portlist,
		(void *)&cmd_set_capture_onoff,
		NULL,
	},
};

/**********************************************************/

struct cmd_rx_tap_result {
	cmdline_fixed_string_t rxtap;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_rx_tap_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_rx_tap_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_rx_tap(info, parseState(res->onOff)) );
}

cmdline_parse_token_string_t cmd_set_rx_tap =
	TOKEN_STRING_INITIALIZER(struct cmd_rx_tap_result, rxtap, "rxtap");
cmdline_parse_token_portlist_t cmd_set_rx_tap_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_rx_tap_result, portlist);
cmdline_parse_token_string_t cmd_set_rx_tap_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_rx_tap_result, onOff, "on#off#enable#disable");

cmdline_parse_inst_t cmd_rx_tap = {
	.f = cmd_rx_tap_parsed,
	.data = NULL,
	.help_str = "rxtap <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_rx_tap,
		(void *)&cmd_set_rx_tap_portlist,
		(void *)&cmd_set_rx_tap_onoff,
		NULL,
	},
};

/**********************************************************/

struct cmd_tx_tap_result {
	cmdline_fixed_string_t txtap;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_tx_tap_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_tx_tap_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_tx_tap(info, parseState(res->onOff)) );
}

cmdline_parse_token_string_t cmd_set_tx_tap =
	TOKEN_STRING_INITIALIZER(struct cmd_tx_tap_result, txtap, "txtap");
cmdline_parse_token_portlist_t cmd_set_tx_tap_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_tx_tap_result, portlist);
cmdline_parse_token_string_t cmd_set_tx_tap_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_tx_tap_result, onOff, "on#off#enable#disable");

cmdline_parse_inst_t cmd_tx_tap = {
	.f = cmd_tx_tap_parsed,
	.data = NULL,
	.help_str = "txtap <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_tx_tap,
		(void *)&cmd_set_tx_tap_portlist,
		(void *)&cmd_set_tx_tap_onoff,
		NULL,
	},
};

/**********************************************************/

struct cmd_vlan_result {
	cmdline_fixed_string_t vlan;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_vlan_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_vlan_result *res = parsed_result;

	foreach_port(res->portlist.map,
			pktgen_set_vlan(info, parseState(res->onOff)) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_vlan =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_result, vlan, "vlan");
cmdline_parse_token_portlist_t cmd_set_vlan_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_vlan_result, portlist);
cmdline_parse_token_string_t cmd_set_vlan_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_vlan_result, onOff, "on#off#enable#disable");

cmdline_parse_inst_t cmd_vlan = {
	.f = cmd_vlan_parsed,
	.data = NULL,
	.help_str = "vlan <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_vlan,
		(void *)&cmd_set_vlan_portlist,
		(void *)&cmd_set_vlan_onoff,
		NULL,
	},
};

/**********************************************************/

struct cmd_vlanid_result {
	cmdline_fixed_string_t vlanid;
	cmdline_portlist_t portlist;
	uint16_t id;
};

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

static void cmd_vlanid_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_vlanid_result *res = parsed_result;

	foreach_port(res->portlist.map,
			pktgen_set_vlanid(info, res->id) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_vlanid =
	TOKEN_STRING_INITIALIZER(struct cmd_vlanid_result, vlanid, "vlanid");
cmdline_parse_token_portlist_t cmd_set_vlanid_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_vlanid_result, portlist);
cmdline_parse_token_num_t cmd_set_vlanid_id =
	TOKEN_NUM_INITIALIZER(struct cmd_vlanid_result, id, UINT16);

cmdline_parse_inst_t cmd_vlanid = {
	.f = cmd_vlanid_parsed,
	.data = NULL,
	.help_str = "vlanid <portlist> id",
	.tokens = {
		(void *)&cmd_set_vlanid,
		(void *)&cmd_set_vlanid_portlist,
		(void *)&cmd_set_vlanid_id,
		NULL,
	},
};

/**********************************************************/

struct cmd_mpls_result {
	cmdline_fixed_string_t mpls;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_mpls_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_mpls_result *res = parsed_result;

	foreach_port(res->portlist.map,
			pktgen_set_mpls(info, parseState(res->onOff)) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_mpls =
	TOKEN_STRING_INITIALIZER(struct cmd_mpls_result, mpls, "mpls");
cmdline_parse_token_portlist_t cmd_set_mpls_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_mpls_result, portlist);
cmdline_parse_token_string_t cmd_set_mpls_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_mpls_result, onOff, "on#off#enable#disable");

cmdline_parse_inst_t cmd_mpls = {
	.f = cmd_mpls_parsed,
	.data = NULL,
	.help_str = "mpls <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_mpls,
		(void *)&cmd_set_mpls_portlist,
		(void *)&cmd_set_mpls_onoff,
		NULL,
	},
};

/**********************************************************/

struct cmd_mpls_entry_result {
	cmdline_fixed_string_t mpls_entry;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t entry;
};

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

static void cmd_mpls_entry_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_mpls_entry_result *res = parsed_result;

	uint32_t entry = strtoul(res->entry, NULL, 16);

	foreach_port(res->portlist.map,
			pktgen_set_mpls_entry(info, entry) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_mpls_entry =
	TOKEN_STRING_INITIALIZER(struct cmd_mpls_entry_result, mpls_entry, "mpls_entry");
cmdline_parse_token_portlist_t cmd_set_mpls_entry_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_mpls_entry_result, portlist);
cmdline_parse_token_string_t cmd_set_mpls_entry_entry =
	TOKEN_STRING_INITIALIZER(struct cmd_mpls_entry_result, entry, NULL);

cmdline_parse_inst_t cmd_mpls_entry = {
	.f = cmd_mpls_entry_parsed,
	.data = NULL,
	.help_str = "mpls_entry <portlist> entry (in hex)",
	.tokens = {
		(void *)&cmd_set_mpls_entry,
		(void *)&cmd_set_mpls_entry_portlist,
		(void *)&cmd_set_mpls_entry_entry,
		NULL,
	},
};

/**********************************************************/

struct cmd_qinq_result {
	cmdline_fixed_string_t qinq;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_qinq_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_qinq_result *res = parsed_result;

	foreach_port(res->portlist.map,
			pktgen_set_qinq(info, parseState(res->onOff)) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_qinq =
	TOKEN_STRING_INITIALIZER(struct cmd_qinq_result, qinq, "qinq");
cmdline_parse_token_portlist_t cmd_set_qinq_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_qinq_result, portlist);
cmdline_parse_token_string_t cmd_set_qinq_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_qinq_result, onOff, "on#off#enable#disable");

cmdline_parse_inst_t cmd_qinq = {
	.f = cmd_qinq_parsed,
	.data = NULL,
	.help_str = "qinq <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_qinq,
		(void *)&cmd_set_qinq_portlist,
		(void *)&cmd_set_qinq_onoff,
		NULL,
	},
};

/**********************************************************/

struct cmd_qinqids_result {
	cmdline_fixed_string_t qinqids;
	cmdline_portlist_t portlist;
	uint16_t outerid;
	uint16_t innerid;
};

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

static void cmd_qinqids_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_qinqids_result *res = parsed_result;

	foreach_port(res->portlist.map,
			pktgen_set_qinqids(info, res->outerid, res->innerid) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_qinqids =
	TOKEN_STRING_INITIALIZER(struct cmd_qinqids_result, qinqids, "qinqids");
cmdline_parse_token_portlist_t cmd_set_qinqids_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_qinqids_result, portlist);
cmdline_parse_token_num_t cmd_set_qinqids_outerid =
	TOKEN_NUM_INITIALIZER(struct cmd_qinqids_result, outerid, UINT16);
cmdline_parse_token_num_t cmd_set_qinqids_innerid =
	TOKEN_NUM_INITIALIZER(struct cmd_qinqids_result, innerid, UINT16);

cmdline_parse_inst_t cmd_qinqids = {
	.f = cmd_qinqids_parsed,
	.data = NULL,
	.help_str = "qinqids <portlist> <outer_id> <inner_id>",
	.tokens = {
		(void *)&cmd_set_qinqids,
		(void *)&cmd_set_qinqids_portlist,
		(void *)&cmd_set_qinqids_outerid,
		(void *)&cmd_set_qinqids_innerid,
		NULL,
	},
};

/**********************************************************/

struct cmd_gre_result {
	cmdline_fixed_string_t gre;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_gre_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_gre_result *res = parsed_result;

	foreach_port(res->portlist.map,
			pktgen_set_gre(info, parseState(res->onOff)) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_gre =
	TOKEN_STRING_INITIALIZER(struct cmd_gre_result, gre, "gre");
cmdline_parse_token_portlist_t cmd_set_gre_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_gre_result, portlist);
cmdline_parse_token_string_t cmd_set_gre_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_gre_result, onOff, "on#off#enable#disable");

cmdline_parse_inst_t cmd_gre = {
	.f = cmd_gre_parsed,
	.data = NULL,
	.help_str = "gre <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_gre,
		(void *)&cmd_set_gre_portlist,
		(void *)&cmd_set_gre_onoff,
		NULL,
	},
};

/**********************************************************/

struct cmd_gre_eth_result {
	cmdline_fixed_string_t gre_eth;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_gre_eth_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_gre_eth_result *res = parsed_result;

	foreach_port(res->portlist.map,
			pktgen_set_gre_eth(info, parseState(res->onOff)) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_gre_eth =
	TOKEN_STRING_INITIALIZER(struct cmd_gre_eth_result, gre_eth, "gre_eth");
cmdline_parse_token_portlist_t cmd_set_gre_eth_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_gre_eth_result, portlist);
cmdline_parse_token_string_t cmd_set_gre_eth_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_gre_eth_result, onOff, "on#off#enable#disable");

cmdline_parse_inst_t cmd_gre_eth = {
	.f = cmd_gre_eth_parsed,
	.data = NULL,
	.help_str = "gre_eth <portlist> <state>",
	.tokens = {
		(void *)&cmd_set_gre_eth,
		(void *)&cmd_set_gre_eth_portlist,
		(void *)&cmd_set_gre_eth_onoff,
		NULL,
	},
};

/**********************************************************/

struct cmd_gre_key_result {
	cmdline_fixed_string_t gre_key_str;
	cmdline_portlist_t portlist;
	uint32_t gre_key;
};

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

static void cmd_gre_key_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_gre_key_result *res = parsed_result;

	foreach_port(res->portlist.map,
			pktgen_set_gre_key(info, res->gre_key) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_gre_key_str =
	TOKEN_STRING_INITIALIZER(struct cmd_gre_key_result, gre_key_str, "gre_key");
cmdline_parse_token_portlist_t cmd_set_gre_key_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_gre_key_result, portlist);
cmdline_parse_token_num_t cmd_set_gre_key_gre_key =
	TOKEN_NUM_INITIALIZER(struct cmd_gre_key_result, gre_key, UINT32);

cmdline_parse_inst_t cmd_gre_key = {
	.f = cmd_gre_key_parsed,
	.data = NULL,
	.help_str = "gre_key <portlist> <GRE key>",
	.tokens = {
		(void *)&cmd_set_gre_key_str,
		(void *)&cmd_set_gre_key_portlist,
		(void *)&cmd_set_gre_key_gre_key,
		NULL,
	},
};

/**********************************************************/

struct cmd_mac_from_arp_result {
	cmdline_fixed_string_t mac_from_arp;
	cmdline_fixed_string_t onOff;
};

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

static void cmd_mac_from_arp_parsed(void *parsed_result,
		__attribute__((unused)) struct cmdline *cl,
		__attribute__((unused)) void *data)
{
	struct cmd_mac_from_arp_result *res = parsed_result;
	uint32_t		onOff = parseState(res->onOff);

	pktgen_mac_from_arp(onOff);
}

cmdline_parse_token_string_t cmd_set_mac_from_arp =
	TOKEN_STRING_INITIALIZER(struct cmd_mac_from_arp_result, mac_from_arp, "mac_from_arp");
cmdline_parse_token_string_t cmd_set_mac_from_arp_onoff =
	TOKEN_STRING_INITIALIZER(struct cmd_mac_from_arp_result, onOff, "on#off#enable#disable");

cmdline_parse_inst_t cmd_mac_from_arp = {
	.f = cmd_mac_from_arp_parsed,
	.data = NULL,
	.help_str = "mac_from_arp <state>",
	.tokens = {
		(void *)&cmd_set_mac_from_arp,
		(void *)&cmd_set_mac_from_arp_onoff,
		NULL,
	},
};

/**********************************************************/

struct cmd_delay_result {
	cmdline_fixed_string_t delay;
	uint32_t value;
};

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

static void cmd_delay_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_delay_result *res = parsed_result;

	wr_delay_ms( res->value );
}

cmdline_parse_token_string_t cmd_set_delay =
	TOKEN_STRING_INITIALIZER(struct cmd_delay_result, delay, "delay");
cmdline_parse_token_num_t cmd_set_msec =
	TOKEN_NUM_INITIALIZER(struct cmd_delay_result, value, UINT32);

cmdline_parse_inst_t cmd_delay = {
	.f = cmd_delay_parsed,
	.data = NULL,
	.help_str = "delay mSec",
	.tokens = {
		(void *)&cmd_set_delay,
		(void *)&cmd_set_msec,
		NULL,
	},
};

/**********************************************************/

struct cmd_sleep_result {
	cmdline_fixed_string_t sleep;
	uint32_t value;
};

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

static void cmd_sleep_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_sleep_result *res = parsed_result;

	wr_sleep(res->value);
}

cmdline_parse_token_string_t cmd_set_sleep =
	TOKEN_STRING_INITIALIZER(struct cmd_sleep_result, sleep, "sleep");
cmdline_parse_token_num_t cmd_set_seconds =
	TOKEN_NUM_INITIALIZER(struct cmd_sleep_result, value, UINT32);

cmdline_parse_inst_t cmd_sleep = {
	.f = cmd_sleep_parsed,
	.data = NULL,
	.help_str = "sleep seconds",
	.tokens = {
		(void *)&cmd_set_sleep,
		(void *)&cmd_set_seconds,
		NULL,
	},
};

/**********************************************************/

struct cmd_setmac_result {
	cmdline_fixed_string_t set;
	cmdline_fixed_string_t mac;
	cmdline_portlist_t portlist;
	cmdline_etheraddr_t addr;
};

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

static void cmd_setmac_parsed(void *parsed_result,
			   __attribute__((unused)) struct cmdline *cl,
			   __attribute__((unused)) void *data)
{
	struct cmd_setmac_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_set_dst_mac(info, &res->addr) );

	pktgen_update_display();
}

cmdline_parse_token_string_t cmd_set_setmac =
	TOKEN_STRING_INITIALIZER(struct cmd_setmac_result, set, "set");
cmdline_parse_token_string_t cmd_set_mac =
	TOKEN_STRING_INITIALIZER(struct cmd_setmac_result, mac, "mac");
cmdline_parse_token_portlist_t cmd_set_mac_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_setmac_result, portlist);
cmdline_parse_token_etheraddr_t cmd_set_addr =
	TOKEN_ETHERADDR_INITIALIZER(struct cmd_setmac_result, addr);

cmdline_parse_inst_t cmd_setmac = {
	.f = cmd_setmac_parsed,
	.data = NULL,
	.help_str = "set mac <portlist> etheraddr",
	.tokens = {
		(void *)&cmd_set_setmac,
		(void *)&cmd_set_mac,
		(void *)&cmd_set_mac_portlist,
		(void *)&cmd_set_addr,
		NULL,
	},
};

/**********************************************************/

struct cmd_start_result {
	cmdline_fixed_string_t start;
	cmdline_portlist_t portlist;
};

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

static void cmd_start_parsed(void *parsed_result,
			    __attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	struct cmd_start_result *res = parsed_result;

	foreach_port( res->portlist.map,
			pktgen_start_transmitting(info) );
}

cmdline_parse_token_string_t cmd_help_start =
	TOKEN_STRING_INITIALIZER(struct cmd_start_result, start, "start");
cmdline_parse_token_portlist_t cmd_start_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_start_result, portlist);

cmdline_parse_inst_t cmd_start = {
	.f = cmd_start_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Start Transmitting on ports",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_start,
		(void *)&cmd_start_portlist,
		NULL,
	},
};

/**********************************************************/

struct cmd_stop_result {
	cmdline_fixed_string_t stop;
	cmdline_portlist_t portlist;
};

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

static void cmd_stop_parsed(void *parsed_result,
			    __attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	struct cmd_stop_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_stop_transmitting(info) );
}

cmdline_parse_token_string_t cmd_help_stop =
	TOKEN_STRING_INITIALIZER(struct cmd_stop_result, stop, "stop");
cmdline_parse_token_portlist_t cmd_stop_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_stop_result, portlist);

cmdline_parse_inst_t cmd_stop = {
	.f = cmd_stop_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Stop Transmitting on ports",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_stop,
		(void *)&cmd_stop_portlist,
		NULL,
	},
};

/**********************************************************/

struct cmd_str_result {
	cmdline_fixed_string_t str;
};

/**************************************************************************//**
*
* cmd_str_parsed - Start sending packets in a given port list.
*
* DESCRIPTION
* Start sending packets on a given port list.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void cmd_str_parsed(__attribute__ ((unused))void *parsed_result,
			    __attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{

	forall_ports( pktgen_start_transmitting(info) );
}

cmdline_parse_token_string_t cmd_help_str =
	TOKEN_STRING_INITIALIZER(struct cmd_str_result, str, "str");

cmdline_parse_inst_t cmd_str = {
	.f = cmd_str_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Start Transmittings on all ports",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_str,
		NULL,
	},
};

/**********************************************************/

struct cmd_stp_result {
	cmdline_fixed_string_t stp;
};

/**************************************************************************//**
*
* cmd_stp_parsed - Stop ports from sending packets on a given port list
*
* DESCRIPTION
* Stop ports from sending packetss on a given port list.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void cmd_stp_parsed(__attribute__ ((unused))void *parsed_result,
			    __attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{

	forall_ports( pktgen_stop_transmitting(info) );
}

cmdline_parse_token_string_t cmd_help_stp =
	TOKEN_STRING_INITIALIZER(struct cmd_stp_result, stp, "stp");

cmdline_parse_inst_t cmd_stp = {
	.f = cmd_stp_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Stop Transmittings on all ports",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_stp,
		NULL,
	},
};

/**********************************************************/

struct cmd_prime_result {
	cmdline_fixed_string_t prime;
	cmdline_portlist_t portlist;
};

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

static void cmd_prime_parsed(void *parsed_result,
			    __attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	struct cmd_set_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_prime_ports(info) );
}

cmdline_parse_token_string_t cmd_help_prime =
	TOKEN_STRING_INITIALIZER(struct cmd_prime_result, prime, "prime");
cmdline_parse_token_portlist_t cmd_prime_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_set_result, portlist);

cmdline_parse_inst_t cmd_prime = {
	.f = cmd_prime_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Prime the L3 cache on DUT",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_prime,
		(void *)&cmd_prime_portlist,
		NULL,
	},
};

/**********************************************************/

struct cmd_clear_result {
	cmdline_fixed_string_t clear;
	cmdline_portlist_t portlist;
};

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

static void cmd_clear_parsed(__attribute__((unused)) void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	struct cmd_clear_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_clear_stats(info) );
}

cmdline_parse_token_string_t cmd_help_clear =
	TOKEN_STRING_INITIALIZER(struct cmd_clear_result, clear, "clear");
cmdline_parse_token_portlist_t cmd_clear_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_clear_result, portlist);

cmdline_parse_inst_t cmd_clear = {
	.f = cmd_clear_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Clear statistics per port",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_clear,
		(void *)&cmd_clear_portlist,
		NULL,
	},
};

/**********************************************************/

struct cmd_clr_result {
	cmdline_fixed_string_t clr;
};

/**************************************************************************//**
*
* cmd_clr_parsed - Clear the statistics on all ports.
*
* DESCRIPTION
* Clear all statistics on all ports.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void cmd_clr_parsed(__attribute__((unused)) void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{

	forall_ports( pktgen_clear_stats(info) );
}

cmdline_parse_token_string_t cmd_help_clr =
	TOKEN_STRING_INITIALIZER(struct cmd_clr_result, clr, "clr");

cmdline_parse_inst_t cmd_clr = {
	.f = cmd_clr_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Clear all port statistics",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_clr,
		NULL,
	},
};

/**********************************************************/

struct cmd_quit_result {
	cmdline_fixed_string_t quit;
};

/**************************************************************************//**
*
* cmd_quit_parsed - quit pktgen.
*
* DESCRIPTION
* Close down and quit Pktgen.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void cmd_quit_parsed(__attribute__((unused)) void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	pktgen_quit();
}

cmdline_parse_token_string_t cmd_help_quit =
	TOKEN_STRING_INITIALIZER(struct cmd_quit_result, quit, "quit");

cmdline_parse_inst_t cmd_quit = {
	.f = cmd_quit_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Quit the PktGen program",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_quit,
		NULL,
	},
};

/**********************************************************/

struct cmd_cls_result {
	cmdline_fixed_string_t cls;
};

/**************************************************************************//**
*
* cmd_cls_parsed - Clear the screen and redisplay the data again.
*
* DESCRIPTION
* Clear the screen and redisplay the data on the screen.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void cmd_cls_parsed(__attribute__((unused)) void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	pktgen_cls();
}

cmdline_parse_token_string_t cmd_help_cls =
	TOKEN_STRING_INITIALIZER(struct cmd_cls_result, cls, "cls");

cmdline_parse_inst_t cmd_cls = {
	.f = cmd_cls_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Clear screen",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_cls,
		NULL,
	},
};

/**********************************************************/

struct cmd_reset_result {
	cmdline_fixed_string_t reset;
	cmdline_portlist_t portlist;
};

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

static void cmd_reset_parsed(__attribute__((unused)) void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{
	struct cmd_reset_result *res = parsed_result;

	foreach_port( res->portlist.map,
		pktgen_reset(info) );
}

cmdline_parse_token_string_t cmd_help_reset =
	TOKEN_STRING_INITIALIZER(struct cmd_reset_result, reset, "reset");
cmdline_parse_token_portlist_t cmd_reset_portlist =
	TOKEN_PORTLIST_INITIALIZER(struct cmd_reset_result, portlist);

cmdline_parse_inst_t cmd_reset = {
	.f = cmd_reset_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Reset configuration to default",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_reset,
		(void *)&cmd_reset_portlist,
		NULL,
	},
};

/**********************************************************/

struct cmd_rst_result {
	cmdline_fixed_string_t rst;
};

/**************************************************************************//**
*
* cmd_rst_parsed - Reset Pktgen to the default configuration state.
*
* DESCRIPTION
* Reset Pktgen to the default configuration state.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void cmd_rst_parsed(__attribute__((unused)) void *parsed_result,
				__attribute__((unused)) struct cmdline *cl,
			    __attribute__((unused)) void *data)
{

	forall_ports( pktgen_reset(info) );
}

cmdline_parse_token_string_t cmd_help_rst =
	TOKEN_STRING_INITIALIZER(struct cmd_rst_result, rst, "rst");

cmdline_parse_inst_t cmd_rst = {
	.f = cmd_rst_parsed,  /* function to call */
	.data = NULL,      /* 2nd arg of func */
	.help_str = "Reset all port configurations to default",
	.tokens = {        /* token list, NULL terminated */
		(void *)&cmd_help_rst,
		NULL,
	},
};

/**********************************************************/

struct cmd_dump_onoff_result {
	cmdline_fixed_string_t dump;
	cmdline_portlist_t portlist;
	cmdline_fixed_string_t what;
};


/**********************************************************/
/**********************************************************/
/****** CONTEXT (list of instruction) */

cmdline_parse_ctx_t main_ctx[] = {
	    (cmdline_parse_inst_t *)&cmd_blink_onoff,
	    (cmdline_parse_inst_t *)&cmd_clear,
	    (cmdline_parse_inst_t *)&cmd_cls,
	    (cmdline_parse_inst_t *)&cmd_delay,
	    (cmdline_parse_inst_t *)&cmd_dest_mac,
	    (cmdline_parse_inst_t *)&cmd_dst_ip,
	    (cmdline_parse_inst_t *)&cmd_dst_port,
	    (cmdline_parse_inst_t *)&cmd_dev,
	    (cmdline_parse_inst_t *)&cmd_geometry,
	    (cmdline_parse_inst_t *)&cmd_help,
	    (cmdline_parse_inst_t *)&cmd_icmp_echo,
	    (cmdline_parse_inst_t *)&cmd_load,
	    (cmdline_parse_inst_t *)&cmd_mac_from_arp,
	    (cmdline_parse_inst_t *)&cmd_mempool,
	    (cmdline_parse_inst_t *)&cmd_page,
	    (cmdline_parse_inst_t *)&cmd_pcap_index,
	    (cmdline_parse_inst_t *)&cmd_pcap_onoff,
	    (cmdline_parse_inst_t *)&cmd_pcap_show,
	    (cmdline_parse_inst_t *)&cmd_pcap_filter,
	    (cmdline_parse_inst_t *)&cmd_pci,
	    (cmdline_parse_inst_t *)&cmd_ping4,
#ifdef INCLUDE_PING6
	    (cmdline_parse_inst_t *)&cmd_ping6,
#endif
	    (cmdline_parse_inst_t *)&cmd_pkt_size,
	    (cmdline_parse_inst_t *)&cmd_pkt_type,
	    (cmdline_parse_inst_t *)&cmd_prime,
	    (cmdline_parse_inst_t *)&cmd_process_onoff,
	    (cmdline_parse_inst_t *)&cmd_garp_onoff,
	    (cmdline_parse_inst_t *)&cmd_proto,
	    (cmdline_parse_inst_t *)&cmd_quit,
	    (cmdline_parse_inst_t *)&cmd_range,
		(cmdline_parse_inst_t *)&cmd_rnd,
	    (cmdline_parse_inst_t *)&cmd_reset,
	    (cmdline_parse_inst_t *)&cmd_save,
	    (cmdline_parse_inst_t *)&cmd_screen,
	    (cmdline_parse_inst_t *)&cmd_script,
	    (cmdline_parse_inst_t *)&cmd_send_arp,
	    (cmdline_parse_inst_t *)&cmd_seq,
	    (cmdline_parse_inst_t *)&cmd_set,
	    (cmdline_parse_inst_t *)&cmd_setip_dst,
	    (cmdline_parse_inst_t *)&cmd_setip_src,
	    (cmdline_parse_inst_t *)&cmd_setmac,
	    (cmdline_parse_inst_t *)&cmd_set_port_number,
	    (cmdline_parse_inst_t *)&cmd_set_pppp,
	    (cmdline_parse_inst_t *)&cmd_sleep,
	    (cmdline_parse_inst_t *)&cmd_src_ip,
	    (cmdline_parse_inst_t *)&cmd_src_mac,
	    (cmdline_parse_inst_t *)&cmd_src_port,
	    (cmdline_parse_inst_t *)&cmd_start,
	    (cmdline_parse_inst_t *)&cmd_stop,
	    (cmdline_parse_inst_t *)&cmd_capture,
	    (cmdline_parse_inst_t *)&cmd_rx_tap,
	    (cmdline_parse_inst_t *)&cmd_tx_tap,
	    (cmdline_parse_inst_t *)&cmd_vlan,
	    (cmdline_parse_inst_t *)&cmd_vlan_id,
	    (cmdline_parse_inst_t *)&cmd_vlanid,
		(cmdline_parse_inst_t *)&cmd_mpls,
		(cmdline_parse_inst_t *)&cmd_mpls_entry,
		(cmdline_parse_inst_t *)&cmd_qinq,
		(cmdline_parse_inst_t *)&cmd_qinqids,
		(cmdline_parse_inst_t *)&cmd_gre,
		(cmdline_parse_inst_t *)&cmd_gre_eth,
		(cmdline_parse_inst_t *)&cmd_gre_key,
	    (cmdline_parse_inst_t *)&cmd_clr,
	    (cmdline_parse_inst_t *)&cmd_on,
	    (cmdline_parse_inst_t *)&cmd_off,
	    (cmdline_parse_inst_t *)&cmd_stp,
	    (cmdline_parse_inst_t *)&cmd_str,
	    (cmdline_parse_inst_t *)&cmd_rst,
	    (cmdline_parse_inst_t *)&cmd_l2p,
	    (cmdline_parse_inst_t *)&cmd_tx_debug,
	    (cmdline_parse_inst_t *)&cmd_theme_state,
	    (cmdline_parse_inst_t *)&cmd_theme_save,
	    (cmdline_parse_inst_t *)&cmd_theme_show,
	    (cmdline_parse_inst_t *)&cmd_theme,
	NULL,
};

void
pktgen_cmdline_start(void)
{
	// Start up the command line, which exits on Control-D
	pktgen.cl = cmdline_stdin_new(main_ctx, "Pktgen > ");
	__set_prompt();

	if ( pktgen.cl && pktgen.cmd_filename ) {
		pktgen_log_info("# *** Executing file (%s)", pktgen.cmd_filename);
		cmdline_in(pktgen.cl, "\r", 1);
		if ( pktgen_load_cmds(pktgen.cmd_filename) == -1 )
			pktgen_log_warning("*** Unable to find file (%s) or invalid call", pktgen.cmd_filename);
		else
			pktgen_log_info("# *** Done.");
		cmdline_in(pktgen.cl, "\r", 1);

		free(pktgen.cmd_filename);
		pktgen.cmd_filename = NULL;
	}

	pktgen_interact(pktgen.cl);

	cmdline_stdin_exit(pktgen.cl);
}


/**************************************************************************//**
*
* pktgen_load_cmds - Load and execute a command file or Lua script file.
*
* DESCRIPTION
* Load and execute a command file or Lua script file.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

int
pktgen_load_cmds( char * filename )
{
    if ( filename == NULL )
        return 0;

    if ( strstr(filename, ".lua") || strstr(filename, ".LUA") ) {
    	if ( pktgen.L == NULL )
    		return -1;

    	// Execute the Lua script file.
    	if ( luaL_dofile(pktgen.L, filename) != 0 ) {
			pktgen_log_error("%s", lua_tostring(pktgen.L,-1));
    		return -1;
    	}
    } else {
        FILE    * fd;
        char    buff[256];

		fd = fopen((const char *)filename, "r");
		if ( fd == NULL )
			return -1;

		// Reset the command line system for the script.
		rdline_reset(&pktgen.cl->rdl);

		// Read and feed the lines to the cmdline parser.
		while(fgets(buff, sizeof(buff), fd) )
			cmdline_in(pktgen.cl, buff, strlen(buff));

		fclose(fd);
    }
    return 0;
}

