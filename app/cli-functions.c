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

#include <rte_version.h>
#include <rte_atomic.h>
#include <rte_devargs.h>
#include <rte_string_fns.h>
#ifndef RTE_LIBRTE_CLI
#include <cli_string_fns.h>
#endif

#include "pktgen.h"

#include <cli.h>
#include <cli_map.h>

#include "copyright_info.h"
#include "pktgen-cmds.h"
#include "pktgen-main.h"
#include "lpktgenlib.h"
#include "pktgen-display.h"
#include "pktgen-random.h"
#include "pktgen-log.h"
#ifndef RTE_LIBRTE_CLI
#include "pg_ether.h"
#endif

static inline uint16_t
valid_pkt_size(char *val)
{
	uint16_t pkt_size;

	if (!val)
		return (MIN_PKT_SIZE + FCS_SIZE);

	pkt_size = atoi(val);
	if (pkt_size < (MIN_PKT_SIZE + FCS_SIZE))
		pkt_size = (MIN_PKT_SIZE + FCS_SIZE);

	if (pkt_size > (MAX_PKT_SIZE + FCS_SIZE))
		pkt_size = MAX_PKT_SIZE + FCS_SIZE;

	return pkt_size;
}

/**********************************************************/
static const char *title_help[] = {
	"   *** Pktgen Help information ***",
	"",
	NULL,
};

static inline int
command_error(const char * msg, const char *group, int argc, char **argv)
{
	int n;

	if (group)
		cli_help_show_group(group);
	if (msg)
		cli_printf("%s:\n", msg);
	cli_printf("  Invalid line: <");
	for(n = 0; n < argc; n++)
		cli_printf("%s ", argv[n]);
	cli_printf(">\n");
	return -1;
}

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
	CLI_HELP_PAUSE,
	NULL
};

#define SMMI	"%|start|minimum|maximum|increment|min|max|inc"
static struct cli_map range_map[] = {
	{ 20, "range %P dst mac "SMMI" %m" },
	{ 21, "range %P src mac "SMMI" %m" },
	{ 22, "range %P dst mac %m %m %m %m" },
	{ 23, "range %P src mac %m %m %m %m" },
	{ 30, "range %P dst ip "SMMI" %4" },
	{ 31, "range %P src ip "SMMI" %4" },
	{ 32, "range %P dst ip %4 %4 %4 %4" },
	{ 33, "range %P src ip %4 %4 %4 %4" },
	{ 40, "range %P proto tcp|udp" },
	{ 50, "range %P dst port "SMMI" %d" },
	{ 51, "range %P src port "SMMI" %d" },
	{ 52, "range %P dst port %d %d %d %d" },
	{ 53, "range %P src port %d %d %d %d" },
	{ 60, "range %P vlan "SMMI" %d" },
	{ 61, "range %P vlan %d %d %d %d" },
	{ 70, "range %P size "SMMI" %d" },
	{ 71, "range %P size %d %d %d %d" },
	{ 80, "range %P mpls entry %x" },
	{ 85, "range %P qinq index %d %d" },
	{ 90, "range %P gre key %d" },
    { -1, NULL }
};

static const char *range_help[] = {
	"  -- Setup the packet range values --",
	"   note: SMMI = start|min|max|inc (start, minimum, maximum, increment)",
	"range <portlist> src|dst mac <SMMI> <etheraddr> - Set destination/source MAC address",
	"            e.g: range 0 src mac start 00:00:00:00:00:00",
	"                 range 0 dst mac max 00:12:34:56:78:90",
	"             or  range 0 src mac 00:00:00:00:00:00 00:00:00:00:00:00 00:12:34:56:78:90 00:00:00:01:01:01",
	"range <portlist> src|dst ip <SMMI> <ipaddr>   - Set source IP start address",
	"            e.g: range 0 dst ip start 0.0.0.0",
	"                 range 0 dst ip min 0.0.0.0",
	"                 range 0 dst ip max 1.2.3.4",
	"                 range 0 dst ip inc 0.0.1.0",
    "             or  range 0 dst ip 0.0.0.0 0.0.0.0 1.2.3.4 0.0.1.0",
	"range <portlist> proto tcp|udp                - Set the IP protocol type",
	"range <portlist> src|dst port <SMMI> <value>  - Set UDP/TCP source/dest port number",
	"   or  range <portlist> src|dst port <start> <min> <max> <inc>",
	"range <portlist> vlan <SMMI> <value>          - Set vlan id start address",
	"   or  range <portlist> vlan <start> <min> <max> <inc>",
	"range <portlist> size <SMMI> <value>          - Set pkt size start address",
	"   or  range <portlist> size <start> <min> <max> <inc>",
	"range <portlist> teid <SMMI> <value>          - Set TEID value",
	"   or  range <portlist> teid <start> <min> <max> <inc>",
	"range <portlist> mpls entry <hex-value>       - Set MPLS entry value",
	"range <portlist> qinq index <val1> <val2>     - Set QinQ index values",
	"range <portlist> gre key <value>              - Set GRE key value",
	CLI_HELP_PAUSE,
	NULL
};

static int
range_cmd(int argc, char **argv)
{
	struct cli_map *m;
	portlist_t portlist;
	struct pg_ipaddr ip;
	char *what, *p;
	const char *val;

	m = cli_mapping(range_map, argc, argv);
	if (!m)
		return command_error("Range command error", "Range", argc, argv);

	rte_parse_portlist(argv[1], &portlist);

	what = argv[4];
	val = (const char*)argv[5];
	switch(m->index) {
		case 20:
			foreach_port(portlist,
			     range_set_dest_mac(info, what, rte_ether_aton(val, NULL)));
			break;
		case 21:
			foreach_port(portlist,
			     range_set_src_mac(info, what, rte_ether_aton(val, NULL)));
			break;
		case 22:
			foreach_port(portlist,
			     range_set_dest_mac(info, "start", rte_ether_aton((const char *)argv[4], NULL));
			     range_set_dest_mac(info, "min", rte_ether_aton((const char *)argv[5], NULL));
			     range_set_dest_mac(info, "max", rte_ether_aton((const char *)argv[6], NULL));
			     range_set_dest_mac(info, "inc", rte_ether_aton((const char *)argv[7], NULL))
				);
			break;
		case 23:
			foreach_port(portlist,
			     range_set_src_mac(info, "start", rte_ether_aton((const char *)argv[4], NULL));
			     range_set_src_mac(info, "min", rte_ether_aton((const char *)argv[5], NULL));
			     range_set_src_mac(info, "max", rte_ether_aton((const char *)argv[6], NULL));
			     range_set_src_mac(info, "inc", rte_ether_aton((const char *)argv[7], NULL))
				);
			break;
		case 30:
			/* Remove the /XX mask value is supplied */
			p = strchr(argv[4], '/');
			if (p)
				*p = '\0';
			rte_atoip(val, PG_IPADDR_V4, &ip, sizeof(ip));
			foreach_port(portlist,
			     range_set_dst_ip(info, what, &ip));
			break;
		case 31:
			/* Remove the /XX mask value is supplied */
			p = strchr(argv[4], '/');
			if (p)
				*p = '\0';
			rte_atoip(argv[5], PG_IPADDR_V4, &ip, sizeof(ip));
			foreach_port(portlist,
			     range_set_src_ip(info, what, &ip));
			break;
		case 32:
			foreach_port(portlist,
				rte_atoip(argv[4], PG_IPADDR_V4, &ip, sizeof(ip));
			    range_set_dst_ip(info, (char *)(uintptr_t)"start", &ip);
				rte_atoip(argv[5], PG_IPADDR_V4, &ip, sizeof(ip));
			    range_set_dst_ip(info, (char *)(uintptr_t)"min", &ip);
				rte_atoip(argv[6], PG_IPADDR_V4, &ip, sizeof(ip));
			    range_set_dst_ip(info, (char *)(uintptr_t)"max", &ip);
				rte_atoip(argv[7], PG_IPADDR_V4, &ip, sizeof(ip));
			    range_set_dst_ip(info, (char *)(uintptr_t)"inc", &ip)
				);
			break;
		case 33:
			foreach_port(portlist,
				rte_atoip(argv[4], PG_IPADDR_V4, &ip, sizeof(ip));
			    range_set_src_ip(info, (char *)(uintptr_t)"start", &ip);
				rte_atoip(argv[5], PG_IPADDR_V4, &ip, sizeof(ip));
			    range_set_src_ip(info, (char *)(uintptr_t)"min", &ip);
				rte_atoip(argv[6], PG_IPADDR_V4, &ip, sizeof(ip));
			    range_set_src_ip(info, (char *)(uintptr_t)"max", &ip);
				rte_atoip(argv[7], PG_IPADDR_V4, &ip, sizeof(ip));
			    range_set_src_ip(info, (char *)(uintptr_t)"inc", &ip)
				);
			break;
		case 40:
			foreach_port(portlist,
				range_set_proto(info, what) );
			break;
		case 50:
			foreach_port(portlist,
				range_set_dst_port(info, what, atoi(val)) );
			break;
		case 51:
			foreach_port(portlist,
				range_set_src_port(info, what, atoi(val)) );
			break;
		case 52:
			foreach_port(portlist,
					range_set_dst_port(info, (char *)(uintptr_t)"start", atoi(argv[4]));
					range_set_dst_port(info, (char *)(uintptr_t)"min", atoi(argv[5]));
					range_set_dst_port(info, (char *)(uintptr_t)"max", atoi(argv[6]));
					range_set_dst_port(info, (char *)(uintptr_t)"inc", atoi(argv[7]))
					);
			break;
		case 53:
			foreach_port(portlist,
					range_set_src_port(info, (char *)(uintptr_t)"start", atoi(argv[4]));
					range_set_src_port(info, (char *)(uintptr_t)"min", atoi(argv[5]));
					range_set_src_port(info, (char *)(uintptr_t)"max", atoi(argv[6]));
					range_set_src_port(info, (char *)(uintptr_t)"inc", atoi(argv[7]))
					);
			break;
		case 60:
			foreach_port(portlist,
				range_set_vlan_id(info, argv[3], atoi(what)) );
			break;
		case 61:
			foreach_port(portlist,
				range_set_vlan_id(info, (char *)(uintptr_t)"start", atoi(argv[3]));
				range_set_vlan_id(info, (char *)(uintptr_t)"min", atoi(argv[4]));
				range_set_vlan_id(info, (char *)(uintptr_t)"max", atoi(argv[5]));
				range_set_vlan_id(info, (char *)(uintptr_t)"inc", atoi(argv[6]))
				);
			break;
		case 70:
			foreach_port(portlist,
				range_set_pkt_size(info, argv[3], valid_pkt_size(what)));
			break;
		case 71:
			foreach_port(portlist,
				range_set_pkt_size(info, (char *)(uintptr_t)"start", valid_pkt_size(argv[3]));
				range_set_pkt_size(info, (char *)(uintptr_t)"min", valid_pkt_size(argv[4]));
				range_set_pkt_size(info, (char *)(uintptr_t)"max", valid_pkt_size(argv[5]));
				range_set_pkt_size(info, (char *)(uintptr_t)"inc", valid_pkt_size(argv[6]));
				);
			break;
		case 80:
			foreach_port(portlist,
				range_set_mpls_entry(info, strtoul(what, NULL, 16)) );
			break;
		case 85:
			foreach_port(portlist,
				range_set_qinqids(info, atoi(what), atoi(val)) );
			break;
		case 90:
			foreach_port(portlist,
				range_set_gre_key(info, strtoul(what, NULL, 10)) );
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
					"seq_cnt|"		/*  7 */ \
					"prime|"		/*  8 */ \
					"dump|"			/*  9 */ \
					"vlan|"			/* 10 */ \
					"seqCnt"		/* 11 */

static struct cli_map set_map[] = {
	{ 10, "set %P %|" set_types " %d" },
	{ 11, "set %P jitter %D" },
	{ 20, "set %P type %|arp|ipv4|ipv6|ip4|ip6|vlan" },
	{ 21, "set %P proto %|udp|tcp|icmp" },
	{ 22, "set %P src mac %m" },
	{ 23, "set %P dst mac %m" },
	{ 24, "set %P pattern %|abc|none|user|zero" },
	{ 25, "set %P user pattern %s" },
	{ 30, "set %P src ip %4" },
	{ 31, "set %P dst ip %4" },
	{ 40, "set ports_per_page %d" },
	{ 50, "set %P qinqids %d %d" },
	{ 60, "set %P rnd %d %d %s" },
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
	"                 seq_cnt           - Set the number of packet in the sequence to send",
	"                 dump              - Dump the next <value> received packets to the screen",
	"                 vlan              - Set the VLAN ID value for the portlist",
	"                 jitter            - Set the jitter threshold in micro-seconds",
	"                 mpls entry        - Set the MPLS entry for the portlist (must be specified in hex)",
	"                 gre_key           - Set the GRE key",
	"                 src|dst mac <etheraddr> - Set MAC addresses 00:11:22:33:44:55",
	"                                     You can use 0011:2233:4455 format as well",
	"set <portlist> jitter <value>      - Set the jitter value",
	"set <portlist> type ipv4|ipv6|vlan|arp - Set the packet type to IPv4 or IPv6 or VLAN",
	"set <portlist> proto udp|tcp|icmp  - Set the packet protocol to UDP or TCP or ICMP per port",
	"set <portlist> pattern <type>      - Set the fill pattern type",
	"     type - abc                    - Default pattern of abc string",
	"            none                   - No fill pattern, maybe random data",
	"            zero                   - Fill of zero bytes",
	"            user                   - User supplied string of max 16 bytes",
	"set <portlist> user pattern <string> - A 16 byte string, must set 'pattern user' command",
	"set <portlist> [src|dst] ip ipaddr - Set IP addresses, Source must include network mask e.g. 10.1.2.3/24",
	"set ports_per_page <value>         - Set ports per page value 1 - 6",
	"set <portlist> qinqids <id1> <id2> - Set the Q-in-Q ID's for the portlist",
	"set <portlist> rnd <idx> <off> <mask> - Set random mask for all transmitted packets from portlist",
	"                                     idx: random mask slot",
	"                                     off: offset in packets, where to apply mask",
	"                                     mask: up to 32 bit long mask specification (empty to disable):",
	"                                       0: bit will be 0",
	"                                       1: bit will be 1",
	"                                       .: bit will be ignored (original value is retained)",
	"                                       X: bit will get random value",
	CLI_HELP_PAUSE,
	NULL
};

static int
set_cmd(int argc, char **argv)
{
	portlist_t portlist;
	char *what, *p;
	int value, n;
	struct cli_map *m;
	struct pg_ipaddr ip;
	uint16_t id1, id2;

	m = cli_mapping(set_map, argc, argv);
	if (!m)
		return command_error("Set command is invalid", "Set", argc, argv);

	rte_parse_portlist(argv[1], &portlist);

	what = argv[2];
	value = atoi(argv[3]);

	switch(m->index) {
		case 10:
			n = cli_map_list_search(m->fmt, argv[2], 2);
			foreach_port(portlist, _do(
				switch(n) {
					case 0: single_set_tx_count(info, value); break;
					case 1: single_set_pkt_size(info, valid_pkt_size(argv[3])); break;
					case 2: single_set_tx_rate(info, argv[3]); break;
					case 3: single_set_tx_burst(info, value); break;
					case 4: debug_set_tx_cycles(info, value); break;
					case 5: single_set_port_value(info, what[0], value); break;
					case 6: single_set_port_value(info, what[0], value); break;
					case 7: pktgen_set_port_seqCnt(info, value); break;
					case 8: pktgen_set_port_prime(info, value); break;
					case 9: debug_set_port_dump(info, value); break;
					case 10: single_set_vlan_id(info, value); break;
					case 11: pktgen_set_port_seqCnt(info, value); break;
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
			foreach_port(portlist,
				single_set_src_mac(info, rte_ether_aton(argv[4], NULL)));
			break;
		case 23:
			foreach_port(portlist,
				single_set_dst_mac(info, rte_ether_aton(argv[4], NULL)));
			break;
		case 24:
			foreach_port(portlist, pattern_set_type(info, argv[3]));
			break;
		case 25:
			foreach_port(portlist,
				 pattern_set_user_pattern(info, argv[3]));
			break;
		case 30:
			p = strchr(argv[4], '/');
			if (!p) {
				char buf[32];
				snprintf(buf, sizeof(buf), "%s/32", argv[4]);
				cli_printf("src IP address should contain /NN subnet value, default /32\n");
				rte_atoip(buf, PG_IPADDR_V4 | PG_IPADDR_NETWORK, &ip, sizeof(ip));
			} else
				rte_atoip(argv[4], PG_IPADDR_V4 | PG_IPADDR_NETWORK, &ip, sizeof(ip));
			foreach_port(portlist, single_set_ipaddr(info, 's', &ip));
			break;
		case 31:
			/* Remove the /XX mask value if supplied */
			p = strchr(argv[4], '/');
			if (p) {
				cli_printf("Subnet mask not required, removing subnet mask value\n");
				*p = '\0';
			}
			rte_atoip(argv[4], PG_IPADDR_V4, &ip, sizeof(ip));
			foreach_port(portlist, single_set_ipaddr(info, 'd', &ip));
			break;
		case 40:
			pktgen_set_page_size(atoi(argv[2]));
			break;
		case 50:
			id1 = strtol(argv[3], NULL, 0);
			id2 = strtol(argv[4], NULL, 0);
			foreach_port(portlist, single_set_qinqids(info, id1, id2));
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

static int
pcap_cmd(int argc, char **argv)
{
	struct cli_map *m;
	pcap_info_t   *pcap;
	uint32_t max_cnt;
	uint32_t value;
	portlist_t portlist;

	m = cli_mapping(pcap_map, argc, argv);
	if (!m)
		return command_error("PCAP command invalid", "PCAP", argc, argv);

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
	CLI_HELP_PAUSE,
	NULL
};

static int
start_stop_cmd(int argc, char **argv)
{
	struct cli_map *m;
	portlist_t portlist;

	m = cli_mapping(start_map, argc, argv);
	if (!m)
		return command_error("Start/Stop command invalid", "Start", argc, argv);

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
	{ 20, "theme %s %s %s %s" },
	{ 30, "theme save" },
	{ -1, NULL }
};

static const char *theme_help[] = {
	"theme <item> <fg> <bg> <attr>      - Set color for item with fg/bg color and attribute value",
	"theme show                         - List the item strings, colors and attributes to the items",
	"theme save <filename>              - Save the current color theme to a file",
	CLI_HELP_PAUSE,
	NULL
};

static int
theme_cmd(int argc, char **argv)
{
	struct cli_map *m;

	m = cli_mapping(theme_map, argc, argv);
	if (!m)
		return command_error("Theme command invalid", "Theme", argc, argv);

	switch(m->index) {
	case 0:  pktgen_theme_show(); break;
	case 10: pktgen_theme_state(argv[1]); pktgen_clear_display(); break;
	case 20: pktgen_set_theme_item(argv[1], argv[2], argv[3], argv[4]); break;
	case 30: pktgen_theme_save(argv[2]); break;
	default: return cli_help_show_group("Theme");
	}
	return 0;
}

#define ed_type	"process|"		/*  0 */	\
				"mpls|" 		/*  1 */	\
				"qinq|" 		/*  2 */	\
				"gre|" 			/*  3 */	\
				"gre_eth|"		/*  4 */	\
				"vlan|" 		/*  5 */	\
				"garp|" 		/*  6 */	\
				"random|" 		/*  7 */	\
				"latency|" 		/*  8 */	\
				"pcap|" 		/*  9 */	\
				"blink|" 		/* 10 */	\
				"rx_tap|" 		/* 11 */	\
				"tx_tap|"		/* 12 */	\
				"icmp|"			/* 13 */	\
				"range|"		/* 14 */	\
				"capture"		/* 15 */

static struct cli_map enable_map[] = {
	{ 10, "enable %P %|" ed_type },
	{ 20, "disable %P %|" ed_type },
	{ 30, "enable %|screen|mac_from_arp" },
	{ 31, "disable %|screen|mac_from_arp"},
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
	"              blink                - Blink LED on port(s)",
	"              rx_tap               - Enable/Disable RX Tap support",
	"              tx_tap               - Enable/Disable TX Tap support",
	"              icmp                 - Enable/Disable sending ICMP packets",
	"              range                - Enable or Disable the given portlist for sending a range of packets",
	"              capture              - Enable/disable packet capturing on a portlist",
	"",
	"enable|disable screen              - Enable/disable updating the screen and unlock/lock window",
	"               mac_from_arp        - Enable/disable MAC address from ARP packet",
	"off                                - screen off shortcut",
	"on                                 - screen on shortcut",
	CLI_HELP_PAUSE,
	NULL
};

static int
en_dis_cmd(int argc, char **argv)
{
	struct cli_map *m;
	portlist_t portlist;
	int n, state;

	m = cli_mapping(enable_map, argc, argv);
	if (!m)
		return command_error("Enable/Disable invalid command", "Enable", argc, argv);

	rte_parse_portlist(argv[1], &portlist);

	switch (m->index) {
		case 10:
		case 20:
			n = cli_map_list_search(m->fmt, argv[2], 2);

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
					foreach_port(portlist, enable_gre_eth(info, state));
					break;
				case 5:
					foreach_port(portlist, enable_vlan(info, state) );
					break;
				case 6:
					foreach_port(portlist, enable_garp(info, state) );
					break;
				case 7:
					foreach_port(portlist, enable_random(info, state) );
					break;
				case 8:
					foreach_port(portlist, enable_latency(info, state) );
					break;
				case 9:
					foreach_port(portlist, enable_pcap(info, state) );
					break;
				case 10:
					foreach_port(portlist, debug_blink(info, state));

					if (pktgen.blinklist)
						pktgen.flags |= BLINK_PORTS_FLAG;
					else
						pktgen.flags &= ~BLINK_PORTS_FLAG;
					break;
				case 11:
					foreach_port(portlist, enable_rx_tap(info, state));
					break;
				case 12:
					foreach_port(portlist, enable_tx_tap(info, state));
					break;
				case 13:
					foreach_port(portlist, enable_icmp_echo(info, state));
					break;
				case 14:
					foreach_port(portlist, enable_range(info, state));
					break;
				case 15:
					foreach_port(portlist, pktgen_set_capture(info, state));
					break;
				default:
					cli_printf("Invalid option %s\n", ed_type);
					return -1;
			}
			break;

		case 30:
		case 31:
			state = estate(argv[0]);

			if (argv[1][0] == 'm')
				enable_mac_from_arp(state);
			else
				pktgen_screen(state);
			break;
		default:
			cli_usage();
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

static int
debug_cmd(int argc, char **argv)
{
	struct cli_map *m;
	portlist_t portlist;

	m = cli_mapping(debug_map, argc, argv);
	if (!m)
		return command_error("Debug invalid command", "Debug", argc, argv);

	switch(m->index) {
		case 10:
			pktgen_l2p_dump();
			break;
		case 20:
			if ( (pktgen.flags & TX_DEBUG_FLAG) == 0)
				pktgen.flags |= TX_DEBUG_FLAG;
			else
				pktgen.flags &= ~TX_DEBUG_FLAG;
			pktgen_clear_display();
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
	char *proto = argv[10], *p;
	char *eth = argv[9];
	int seqnum = atoi(argv[1]);
	portlist_t portlist;
	struct pg_ipaddr dst, src;
	struct ether_addr dmac, smac;
	uint32_t teid;

	if ( (proto[0] == 'i') && (eth[3] == '6') ) {
		cli_printf("Must use IPv4 with ICMP type packets\n");
		return -1;
	}

	if (seqnum >= NUM_SEQ_PKTS)
		return -1;

	teid = (argc == 11)? strtoul(argv[13], NULL, 10) : 0;
	p = strchr(argv[5], '/'); /* remove subnet if found */
	if (p)
		*p = '\0';
	rte_atoip(argv[5], PG_IPADDR_V4, &dst, sizeof(dst));
	p = strchr(argv[6], '/');
	if (!p) {
		char buf[32];
		cli_printf("src IP address should contain /NN subnet value, default /32\n");
		snprintf(buf, sizeof(buf), "%s/32", argv[6]);
		rte_atoip(buf, PG_IPADDR_V4 | PG_IPADDR_NETWORK, &src, sizeof(src));
	} else
		rte_atoip(argv[6], PG_IPADDR_V4 | PG_IPADDR_NETWORK, &src, sizeof(src));
	rte_parse_portlist(argv[2], &portlist);
    rte_ether_aton(argv[3], &dmac);
	rte_ether_aton(argv[4], &smac);
	foreach_port(portlist,
		     pktgen_set_seq(info, seqnum,
				    &dmac, &smac,
				    &dst, &src,
				    atoi(argv[7]), atoi(argv[8]),
					eth[3], proto[0],
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
	char *proto = argv[16], *p;
	char *eth = argv[15];
	int seqnum = atoi(argv[1]);
	portlist_t portlist;
	struct pg_ipaddr dst, src;
	struct ether_addr dmac, smac;
	uint32_t teid;

	if ( (proto[0] == 'i') && (eth[3] == '6') ) {
		cli_printf("Must use IPv4 with ICMP type packets\n");
		return -1;
	}

	if (seqnum >= NUM_SEQ_PKTS)
		return -1;

	teid = (argc == 23)? strtoul(argv[22], NULL, 10) : 0;
	p = strchr(argv[8], '/'); /* remove subnet if found */
	if (p)
		*p = '\0';
	rte_atoip(argv[8], PG_IPADDR_V4, &dst, sizeof(dst));
	p = strchr(argv[10], '/');
	if (p == NULL) {
		char buf[32];
		snprintf(buf, sizeof(buf), "%s/32", argv[10]);
		cli_printf("src IP address should contain /NN subnet value, default /32");
		rte_atoip(buf, PG_IPADDR_V4 | PG_IPADDR_NETWORK, &src, sizeof(src));
	} else
		rte_atoip(argv[10], PG_IPADDR_V4 | PG_IPADDR_NETWORK, &src, sizeof(src));
	rte_parse_portlist(argv[2], &portlist);
    rte_ether_aton(argv[3], &dmac);
	rte_ether_aton(argv[4], &smac);
	foreach_port(portlist,
		     pktgen_set_seq(info, seqnum,
				    &dmac, &smac,
				    &dst, &src,
				    atoi(argv[12]), atoi(argv[14]), eth[3],
				    proto[0],
				    atoi(argv[18]), atoi(argv[20]),
				    teid) );

	pktgen_update_display();
	return 0;
}

static struct cli_map seq_map[] = {
	{ 10, "%|seq|sequence %d %P %m %m %4 %4 %d %d %|ipv4|ipv6 %|udp|tcp|icmp %d %d" },
	{ 11, "%|seq|sequence %d %P %m %m %4 %4 %d %d %|ipv4|ipv6 %|udp|tcp|icmp %d %d %d" },
	{ 12, "%|seq|sequence %d %P dst %m src %m dst %4 src %4 sport %d dport %d %|ipv4|ipv6 %|udp|tcp|icmp vlan %d size %d" },
	{ 13, "%|seq|sequence %d %P dst %m src %m dst %4 src %4 sport %d dport %d %|ipv4|ipv6 %|udp|tcp|icmp vlan %d size %d teid %d" },
	{ -1, NULL }
};

static const char *seq_help[] = {
	"sequence <seq#> <portlist> dst <Mac> src <Mac> dst <IP> src <IP> sport <val> dport <val> ipv4|ipv6 udp|tcp|icmp vlan <val> pktsize <val> [teid <val>]",
	"sequence <seq#> <portlist> <dst-Mac> <src-Mac> <dst-IP> <src-IP> <sport> <dport> ipv4|ipv6 udp|tcp|icmp <vlanid> <pktsize> [<teid>]",
	"                                   - Set the sequence packet information, make sure the src-IP",
	"                                     has the netmask value eg 1.2.3.4/24",
	"",
	NULL
};

static int
seq_cmd(int argc, char **argv)
{
	struct cli_map *m;

	m = cli_mapping(seq_map, argc, argv);
	if (!m)
		return command_error("Sequence invalid command", "Seq", argc, argv);

	switch(m->index) {
		case 10:
		case 11: seq_1_set_cmd(argc, argv); break;
		case 12:
		case 13: seq_2_set_cmd(argc, argv); break;
		default:
			return -1;
	}
	return 0;
}

/**************************************************************************//**
 *
 * script_cmd - Command to execute a script.
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

	if (is_help(argc, argv)) {
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
	char buff[512], *p;
	int i;
	size_t n, sz;

	if (L == NULL) {
		pktgen_log_error("Lua is not initialized!");
		return -1;
	}

	if (is_help(argc, argv)) {
		cli_printf("\nUsage: %s <script-string>\n", argv[0]);
		return 0;
	}

	memset(buff, 0, sizeof(buff));
	sz = sizeof(buff) - 1;
	n = 0;
	for(i = 1, p = buff; i < argc; i++) {
		if ((strlen(argv[i]) + 1) > (sz - n)) {
			cli_printf("Input line too long > 512 bytes\n");
			return -1;
		}
		n += snprintf(&p[n], sz - n, "%s ", argv[i]);
	}

	if (luaL_dostring(L, buff) != 0)
		pktgen_log_error("%s", lua_tostring(L, -1));
	return 0;
}

static struct cli_map misc_map[] = {
	{ 10, "clear %P stats" },
	{ 20, "geometry %s" },
	{ 21, "geometry" },
	{ 30, "load %s" },
	{ 40, "script %l" },
	{ 50, "lua %l" },
	{ 60, "save %s" },
	{ 70, "redisplay" },
	{ 100, "reset %P" },
	{ 110, "restart" },
	{ 130, "port %d" },
	{ 135, "ports per page %d" },
	{ 140, "ping4 %P" },
#ifdef INCLUDE_PING6
	{ 141, "ping6 %P" },
#endif
	{ -1, NULL }
};

static const char *misc_help[] = {
	"save <path-to-file>                - Save a configuration file using the filename",
	"load <path-to-file>                - Load a command/script file from the given path",
	"script <filename>                  - Execute the Lua script code in file (www.lua.org).",
	"lua 'lua string'                   - Execute the Lua code in the string needs quotes",
	"geometry <geom>                    - Set the display geometry Columns by Rows (ColxRow)",
	"clear <portlist> stats             - Clear the statistics",
	"clr                                - Clear all Statistices",
	"reset <portlist>                   - Reset the configuration the ports to the default",
	"rst                                - Reset the configuration for all ports",
	"ports per page [1-6]               - Set the number of ports displayed per page",
	"port <number>                      - Sets the sequence packets to display for a given port",
	"restart <portlist>                 - Restart or stop a ethernet port and restart",
	"ping4 <portlist>                   - Send a IPv4 ICMP echo request on the given portlist",
#ifdef INCLUDE_PING6
	"ping6 <portlist>                   - Send a IPv6 ICMP echo request on the given portlist",
#endif
	CLI_HELP_PAUSE,
	NULL
};

static int
misc_cmd(int argc, char **argv)
{
	struct cli_map *m;
	portlist_t portlist;
	uint16_t rows, cols;
	char *p;

	m = cli_mapping(misc_map, argc, argv);
	if (!m)
		return command_error("Misc invalid command", "Misc", argc, argv);

	switch(m->index) {
		case 10:
			rte_parse_portlist(argv[1], &portlist);
			foreach_port(portlist,
				     pktgen_clear_stats(info) );
			pktgen_clear_display();
			break;
		case 20:
			p = strchr(argv[1], 'x');
			if (p) {
				rows = strtol(++p, NULL, 10);
				cols = strtol(argv[1], NULL, 10);

				pktgen_display_set_geometry(rows, cols);
				pktgen_clear_display();
			} else
				return -1;
			/* FALLTHRU */
		case 21:
			pktgen_display_get_geometry(&rows, &cols);
			break;
		case 30:
			if (cli_execute_cmdfile(argv[1]) )
				cli_printf("load command failed for %s\n", argv[1]);
			if (!scrn_is_paused() )
				pktgen_force_update();
			break;
		case 40: script_cmd(argc, argv); break;
		case 50: exec_lua_cmd(argc, argv); break;
		case 60: pktgen_save(argv[1]); break;
		case 70: pktgen_clear_display(); break;
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
			pktgen_force_update();
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
	CLI_HELP_PAUSE,
	NULL
};

static int
page_cmd(int argc, char **argv)
{
	struct cli_map *m;

	m = cli_mapping(page_map, argc, argv);
	if (!m)
		return command_error("Page invalid command", "Page", argc, argv);

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

	c_cmd("clear",		misc_cmd,		"clear stats, ..."),
	c_alias("clr",		"clear all stats",	"clear all port stats"),
	c_cmd("geometry",	misc_cmd, 		"set the screen geometry"),
	c_alias("geom",		"geometry",		"set or show screen geometry"),
	c_cmd("load",		misc_cmd, 		"load command file"),
	c_cmd("script", 	misc_cmd,		"run a Lua script"),
	c_cmd("lua", 		misc_cmd,		"execute a Lua string"),
	c_cmd("save", 		misc_cmd,		"save the current state"),
	c_cmd("redisplay",	misc_cmd,		"redisplay the screen"),
	c_alias("cls",		"redisplay",	"redraw screen"),
	c_cmd("reset",		misc_cmd,		"reset pktgen configuration"),
	c_alias("rst",          "reset all",    "reset all ports"),
	c_cmd("restart", 	misc_cmd,		"restart port"),
	c_cmd("port", 		misc_cmd, 		"Switch between ports"),
	c_cmd("ping4", 		misc_cmd, 		"Send a ping packet for IPv4"),
#ifdef INCLUDE_PING6
	c_cmd("ping6", 		misc_cmd,		"Send a ping packet for IPv6"),
#endif

	c_cmd("sequence",	seq_cmd,		"sequence command"),
	c_alias("seq",		"sequence",		"sequence command"),

	c_cmd("page",		page_cmd,		"change page displays"),
	c_cmd("theme", 		theme_cmd,		"Set, save, show the theme"),
	c_cmd("range",		range_cmd,		"Range commands"),
	c_cmd("enable",		en_dis_cmd,		"enable features"),
	c_cmd("disable",	en_dis_cmd,		"disable features"),
	c_cmd("start",		start_stop_cmd,	"start features"),
	c_cmd("stop",		start_stop_cmd,	"stop features"),
	c_alias("str",		"start all",	"start all ports sending packets"),
	c_alias("stp",		"stop all",		"stop all ports sending packets"),
	c_cmd("pcap",		pcap_cmd, 		"pcap commands"),
	c_cmd("set", 		set_cmd, 		"set a number of options"),
	c_cmd("debug",          debug_cmd,		"debug commands"),

	c_alias("on",       "enable screen","Enable screen updates"),
	c_alias("off",      "disable screen", "Disable screen updates"),

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

	cli_help_add("Title", NULL, title_help);
	cli_help_add("Page", page_map, page_help);
	cli_help_add("Enable", enable_map, enable_help);
	cli_help_add("Set", set_map, set_help);
	cli_help_add("Range", range_map, range_help);
	cli_help_add("Sequence", seq_map, seq_help);
	cli_help_add("PCAP", pcap_map, pcap_help);
	cli_help_add("Start", start_map, start_help);
	cli_help_add("Debug", debug_map, debug_help);
	cli_help_add("Misc", misc_map, misc_help);
	cli_help_add("Theme", theme_map, theme_help);
	cli_help_add("Status", NULL, status_help);

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
    cli_start_with_timers(NULL);

    cli_destroy();
}

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
	scrn_pos(1, 1);

	cli_help_show_all("** Pktgen Help Information **");

	if (!paused) {
		scrn_setw(pktgen.last_row + 1);
		scrn_resume();
		pktgen_clear_display();
	}
	return 0;
}
