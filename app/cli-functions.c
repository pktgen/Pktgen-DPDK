/*-
 * Copyright(c) <2020-2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2018 by Keith Wiles @ intel.com */

#include "cli-functions.h"

#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include <rte_version.h>
#include <rte_atomic.h>
#include <rte_devargs.h>
#include <rte_ether.h>
#include <rte_string_fns.h>
#include <pg_strings.h>
#include <rte_hexdump.h>
#include <rte_cycles.h>
#include <rte_malloc.h>

#include <lua_config.h>

#include "pktgen.h"

#include <cli.h>
#include <cli_map.h>
#include <plugin.h>

#include "copyright_info.h"
#include "pktgen-cmds.h"
#include "pktgen-main.h"
#include "lpktgenlib.h"
#include "pktgen-display.h"
#include "pktgen-random.h"
#include "pktgen-log.h"
#include "pg_ether.h"
#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
#include <rte_eth_bond.h>
#include <rte_eth_bond_8023ad.h>
#endif

static inline uint16_t
valid_pkt_size(char *val)
{
    uint16_t pkt_size;

    if (!val)
        return (MIN_PKT_SIZE + RTE_ETHER_CRC_LEN);

    pkt_size = atoi(val);
    if (pkt_size < pktgen.eth_min_pkt)
        pkt_size = pktgen.eth_min_pkt;

    if (pkt_size > pktgen.eth_max_pkt)
        pkt_size = pktgen.eth_max_pkt;

    return pkt_size;
}

static inline uint16_t
valid_gtpu_teid(port_info_t *info __rte_unused, char *val)
{
    uint16_t gtpu_teid;

    gtpu_teid = atoi(val);

    return gtpu_teid;
}

/**********************************************************/
static const char *title_help[] = {
    "   *** Pktgen Help information ***",
    "",
    NULL,
};

static const char *status_help[] = {
    "",
    "       Flags: P----------------- - Promiscuous mode enabled",
    "               E                 - ICMP Echo enabled",
    "                B                - Bonding enabled LACP 802.3ad",
    "                 I               - Process packets on input enabled",
    "                  *              - Using TAP interface for this port can be [-rt*]",
    "                   C             - Capture received packets",
    "                    ------       - Modes Single, pcap, sequence, latency, random, Rate",
    "                          ------ - Modes VLAN, VxLAN, MPLS, QnQ, GRE IPv4, GRE ETH",
    "Notes: <state>       - Use enable|disable or on|off to set the state.",
    "       <portlist>    - a list of ports (no spaces) as 2,4,6-9,12 or 3-5,8 or 5 or the word "
    "'all'",
    "       Colors best seen on a black background for now",
    CLI_HELP_PAUSE,
    NULL};

#define SMMI "%|start|minimum|maximum|increment|min|max|inc"
// clang-format off
static struct cli_map range_map[] = {
    {20, "range %P dst mac " SMMI " %m"},
    {21, "range %P src mac " SMMI " %m"},
    {22, "range %P dst mac %m %m %m %m"},
    {23, "range %P src mac %m %m %m %m"},
    {30, "range %P dst ip " SMMI " %4"},
    {31, "range %P src ip " SMMI " %4"},
    {32, "range %P dst ip %4 %4 %4 %4"},
    {33, "range %P src ip %4 %4 %4 %4"},
    {34, "range %P dst ip " SMMI " %6"},
    {35, "range %P src ip " SMMI " %6"},
    {36, "range %P dst ip %6 %6 %6 %6"},
    {37, "range %P src ip %6 %6 %6 %6"},
    {40, "range %P proto %|tcp|udp"},
    {41, "range %P type %|ipv4|ipv6"},
    {42, "range %P tcp flag set %|urg|ack|psh|rst|syn|fin|all"},
    {43, "range %P tcp flag clr %|urg|ack|psh|rst|syn|fin|all"},
    {44, "range %P tcp seq %d %d %d %d"},
    {45, "range %P tcp ack %d %d %d %d"},
    {46, "range %P tcp seq " SMMI " %d"},
    {47, "range %P tcp ack " SMMI " %d"},
    {50, "range %P dst port " SMMI " %d"},
    {51, "range %P src port " SMMI " %d"},
    {52, "range %P dst port %d %d %d %d"},
    {53, "range %P src port %d %d %d %d"},
    {55, "range %P ttl " SMMI " %b"},
    {56, "range %P ttl %b %b %b %b"},
    {60, "range %P vlan " SMMI " %d"},
    {61, "range %P vlan %d %d %d %d"},
    {70, "range %P size " SMMI " %d"},
    {71, "range %P size %d %d %d %d"},
    {80, "range %P mpls entry %h"},
    {85, "range %P qinq index %d %d"},
    {90, "range %P gre key %d"},
    {91, "range %P gre_key %d"},
    {100, "range %P gtpu " SMMI " %d"},
    {101, "range %P gtpu %d %d %d %d"},
    {160, "range %P cos " SMMI " %d"},
    {161, "range %P cos %d %d %d %d"},
    {170, "range %P tos " SMMI " %d"},
    {171, "range %P tos %d %d %d %d"},
    {172, "range %P hop_limits " SMMI " %b"},
    {173, "range %P hop_limits %d %d %d %d"},
    {174, "range %P traffic_class " SMMI " %d"},
    {175, "range %P traffic_class %d %d %d %d"},
    {-1, NULL}
};
// clang-format on

static const char *range_help[] = {
    "",
    "  -- Setup the packet range values --",
    "     note: SMMI = start|min|max|inc (start, minimum, maximum, increment)",
    "",
    "range <portlist> src|dst mac <SMMI> <etheraddr> - Set destination/source MAC address",
    "      e.g: range 0 src mac start 00:00:00:00:00:00",
    "           range 0 dst mac max 00:12:34:56:78:90",
    "      or  range 0 src mac 00:00:00:00:00:00 00:00:00:00:00:00 00:12:34:56:78:90 "
    "00:00:00:01:01:01",
    "range <portlist> src|dst ip <SMMI> <ipaddr>   - Set source IP start address",
    "      e.g: range 0 dst ip start 0.0.0.0",
    "           range 0 dst ip min 0.0.0.0",
    "           range 0 dst ip max 1.2.3.4",
    "           range 0 dst ip inc 0.0.1.0",
    "       or  range 0 dst ip 0.0.0.0 0.0.0.0 1.2.3.4 0.0.1.0",
    "range <portlist> type ipv4|ipv6               - Set the range packet type to IPv4 or IPv6",
    "range <portlist> proto tcp|udp                - Set the IP protocol type",
    "range <portlist> tcp flag set <flag>          - Set the TCP flag",
    "range <portlist> tcp flag clr <flag>          - Clear the TCP flag",
    "range <portlist> tcp seq <SMMI> <value>       - Set the TCP sequence number",
    "       or  range <portlist> tcp seq <start> <min> <max> <inc>",
    "range <portlist> tcp ack <SMMI> <value>       - Set the TCP acknowledge number",
    "       or  range <portlist> tcp ack <start> <min> <max> <inc>",
    "range <portlist> src|dst port <SMMI> <value>  - Set UDP/TCP source/dest port number",
    "       or  range <portlist> src|dst port <start> <min> <max> <inc>",
    "range <portlist> vlan <SMMI> <value>          - Set vlan id start address",
    "      or  range <portlist> vlan <start> <min> <max> <inc>",
    "range <portlist> size <SMMI> <value>          - Set pkt size start address",
    "      or  range <portlist> size <start> <min> <max> <inc>",
    "range <portlist> teid <SMMI> <value>          - Set TEID value",
    "      or  range <portlist> teid <start> <min> <max> <inc>",
    "range <portlist> mpls entry <hex-value>       - Set MPLS entry value",
    "range <portlist> qinq index <val1> <val2>     - Set QinQ index values",
    "range <portlist> gre key <value>              - Set GRE key value",
    "range <portlist> cos <SMMI> <value>           - Set cos value",
    "range <portlist> tos <SMMI> <value>           - Set tos value",
    "range <portlist> ttl <SMMI> <value>           - Set TTL",
    "range <portlist> hop_limits <SMMI> <value>    - Set Hop Limits",
    "range <portlist> traffic_class <SMMI> <value> - Set Traffic Class value",

    CLI_HELP_PAUSE,
    NULL};

static int
range_cmd(int argc, char **argv)
{
    struct cli_map *m;
    portlist_t portlist;
    struct pg_ipaddr ip;
    struct rte_ether_addr mac[4];
    char *what, *p;
    const char *val;

    m = cli_mapping(range_map, argc, argv);
    if (!m)
        return cli_cmd_error("Range command error", "Range", argc, argv);

    portlist_parse(argv[1], pktgen.nb_ports, &portlist);

    what = argv[4];
    val  = (const char *)argv[5];
    switch (m->index) {
    case 20:
        if (pg_ether_aton(val, &mac[0]) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", val);
            break;
        }
        foreach_port(portlist, range_set_dest_mac(info, what, &mac[0]));
        break;
    case 21:
        if (pg_ether_aton(val, &mac[0]) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", val);
            break;
        }
        foreach_port(portlist, range_set_src_mac(info, what, &mac[0]));
        break;
    case 22:
        if (pg_ether_aton(argv[4], &mac[0]) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[4]);
            break;
        }
        if (pg_ether_aton(argv[5], &mac[1]) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[5]);
            break;
        }
        if (pg_ether_aton(argv[6], &mac[2]) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[6]);
            break;
        }
        if (pg_ether_aton(argv[7], &mac[3]) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[7]);
            break;
        }
        foreach_port(portlist, range_set_dest_mac(info, "start", &mac[0]);
                     range_set_dest_mac(info, "min", &mac[1]);
                     range_set_dest_mac(info, "max", &mac[2]);
                     range_set_dest_mac(info, "inc", &mac[3]));
        break;
    case 23:
        if (pg_ether_aton(argv[4], &mac[0]) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[4]);
            break;
        }
        if (pg_ether_aton(argv[5], &mac[1]) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[5]);
            break;
        }
        if (pg_ether_aton(argv[6], &mac[2]) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[6]);
            break;
        }
        if (pg_ether_aton(argv[7], &mac[3]) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[7]);
            break;
        }
        foreach_port(portlist, range_set_src_mac(info, "start", &mac[0]);
                     range_set_src_mac(info, "min", &mac[1]);
                     range_set_src_mac(info, "max", &mac[2]);
                     range_set_src_mac(info, "inc", &mac[3]));
        break;
    case 30:
        /* Remove the /XX mask value is supplied */
        p = strchr(argv[4], '/');
        if (p)
            *p = '\0';
        _atoip(val, 0, &ip, sizeof(ip));
        foreach_port(portlist, range_set_dst_ip(info, what, &ip));
        break;
    case 31:
        /* Remove the /XX mask value is supplied */
        p = strchr(argv[4], '/');
        if (p)
            *p = '\0';
        _atoip(argv[5], 0, &ip, sizeof(ip));
        foreach_port(portlist, range_set_src_ip(info, what, &ip));
        break;
    case 32:
        foreach_port(portlist, _atoip(argv[4], PG_IPADDR_V4, &ip, sizeof(ip));
                     range_set_dst_ip(info, (char *)(uintptr_t) "start", &ip);
                     _atoip(argv[5], PG_IPADDR_V4, &ip, sizeof(ip));
                     range_set_dst_ip(info, (char *)(uintptr_t) "min", &ip);
                     _atoip(argv[6], PG_IPADDR_V4, &ip, sizeof(ip));
                     range_set_dst_ip(info, (char *)(uintptr_t) "max", &ip);
                     _atoip(argv[7], PG_IPADDR_V4, &ip, sizeof(ip));
                     range_set_dst_ip(info, (char *)(uintptr_t) "inc", &ip));
        break;
    case 33:
        foreach_port(portlist, _atoip(argv[4], PG_IPADDR_V4, &ip, sizeof(ip));
                     range_set_src_ip(info, (char *)(uintptr_t) "start", &ip);
                     _atoip(argv[5], PG_IPADDR_V4, &ip, sizeof(ip));
                     range_set_src_ip(info, (char *)(uintptr_t) "min", &ip);
                     _atoip(argv[6], PG_IPADDR_V4, &ip, sizeof(ip));
                     range_set_src_ip(info, (char *)(uintptr_t) "max", &ip);
                     _atoip(argv[7], PG_IPADDR_V4, &ip, sizeof(ip));
                     range_set_src_ip(info, (char *)(uintptr_t) "inc", &ip));
        break;
    case 34:
        /* Remove the /XX mask value is supplied */
        p = strchr(argv[4], '/');
        if (p)
            *p = '\0';
        _atoip(val, 0, &ip, sizeof(ip));
        foreach_port(portlist, range_set_dst_ip(info, what, &ip));
        break;
    case 35:
        /* Remove the /XX mask value is supplied */
        p = strchr(argv[4], '/');
        if (p)
            *p = '\0';
        _atoip(argv[5], 0, &ip, sizeof(ip));
        foreach_port(portlist, range_set_src_ip(info, what, &ip));
        break;
    case 36:
        foreach_port(portlist, _atoip(argv[4], PG_IPADDR_V6, &ip, sizeof(ip));
                     range_set_dst_ip(info, (char *)(uintptr_t) "start", &ip);
                     _atoip(argv[5], PG_IPADDR_V6, &ip, sizeof(ip));
                     range_set_dst_ip(info, (char *)(uintptr_t) "min", &ip);
                     _atoip(argv[6], PG_IPADDR_V6, &ip, sizeof(ip));
                     range_set_dst_ip(info, (char *)(uintptr_t) "max", &ip);
                     _atoip(argv[7], PG_IPADDR_V6, &ip, sizeof(ip));
                     range_set_dst_ip(info, (char *)(uintptr_t) "inc", &ip));
        break;
    case 37:
        foreach_port(portlist, _atoip(argv[4], PG_IPADDR_V6, &ip, sizeof(ip));
                     range_set_src_ip(info, (char *)(uintptr_t) "start", &ip);
                     _atoip(argv[5], PG_IPADDR_V6, &ip, sizeof(ip));
                     range_set_src_ip(info, (char *)(uintptr_t) "min", &ip);
                     _atoip(argv[6], PG_IPADDR_V6, &ip, sizeof(ip));
                     range_set_src_ip(info, (char *)(uintptr_t) "max", &ip);
                     _atoip(argv[7], PG_IPADDR_V6, &ip, sizeof(ip));
                     range_set_src_ip(info, (char *)(uintptr_t) "inc", &ip));
        break;
    case 40:
        foreach_port(portlist, range_set_proto(info, argv[3]));
        break;
    case 41:
        foreach_port(portlist, range_set_pkt_type(info, argv[3]));
        break;
    case 42:
        foreach_port(portlist, range_set_tcp_flag_set(info, argv[5]));
        break;
    case 43:
        foreach_port(portlist, range_set_tcp_flag_clr(info, argv[5]));
        break;
    case 44:
        foreach_port(portlist, range_set_tcp_seq(info, (char *)(uintptr_t) "start", atoi(argv[4]));
                     range_set_tcp_seq(info, (char *)(uintptr_t) "min", atoi(argv[5]));
                     range_set_tcp_seq(info, (char *)(uintptr_t) "max", atoi(argv[6]));
                     range_set_tcp_seq(info, (char *)(uintptr_t) "inc", atoi(argv[7])));
        break;
    case 45:
        foreach_port(portlist, range_set_tcp_ack(info, (char *)(uintptr_t) "start", atoi(argv[4]));
                     range_set_tcp_ack(info, (char *)(uintptr_t) "min", atoi(argv[5]));
                     range_set_tcp_ack(info, (char *)(uintptr_t) "max", atoi(argv[6]));
                     range_set_tcp_ack(info, (char *)(uintptr_t) "inc", atoi(argv[7])));
        break;
    case 46:
        foreach_port(portlist, range_set_tcp_seq(info, what, atoi(val)));
        break;
    case 47:
        foreach_port(portlist, range_set_tcp_ack(info, what, atoi(val)));
        break;
    case 50:
        foreach_port(portlist, range_set_dst_port(info, what, atoi(val)));
        break;
    case 51:
        foreach_port(portlist, range_set_src_port(info, what, atoi(val)));
        break;
    case 52:
        foreach_port(portlist, range_set_dst_port(info, (char *)(uintptr_t) "start", atoi(argv[4]));
                     range_set_dst_port(info, (char *)(uintptr_t) "min", atoi(argv[5]));
                     range_set_dst_port(info, (char *)(uintptr_t) "max", atoi(argv[6]));
                     range_set_dst_port(info, (char *)(uintptr_t) "inc", atoi(argv[7])));
        break;
    case 53:
        foreach_port(portlist, range_set_src_port(info, (char *)(uintptr_t) "start", atoi(argv[4]));
                     range_set_src_port(info, (char *)(uintptr_t) "min", atoi(argv[5]));
                     range_set_src_port(info, (char *)(uintptr_t) "max", atoi(argv[6]));
                     range_set_src_port(info, (char *)(uintptr_t) "inc", atoi(argv[7])));
        break;
    case 55:
        foreach_port(portlist, range_set_ttl(info, argv[3], atoi(argv[4])));
        break;
    case 56:
        foreach_port(portlist, range_set_ttl(info, (char *)(uintptr_t) "start", atoi(argv[3]));
                     range_set_ttl(info, (char *)(uintptr_t) "min", atoi(argv[4]));
                     range_set_ttl(info, (char *)(uintptr_t) "max", atoi(argv[5]));
                     range_set_ttl(info, (char *)(uintptr_t) "inc", atoi(argv[6])));
        break;
    case 60:
        foreach_port(portlist, range_set_vlan_id(info, argv[3], atoi(what)));
        break;
    case 61:
        foreach_port(portlist, range_set_vlan_id(info, (char *)(uintptr_t) "start", atoi(argv[3]));
                     range_set_vlan_id(info, (char *)(uintptr_t) "min", atoi(argv[4]));
                     range_set_vlan_id(info, (char *)(uintptr_t) "max", atoi(argv[5]));
                     range_set_vlan_id(info, (char *)(uintptr_t) "inc", atoi(argv[6])));
        break;
    case 70:
        foreach_port(portlist, range_set_pkt_size(info, argv[3],
                                                  strcmp("inc", argv[3]) ? valid_pkt_size(what)
                                                                         : atoi(what)));
        break;
    case 71:
        foreach_port(portlist,
                     range_set_pkt_size(info, (char *)(uintptr_t) "start", valid_pkt_size(argv[3]));
                     range_set_pkt_size(info, (char *)(uintptr_t) "min", valid_pkt_size(argv[4]));
                     range_set_pkt_size(info, (char *)(uintptr_t) "max", valid_pkt_size(argv[5]));
                     range_set_pkt_size(info, (char *)(uintptr_t) "inc", atoi(argv[6])););
        break;
    case 80:
        foreach_port(portlist, range_set_mpls_entry(info, strtoul(what, NULL, 16)));
        break;
    case 85:
        foreach_port(portlist, range_set_qinqids(info, atoi(what), atoi(val)));
        break;
    case 90:
    case 91:
        foreach_port(portlist, range_set_gre_key(info, strtoul(what, NULL, 10)));
        break;
    case 100:
        foreach_port(
            portlist,
            range_set_gtpu_teid(info, argv[3],
                                strcmp("inc", argv[3]) ? valid_gtpu_teid(info, what) : atoi(what)));
        break;
    case 101:
        foreach_port(
            portlist,
            range_set_gtpu_teid(info, (char *)(uintptr_t) "start", valid_gtpu_teid(info, argv[3]));
            range_set_gtpu_teid(info, (char *)(uintptr_t) "min", valid_gtpu_teid(info, argv[4]));
            range_set_gtpu_teid(info, (char *)(uintptr_t) "max", valid_gtpu_teid(info, argv[5]));
            range_set_gtpu_teid(info, (char *)(uintptr_t) "inc", atoi(argv[6])););
        break;
    case 160:
        foreach_port(portlist, range_set_cos_id(info, argv[3], atoi(what)));
        break;
    case 161:
        foreach_port(portlist, range_set_cos_id(info, (char *)(uintptr_t) "start", atoi(argv[3]));
                     range_set_cos_id(info, (char *)(uintptr_t) "min", atoi(argv[4]));
                     range_set_cos_id(info, (char *)(uintptr_t) "max", atoi(argv[5]));
                     range_set_cos_id(info, (char *)(uintptr_t) "inc", atoi(argv[6])););
        break;
    case 170:
        foreach_port(portlist, range_set_tos_id(info, argv[3], atoi(what)));
        break;
    case 171:
        foreach_port(portlist, range_set_tos_id(info, (char *)(uintptr_t) "start", atoi(argv[3]));
                     range_set_tos_id(info, (char *)(uintptr_t) "min", atoi(argv[4]));
                     range_set_tos_id(info, (char *)(uintptr_t) "max", atoi(argv[5]));
                     range_set_tos_id(info, (char *)(uintptr_t) "inc", atoi(argv[6])));
        break;
    case 172:
        foreach_port(portlist, range_set_hop_limits(info, argv[3], atoi(what)));
        break;
    case 173:
        foreach_port(portlist,
                     range_set_hop_limits(info, (char *)(uintptr_t) "start", atoi(argv[3]));
                     range_set_hop_limits(info, (char *)(uintptr_t) "min", atoi(argv[4]));
                     range_set_hop_limits(info, (char *)(uintptr_t) "max", atoi(argv[5]));
                     range_set_hop_limits(info, (char *)(uintptr_t) "inc", atoi(argv[6])));
        break;
    case 174:
        foreach_port(portlist, range_set_traffic_class(info, argv[3], atoi(what)));
        break;
    case 175:
        foreach_port(portlist,
                     range_set_traffic_class(info, (char *)(uintptr_t) "start", atoi(argv[3]));
                     range_set_traffic_class(info, (char *)(uintptr_t) "min", atoi(argv[4]));
                     range_set_traffic_class(info, (char *)(uintptr_t) "max", atoi(argv[5]));
                     range_set_traffic_class(info, (char *)(uintptr_t) "inc", atoi(argv[6])));
        break;

    default:
        return cli_cmd_error("Range command error", "Range", argc, argv);
    }
    pktgen_update_display();
    return 0;
}

#define set_types         \
    "count|"     /*  0 */ \
    "size|"      /*  1 */ \
    "rate|"      /*  2 */ \
    "burst|"     /*  3 */ \
    "tx_cycles|" /*  4 */ \
    "sport|"     /*  5 */ \
    "dport|"     /*  6 */ \
    "prime|"     /*  7 */ \
    "dump|"      /*  8 */ \
    "vlan|"      /*  9 */ \
    "vlanid|"    /* 10 */ \
    "seq_cnt|"   /* 11 */ \
    "seqCnt|"    /* 12 */ \
    "seqcnt|"    /* 13 */ \
    "ttl|"       /* 14 */ \
    "txburst|"   /* 15 */ \
    "rxburst"    /* 16 */

// clang-format off
static struct cli_map set_map[] = {
    {10, "set %P %|" set_types " %d"},
    {11, "set %P jitter %D"},
    {20, "set %P type %|arp|ipv4|ipv6|ip4|ip6|vlan"},
    {21, "set %P proto %|udp|tcp|icmp"},
    {22, "set %P src mac %m"},
    {23, "set %P dst mac %m"},
    {24, "set %P pattern %|abc|none|user|zero"},
    {25, "set %P user pattern %s"},
    {30, "set %P src ip %4"},
    {31, "set %P dst ip %4"},
    {32, "set %P src ip %6"},
    {33, "set %P dst ip %6"},
    {34, "set %P tcp flag set %|urg|ack|psh|rst|syn|fin|all"},
    {35, "set %P tcp flag clr %|urg|ack|psh|rst|syn|fin|all"},
    {36, "set %P tcp seq %d"},
    {37, "set %P tcp ack %d"},
    {40, "set ports_per_page %d"},
    {50, "set %P qinqids %d %d"},
    {60, "set %P rnd %d %d %s"},
    {70, "set %P cos %d"},
    {80, "set %P tos %d"},
    {90, "set %P vxlan %h %d %d"},
    {100, "set %P latsampler %|simple|poisson %d %d %s"},
    {-1, NULL}
};
// clang format on

static const char *set_help[] = {
    "",
    "    note: <portlist>               - a list of ports (no spaces) e.g. 2,4,6-9,12 or the word "
    "'all'",
    "set <portlist> count <value>       - number of packets to transmit",
    "set <portlist> size <value>        - size of the packet to transmit",
    "set <portlist> rate <percent>      - Packet rate in percentage",
    "set <portlist> txburst|burst <value> - number of packets in a TX burst",
    "set <portlist> rxburst <value>     - number of packets in a RX burst",
    "set <portlist> tx_cycles <value>   - DEBUG to set the number of cycles per TX burst",
    "set <portlist> sport <value>       - Source port number for UDP/TCP",
    "set <portlist> dport <value>       - Destination port number for UDP/TCP",
    "set <portlist> ttl <value>         - Set the TTL value for the single port more",
    "set <portlist> seq_cnt|seqcnt|seqCnt <value>",
    "                                   - Set the number of packet in the sequence to send [0-16]",
    "set <portlist> prime <value>       - Set the number of packets to send on prime command",
    "set <portlist> dump <value>        - Dump the next 1-32 received packets to the screen",
    "                                     Dumped packets are in the log, use 'page log' to view",
    "set <portlist> vlan|vlanid <value> - Set the VLAN ID value for the portlist",
    "set <portlist> jitter <value>      - Set the jitter threshold in micro-seconds",
    "set <portlist> src|dst mac <addr>  - Set MAC addresses 00:11:22:33:44:55 or 0011:2233:4455 "
    "format",
    "set <portlist> type ipv4|ipv6|vlan|arp - Set the packet type to IPv4 or IPv6 or VLAN",
    "set <portlist> proto udp|tcp|icmp  - Set the packet protocol to UDP or TCP or ICMP per port",
    "set <portlist> pattern <type>      - Set the fill pattern type",
    "                 type - abc        - Default pattern of abc string",
    "                        none       - No fill pattern, maybe random data",
    "                        zero       - Fill of zero bytes",
    "                        user       - User supplied string of max 16 bytes",
    "set <portlist> user pattern <string> - A 16 byte string, must set 'pattern user' command",
    "set <portlist> [src|dst] ip ipaddr - Set IP addresses, Source must include network mask e.g. "
    "10.1.2.3/24",
    "set <portlist> tcp flag set urg|ack|psh|rst|syn|fin|all - Set a TCP flag",
    "set <portlist> tcp flag clr urg|ack|psh|rst|syn|fin|all - Clear a TCP flag",
    "set <portlist> tcp seq <sequence> - Set the TCP sequence number",
    "set <portlist> tcp ack <acknowledge> - Set the TCP acknowledge number",
    "set <portlist> qinqids <id1> <id2> - Set the Q-in-Q ID's for the portlist",
    "set <portlist> rnd <idx> <off> <mask> - Set random mask for all transmitted packets from "
    "portlist",
    "    idx: random mask index slot",
    "    off: offset in bytes to apply mask value",
    "    mask: up to 32 bit long mask specification (empty to disable):",
    "          0: bit will be 0",
    "          1: bit will be 1",
    "          .: bit will be ignored (original value is retained)",
    "          X: bit will get random value",
    "set <portlist> cos <value>         - Set the CoS value for the portlist",
    "set <portlist> tos <value>         - Set the ToS value for the portlist",
    "set <portlist> vxlan <flags> <group id> <vxlan_id> - Set the vxlan values",
    "set <portlist> latsampler [simple|poisson] <num-samples> <rate> <outfile>	- Set latency "
    "sampler parameters",
    "		num-samples: number of samples.",
    "		rate: sampling rate i.e., samples per second.",
    "		outfile: path to output file to dump all sampled latencies",
    "set ports_per_page <value>         - Set ports per page value 1 - 6",
    CLI_HELP_PAUSE,
    NULL};

static int
set_cmd(int argc, char **argv)
{
    portlist_t portlist;
    struct rte_ether_addr mac;
    char *what, *p;
    int value, n;
    struct cli_map *m;
    struct pg_ipaddr ip;
    uint16_t id1, id2;
    uint32_t u1, u2;
    int ip_ver;

    m = cli_mapping(set_map, argc, argv);
    if (!m)
        return cli_cmd_error("Set command is invalid", "Set", argc, argv);

    portlist_parse(argv[1], pktgen.nb_ports, &portlist);

    what  = argv[2];
    value = atoi(argv[3]);

    switch (m->index) {
    case 10:
        n = cli_map_list_search(m->fmt, argv[2], 2);
        foreach_port(portlist, _do(switch (n) {
            case 0:
                single_set_tx_count(info, value);
                break;
            case 1:
                single_set_pkt_size(info, valid_pkt_size(argv[3]));
                break;
            case 2:
                single_set_tx_rate(info, argv[3]);
                break;
            case 3:
                single_set_tx_burst(info, value);
                break;
            case 4:
                debug_set_tx_cycles(info, value);
                break;
            case 5:
                single_set_port_value(info, what[0], value);
                break;
            case 6:
                single_set_port_value(info, what[0], value);
                break;
            case 7:
                pktgen_set_port_prime(info, value);
                break;
            case 8:
                debug_set_port_dump(info, value);
                break;
            case 9: /* vlanid and vlan are valid */
            case 10:
                single_set_vlan_id(info, value);
                break;
            case 11:
                /* FALLTHRU */
            case 12:
                /* FALLTHRU */
            case 13:
                pktgen_set_port_seqCnt(info, value);
                break;
            case 14:
                single_set_ttl_value(info, value);
                break;
            case 15:
                single_set_tx_burst(info, value);
                break;
            case 16:
                single_set_rx_burst(info, value);
                break;
            default:
                return cli_cmd_error("Set command is invalid", "Set", argc, argv);
        }));
        break;
    case 11:
        foreach_port(portlist, single_set_jitter(info, strtoull(argv[3], NULL, 0)));
        break;
    case 20:
        foreach_port(portlist, single_set_pkt_type(info, argv[3]));
        break;
    case 21:
        foreach_port(portlist, single_set_proto(info, argv[3]));
        break;
    case 22:
        if (pg_ether_aton(argv[4], &mac) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[4]);
            break;
        }
        foreach_port(portlist, single_set_src_mac(info, &mac));
        break;
    case 23:
        if (pg_ether_aton(argv[4], &mac) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[4]);
            break;
        }
        foreach_port(portlist, single_set_dst_mac(info, &mac));
        break;
    case 24:
        foreach_port(portlist, pattern_set_type(info, argv[3]));
        break;
    case 25:
        foreach_port(portlist, pattern_set_user_pattern(info, argv[4]));
        break;
    case 30:
        p = strchr(argv[4], '/');
        if (!p)
            cli_printf("src IP address should contain subnet value, default /32 for IPv4\n");
        ip_ver = _atoip(argv[4], PG_IPADDR_V4 | PG_IPADDR_NETWORK, &ip, sizeof(ip));
        foreach_port(portlist, single_set_ipaddr(info, 's', &ip, ip_ver));
        break;
    case 31:
        /* Remove the /XX mask value if supplied */
        p = strchr(argv[4], '/');
        if (p) {
            cli_printf("Subnet mask not required, removing subnet mask value\n");
            *p = '\0';
        }
        ip_ver = _atoip(argv[4], PG_IPADDR_V4, &ip, sizeof(ip));
        foreach_port(portlist, single_set_ipaddr(info, 'd', &ip, ip_ver));
        break;
    case 32:
        p = strchr(argv[4], '/');
        if (!p)
            cli_printf("src IP address should contain subnet value, default /128 for IPv6\n");

        ip_ver = _atoip(argv[4], PG_IPADDR_V6 | PG_IPADDR_NETWORK, &ip, sizeof(ip));
        foreach_port(portlist, single_set_ipaddr(info, 's', &ip, ip_ver));
        break;
    case 33:
        /* Remove the /XX mask value if supplied */
        p = strchr(argv[4], '/');
        if (p) {
            cli_printf("Subnet mask not required, removing subnet mask value\n");
            *p = '\0';
        }
        ip_ver = _atoip(argv[4], PG_IPADDR_V6, &ip, sizeof(ip));
        foreach_port(portlist, single_set_ipaddr(info, 'd', &ip, ip_ver));
        break;
    case 34:
        foreach_port(portlist, single_set_tcp_flag_set(info, argv[5]));
        break;
    case 35:
        foreach_port(portlist, single_set_tcp_flag_clr(info, argv[5]));
        break;
    case 36:
        foreach_port(portlist, single_set_tcp_seq(info, atoi(argv[4])));
        break;
    case 37:
        foreach_port(portlist, single_set_tcp_ack(info, atoi(argv[4])));
        break;
    case 40:
        pktgen_set_page_size(atoi(argv[2]));
        break;
    case 50:
        id1 = strtol(argv[3], NULL, 0);
        id2 = strtol(argv[4], NULL, 0);
        foreach_port(portlist, single_set_qinqids(info, id1, id2));
        break;
    case 60: {
        char mask[34] = {0}, *m;
        char cb;

        id1 = strtol(argv[3], NULL, 0);
        id2 = strtol(argv[4], NULL, 0);
        m   = argv[5];
        if (strcmp(m, "off")) {
            int idx;
            /* Filter invalid characters from provided mask. This way the user can
             * more easily enter long bitmasks, using for example '_' as a separator
             * every 8 bits. */
            for (n = 0, idx = 0; (idx < 32) && ((cb = m[n]) != '\0'); n++)
                if ((cb == '0') || (cb == '1') || (cb == '.') || (cb == 'X') || (cb == 'x'))
                    mask[idx++] = cb;
        }
        foreach_port(portlist, enable_random(info, pktgen_set_random_bitfield(info->rnd_bitfields,
                                                                              id1, id2, mask)
                                                       ? ENABLE_STATE
                                                       : DISABLE_STATE));
    } break;
    case 70:
        id1 = strtol(argv[3], NULL, 0);
        foreach_port(portlist, single_set_cos(info, id1));
        break;
    case 80:
        id1 = strtol(argv[3], NULL, 0);
        foreach_port(portlist, single_set_tos(info, id1));
        break;
    case 90:
        id1 = strtol(argv[3], NULL, 0);
        id2 = strtol(argv[4], NULL, 0);
        u1  = strtol(argv[5], NULL, 0);
        foreach_port(portlist, single_set_vxlan(info, id1, id2, u1));
        break;
    case 100:
        u1 = strtol(argv[4], NULL, 0);
        u2 = strtol(argv[5], NULL, 0);
        foreach_port(portlist, single_set_latsampler_params(info, argv[3], u1, u2, argv[6]));
        break;
    default:
        return cli_cmd_error("Command invalid", "Set", argc, argv);
    }

    pktgen_update_display();
    return 0;
}

// clang-format off
static struct cli_map pcap_map[] = {
    {10, "pcap %D"},
    {20, "pcap show"},
    {30, "pcap filter %P %s"},
    {-1, NULL}
};
// clang-format on

static const char *pcap_help[] = {
    "",
    "pcap show                          - Show PCAP information",
    "pcap <index>                       - Move the PCAP file index to the given packet number,\n   "
    "    0 - rewind, -1 - end of file",
    "pcap filter <portlist> <string>    - PCAP filter string to filter packets on receive",
    CLI_HELP_PAUSE,
    NULL};

static int
pcap_cmd(int argc, char **argv)
{
    struct cli_map *m;
    pcap_info_t *pcap;
    uint32_t max_cnt;
    uint32_t value;
    portlist_t portlist;

    m = cli_mapping(pcap_map, argc, argv);
    if (!m)
        return cli_cmd_error("PCAP command invalid", "PCAP", argc, argv);

    switch (m->index) {
    case 10:
        pcap  = pktgen.info[pktgen.portNum].pcap;
        value = strtoul(argv[1], NULL, 10);

        if (pcap) {
            max_cnt = pcap->pkt_count;
            if (value >= max_cnt)
                pcap->pkt_idx = max_cnt - RTE_MIN(PCAP_PAGE_SIZE, (int)max_cnt);
            else
                pcap->pkt_idx = value;
            pktgen.flags |= PRINT_LABELS_FLAG;
        } else
            pktgen_log_error(" ** PCAP file is not loaded on port %d", pktgen.portNum);
        break;
    case 20:
        if (pktgen.info[pktgen.portNum].pcap)
            _pcap_info(pktgen.info[pktgen.portNum].pcap, pktgen.portNum, 1);
        else
            pktgen_log_error(" ** PCAP file is not loaded on port %d", pktgen.portNum);
        break;
    case 30:
        portlist_parse(argv[2], pktgen.nb_ports, &portlist);
        foreach_port(portlist, pcap_filter(info, argv[3]));
        break;
    default:
        return cli_cmd_error("PCAP command invalid", "PCAP", argc, argv);
    }
    pktgen_update_display();
    return 0;
}

// clang-format off
static struct cli_map start_map[] = {
    {10, "start %P"},
    {20, "stop %P"},
    {40, "start %P prime"},
    {50, "start %P arp %|request|gratuitous|req|grat"},
    {60, "start %P latsampler"},
    {70, "stop %P latsampler"},
    {-1, NULL}
};
// clagn-format on

static const char *start_help[] = {
    "",
    "start <portlist>                   - Start transmitting packets",
    "stop <portlist>                    - Stop transmitting packets",
    "stp                                - Stop all ports from transmitting",
    "str                                - Start all ports transmitting",
    "start <portlist> prime             - Transmit packets on each port listed. See set prime "
    "command above",
    "start <portlist> arp <type>        - Send a ARP type packet",
    "    type - request | gratuitous | req | grat",
    "start <portlist> latsampler        - Start latency sampler, make sure to set sampling "
    "parameters before starting",
    "stop <portlist> latsampler        	- Stop latency sampler, dumps to file if specified",
    CLI_HELP_PAUSE,
    NULL};

static int
start_stop_cmd(int argc, char **argv)
{
    struct cli_map *m;
    portlist_t portlist;

    m = cli_mapping(start_map, argc, argv);
    if (!m)
        return cli_cmd_error("Start/Stop command invalid", "Start", argc, argv);

    portlist_parse(argv[1], pktgen.nb_ports, &portlist);

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
            foreach_port(portlist, pktgen_send_arp_requests(info, GRATUITOUS_ARP));
        else
            foreach_port(portlist, pktgen_send_arp_requests(info, 0));
        break;
    case 60:
        foreach_port(portlist, pktgen_start_latency_sampler(info));
        break;
    case 70:
        foreach_port(portlist, pktgen_stop_latency_sampler(info));
        break;
    default:
        return cli_cmd_error("Start/Stop command invalid", "Start", argc, argv);
    }
    pktgen_update_display();
    return 0;
}

// clang-format off
static struct cli_map theme_map[] = {
    {0, "theme"},
    {10, "theme %|on|off"},
    {20, "theme %s %s %s %s"},
    {30, "theme save %s"},
    {-1, NULL}
};
// clang-format on

static const char *theme_help[] = {
    "",
    "theme <item> <fg> <bg> <attr>      - Set color for item with fg/bg color and attribute value",
    "theme show                         - List the item strings, colors and attributes to the "
    "items",
    "theme save <filename>              - Save the current color theme to a file",
    CLI_HELP_PAUSE,
    NULL};

static int
theme_cmd(int argc, char **argv)
{
    struct cli_map *m;

    m = cli_mapping(theme_map, argc, argv);
    if (!m)
        return cli_cmd_error("Theme command invalid", "Theme", argc, argv);

    switch (m->index) {
    case 0:
        pktgen_theme_show();
        break;
    case 10:
        pktgen_theme_state(argv[1]);
        pktgen_clear_display();
        break;
    case 20:
        pktgen_set_theme_item(argv[1], argv[2], argv[3], argv[4]);
        break;
    case 30:
        pktgen_theme_save(argv[2]);
        break;
    default:
        return cli_help_show_group("Theme");
    }
    return 0;
}

#define ed_type         \
    "process|" /*  0 */ \
    "mpls|"    /*  1 */ \
    "qinq|"    /*  2 */ \
    "gre|"     /*  3 */ \
    "gre_eth|" /*  4 */ \
    "vlan|"    /*  5 */ \
    "random|"  /*  6 */ \
    "latency|" /*  7 */ \
    "pcap|"    /*  8 */ \
    "blink|"   /*  9 */ \
    "rx_tap|"  /* 10 */ \
    "tx_tap|"  /* 11 */ \
    "icmp|"    /* 12 */ \
    "range|"   /* 13 */ \
    "capture|" /* 14 */ \
    "bonding|" /* 15 */ \
    "vxlan|"   /* 16 */ \
    "rate|"    /* 17 */ \
    "lat"      /* 18 */

// clang-format off
static struct cli_map enable_map[] = {
    {10, "enable %P %|" ed_type},
    {20, "disable %P %|" ed_type},
    {30, "enable %|screen|mac_from_arp"},
    {31, "disable %|screen|mac_from_arp"},
    {40, "enable clock_gettime"},
    {41, "disable clock_gettime"},
    {-1, NULL}
};
// clang-format off

static const char *enable_help[] = {
    "",
    "enable|disable <portlist> process  - Enable or Disable processing of ARP/ICMP/IPv4/IPv6 "
    "packets",
    "enable|disable <portlist> mpls     - Enable/disable sending MPLS entry in packets",
    "enable|disable <portlist> qinq     - Enable/disable sending Q-in-Q header in packets",
    "enable|disable <portlist> gre      - Enable/disable GRE support",
    "enable|disable <portlist> gre_eth  - Enable/disable GRE with Ethernet frame payload",
    "enable|disable <portlist> vlan     - Enable/disable VLAN tagging",
    "enable|disable <portlist> random   - Enable/disable Random packet support",
    "enable|disable <portlist> latency  - Enable/disable latency testing",
    "enable|disable <portlist> pcap     - Enable or Disable sending pcap packets on a portlist",
    "enable|disable <portlist> blink    - Blink LED on port(s)",
    "enable|disable <portlist> rx_tap   - Enable/Disable RX Tap support",
    "enable|disable <portlist> tx_tap   - Enable/Disable TX Tap support",
    "enable|disable <portlist> icmp     - Enable/Disable sending ICMP packets",
    "enable|disable <portlist> range    - Enable or Disable the given portlist for sending a range "
    "of packets",
    "enable|disable <portlist> capture  - Enable/disable packet capturing on a portlist, will "
    "capture RX packets",
    "                                     Disable capture on a port to save the data into the "
    "current working directory.",
    "enable|disable <portlist> bonding  - Enable call TX with zero packets for bonding driver",
    "enable|disable <portlist> vxlan    - Send VxLAN packets",
    "enable|disable <portlist> rate     - Enable/Disable Rate Packing on given ports",
    "enable|disable mac_from_arp        - Enable/disable MAC address from ARP packet",
    "enable|disable clock_gettime       - Enable/disable use of new clock_gettime() instead of rdtsc()",
    "enable|disable screen              - Enable/disable updating the screen and unlock/lock window"
    "    off                            - screen off shortcut",
    "    on                             - screen on shortcut",
    CLI_HELP_PAUSE,
    NULL};

static int
en_dis_cmd(int argc, char **argv)
{
    struct cli_map *m;
    portlist_t portlist;
    int n, state;

    m = cli_mapping(enable_map, argc, argv);
    if (!m)
        return cli_cmd_error("Enable/Disable invalid command", "Enable", argc, argv);

    portlist_parse(argv[1], pktgen.nb_ports, &portlist);

    switch (m->index) {
    case 10:
    case 20:
        n = cli_map_list_search(m->fmt, argv[2], 2);

        state = estate(argv[0]);

        switch (n) {
        case 0:
            foreach_port(portlist, enable_process(info, state));
            break;
        case 1:
            foreach_port(portlist, enable_mpls(info, state));
            break;
        case 2:
            foreach_port(portlist, enable_qinq(info, state));
            break;
        case 3:
            foreach_port(portlist, enable_gre(info, state));
            break;
        case 4:
            foreach_port(portlist, enable_gre_eth(info, state));
            break;
        case 5:
            foreach_port(portlist, enable_vlan(info, state));
            break;
        case 6:
            foreach_port(portlist, enable_random(info, state));
            break;
        case 7:
        case 18: /* lat or latency type */
            foreach_port(portlist, enable_latency(info, state));
            break;
        case 8:
            foreach_port(portlist, enable_pcap(info, state));
            break;
        case 9:
            foreach_port(portlist, debug_blink(info, state));

            if (pktgen.blinklist)
                pktgen.flags |= BLINK_PORTS_FLAG;
            else
                pktgen.flags &= ~BLINK_PORTS_FLAG;
            break;
        case 10:
            foreach_port(portlist, enable_rx_tap(info, state));
            break;
        case 11:
            foreach_port(portlist, enable_tx_tap(info, state));
            break;
        case 12:
            foreach_port(portlist, enable_icmp_echo(info, state));
            break;
        case 13:
            foreach_port(portlist, enable_range(info, state));
            break;
        case 14:
            foreach_port(portlist, pktgen_set_capture(info, state));
            break;
        case 15:
#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
            foreach_port(portlist, enable_bonding(info, state));
#endif
            break;
        case 16:
            foreach_port(portlist, enable_vxlan(info, state));
            break;
        case 17:
            foreach_port(portlist, enable_rate(info, state));
            break;
        default:
            return cli_cmd_error("Enable/Disable invalid command", "Enable", argc, argv);
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
    case 40:
    case 41:
        enable_clock_gettime(estate(argv[0]));
        break;
    default:
        return cli_cmd_error("Enable/Disable invalid command", "Enable", argc, argv);
    }
    pktgen_update_display();
    return 0;
}

#ifdef RTE_LIBRTE_SMEM
#include <rte_smem.h>
#endif

// clang-format off
static struct cli_map dbg_map[] = {
    {10, "dbg l2p"},
    {20, "dbg tx_dbg"},
    {21, "dbg tx_rate %P"},
    {30, "dbg %|mempool|dump %P %s"},
    {40, "dbg pdump %P"},
    {50, "dbg memzone"},
    {51, "dbg memseg"},
    {60, "dbg hexdump %H %d"},
    {61, "dbg hexdump %H"},
#ifdef RTE_LIBRTE_SMEM
    {70, "dbg smem"},
#endif
    {80, "dbg break"},
    {90, "dbg memcpy"},
    {91, "dbg memcpy %d %d"},
    {-1, NULL}
};
// clang-format on

static const char *dbg_help[] = {
    "",
    "dbg l2p                          - Dump out internal lcore to port mapping",
    "dbg tx_dbg                       - Enable tx debug output",
    "dbg tx_rate <portlist>           - Show packet rate for all ports",
    "dbg mempool|dump <portlist> <type>    - Dump out the mempool info for a given type",
    "                                   types - rx|tx|range|seq|rate|arp|pcap",
    "dbg pdump <portlist>             - Hex dump the first packet to be sent, single packet mode "
    "only",
    "dbg memzone                      - List all of the current memzones",
    "dbg memseg                       - List all of the current memsegs",
    "dbg hexdump <addr> <len>         - hex dump memory at given address",
#ifdef RTE_LIBRTE_SMEM
    "dbg smem                         - dump out the SMEM structure",
#endif
    "dbg break                        - break into the debugger",
    "dbg memcpy [loop-cnt KBytes]     - run a memcpy test",
    CLI_HELP_PAUSE,
    NULL};

static void
rte_memcpy_perf(unsigned int cnt, unsigned int kb, int flag)
{
    char *buf[2], *src, *dst;
    uint64_t start_time, total_time;
    uint64_t total_bits, bits_per_tick;
    unsigned int i;

    kb *= 1024;

    buf[0] = malloc(kb + RTE_CACHE_LINE_SIZE);
    buf[1] = malloc(kb + RTE_CACHE_LINE_SIZE);

    src = RTE_PTR_ALIGN(buf[0], RTE_CACHE_LINE_SIZE);
    dst = RTE_PTR_ALIGN(buf[1], RTE_CACHE_LINE_SIZE);

    start_time = rte_get_tsc_cycles();
    for (i = 0; i < cnt; i++) {
        if (flag)
            rte_memcpy(dst, src, kb);
        else
            memcpy(dst, src, kb);
    }
    total_time = rte_get_tsc_cycles() - start_time;

    total_bits = ((uint64_t)cnt * (uint64_t)kb) * 8L;

    bits_per_tick = total_bits / total_time;

    free(buf[0]);
    free(buf[1]);

#define MEGA (uint64_t)(1024 * 1024)
    printf("%3d Kbytes for %8d loops, ", (kb / 1024), cnt);
    printf("%3ld bits/tick, ", bits_per_tick);
    printf("%6ld Mbits/sec with %s\n", (bits_per_tick * pktgen_get_timer_hz()) / MEGA,
           (flag) ? "rte_memcpy" : "memcpy");
}

static int
dbg_cmd(int argc, char **argv)
{
    struct cli_map *m;
    portlist_t portlist;
    unsigned int len, cnt;
    const void *addr;

    m = cli_mapping(dbg_map, argc, argv);
    if (!m)
        return cli_cmd_error("Debug invalid command", "Debug", argc, argv);

    len = 32;
    cnt = 100000;
    switch (m->index) {
    case 10:
        pktgen_l2p_dump();
        break;
    case 20:
        if ((pktgen.flags & TX_DEBUG_FLAG) == 0)
            pktgen.flags |= TX_DEBUG_FLAG;
        else
            pktgen.flags &= ~TX_DEBUG_FLAG;
        pktgen_clear_display();
        break;
    case 21:
        portlist_parse(argv[2], pktgen.nb_ports, &portlist);
        foreach_port(portlist, debug_tx_rate(info));
        break;
    case 30:
        portlist_parse(argv[2], pktgen.nb_ports, &portlist);
        foreach_port(portlist, debug_mempool_dump(info, argv[3]));
        break;
    case 40:
        portlist_parse(argv[2], pktgen.nb_ports, &portlist);
        foreach_port(portlist, debug_pdump(info));
        pktgen_update_display();
        break;
    case 50:
        rte_memzone_dump(stdout);
        break;
    case 51:
        rte_dump_physmem_layout(stdout);
        break;
    case 60:
    case 61:
        addr = (void *)(uintptr_t)strtoull(argv[2], NULL, 0);
        if (argc == 3)
            len = 64;
        else
            len = strtoul(argv[3], NULL, 0);
        rte_hexdump(stdout, "", addr, len);
        break;
#ifdef RTE_LIBRTE_SMEM
    case 70:
        rte_smem_list_dump(stdout);
        break;
#endif
    case 80:
        kill(0, SIGINT);
        break;
    case 91:
        cnt = atoi(argv[2]);
        len = atoi(argv[3]);
        /*FALLTHRU*/
    case 90:
        rte_memcpy_perf(cnt, len, 0);
        rte_memcpy_perf(cnt, len, 1);
        break;
    default:
        return cli_cmd_error("Debug invalid command", "Debug", argc, argv);
    }
    return 0;
}

/**
 *
 * Set a sequence config for given port and slot.
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
    char *eth   = argv[9];
    int seqnum  = atoi(argv[1]);
    portlist_t portlist;
    struct pg_ipaddr dst, src;
    struct rte_ether_addr dmac, smac;
    uint32_t teid;

    if ((proto[0] == 'i') && (eth[3] == '6')) {
        cli_printf("Must use IPv4 with ICMP type packets\n");
        return -1;
    }

    if (seqnum >= NUM_SEQ_PKTS) {
        cli_printf("sequence number too large\n");
        return -1;
    }

    teid = (argc == 14) ? strtoul(argv[13], NULL, 10) : 0;
    p    = strchr(argv[5], '/'); /* remove subnet if found */
    if (p)
        *p = '\0';
    _atoip(argv[5], 0, &dst, sizeof(dst));
    p = strchr(argv[6], '/');
    if (!p) {
        cli_printf(
            "src IP address should contain subnet value, default /32 for IPv4, /128 for IPv6\n");
    }
    _atoip(argv[6], PG_IPADDR_NETWORK, &src, sizeof(src));
    portlist_parse(argv[2], pktgen.nb_ports, &portlist);
    if (pg_ether_aton(argv[3], &dmac) == NULL) {
        cli_printf("invalid MAC string (%s)\n", argv[3]);
        return -1;
    }
    if (pg_ether_aton(argv[4], &smac) == NULL) {
        cli_printf("invalid MAC string (%s)\n", argv[4]);
        return -1;
    }
    foreach_port(portlist, pktgen_set_seq(info, seqnum, &dmac, &smac, &dst, &src, atoi(argv[7]),
                                          atoi(argv[8]), eth[3], proto[0], atoi(argv[11]),
                                          atoi(argv[12]), teid));

    pktgen_update_display();
    return 0;
}

/**
 *
 * Set a sequence config for given port and slot.
 *
 * DESCRIPTION
 * Set up the sequence packets for a given port and slot.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 * 	"%|seq|sequence %d %P dst %m src %m dst %4 src %4 sport %d dport %d %|ipv4|ipv6 %|udp|tcp|icmp
 * vlan %d size %d teid %d"
 */

static int
seq_2_set_cmd(int argc __rte_unused, char **argv)
{
    char *proto = argv[16], *p;
    char *eth   = argv[15];
    int seqnum  = atoi(argv[1]);
    portlist_t portlist;
    struct pg_ipaddr dst, src;
    struct rte_ether_addr dmac, smac;
    uint32_t teid;

    if ((proto[0] == 'i') && (eth[3] == '6')) {
        cli_printf("Must use IPv4 with ICMP type packets\n");
        return -1;
    }

    if (seqnum >= NUM_SEQ_PKTS) {
        cli_printf("Sequence number too large\n");
        return -1;
    }

    teid = (argc == 23) ? strtoul(argv[22], NULL, 10) : 0;
    p    = strchr(argv[8], '/'); /* remove subnet if found */
    if (p)
        *p = '\0';
    _atoip(argv[8], 0, &dst, sizeof(dst));
    p = strchr(argv[10], '/');
    if (p == NULL) {
        cli_printf(
            "src IP address should contain subnet value, default /32 for IPv4, /128 for IPv6\n");
    }
    _atoip(argv[10], PG_IPADDR_NETWORK, &src, sizeof(src));
    portlist_parse(argv[2], pktgen.nb_ports, &portlist);
    if (pg_ether_aton(argv[4], &dmac) == NULL) {
        cli_printf("invalid MAC string (%s)\n", argv[4]);
        return -1;
    }
    if (pg_ether_aton(argv[6], &smac) == NULL) {
        cli_printf("invalid MAC string (%s)\n", argv[6]);
        return -1;
    }
    foreach_port(portlist, pktgen_set_seq(info, seqnum, &dmac, &smac, &dst, &src, atoi(argv[12]),
                                          atoi(argv[14]), eth[3], proto[0], atoi(argv[18]),
                                          atoi(argv[20]), teid));

    pktgen_update_display();
    return 0;
}

/**
 *
 * Set a sequence config for given port and slot.
 *
 * DESCRIPTION
 * Set up the sequence packets for a given port and slot.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 * 	"%|seq|sequence %d %P cos %d tos %d"
 */

static int
seq_3_set_cmd(int argc __rte_unused, char **argv)
{
    int seqnum = atoi(argv[1]);
    portlist_t portlist;
    uint32_t cos, tos;

    if (seqnum >= NUM_SEQ_PKTS) {
        cli_printf("Sequence number too large\n");
        return -1;
    }

    cos = strtoul(argv[4], NULL, 10);
    tos = strtoul(argv[6], NULL, 10);

    portlist_parse(argv[2], pktgen.nb_ports, &portlist);

    foreach_port(portlist, pktgen_set_cos_tos_seq(info, seqnum, cos, tos));

    pktgen_update_display();
    return 0;
}

/**
 *
 * Set a sequence config for given port and slot.
 *
 * DESCRIPTION
 * Set up the sequence packets for a given port and slot.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 * 	"%|seq|sequence %d %P vxlan %d gid %d vid %d"
 */

static int
seq_4_set_cmd(int argc __rte_unused, char **argv)
{
    int seqnum = atoi(argv[1]);
    portlist_t portlist;
    uint32_t flag, gid, vid;

    if (seqnum >= NUM_SEQ_PKTS) {
        cli_printf("Sequence number too large\n");
        return -1;
    }

    flag = strtoul(argv[4], NULL, 0);
    gid  = strtoul(argv[6], NULL, 10);
    vid  = strtoul(argv[8], NULL, 10);

    portlist_parse(argv[2], pktgen.nb_ports, &portlist);

    foreach_port(portlist, pktgen_set_vxlan_seq(info, seqnum, flag, gid, vid));

    pktgen_update_display();
    return 0;
}

// clang-format off
static struct cli_map seq_map[] = {
    {10, "%|seq|sequence %d %P %m %m %4 %4 %d %d %|ipv4|ipv6 %|udp|tcp|icmp %d %d"},
    {11, "%|seq|sequence %d %P %m %m %4 %4 %d %d %|ipv4|ipv6 %|udp|tcp|icmp %d %d %d"},
    {12, "%|seq|sequence %d %P dst %m src %m dst %4 src %4 sport %d dport %d %|ipv4|ipv6 "
         "%|udp|tcp|icmp vlan %d size %d"},
    {13, "%|seq|sequence %d %P dst %m src %m dst %4 src %4 sport %d dport %d %|ipv4|ipv6 "
         "%|udp|tcp|icmp vlan %d size %d teid %d"},
    {15, "%|seq|sequence %d %P cos %d tos %d"},
    {16, "%|seq|sequence %d %P vxlan %d gid %d vid %d"},
    {17, "%|seq|sequence %d %P vxlan %h gid %d vid %d"},
    {-1, NULL}
};
// clang-format on

static const char *seq_help[] = {
    "",
    "sequence <seq#> <portlist> dst <Mac> src <Mac> dst <IP> src <IP> sport <val> dport <val> "
    "ipv4|ipv6 udp|tcp|icmp vlan <val> size <val> [teid <val>]",
    "sequence <seq#> <portlist> <dst-Mac> <src-Mac> <dst-IP> <src-IP> <sport> <dport> ipv4|ipv6 "
    "udp|tcp|icmp <vlanid> <pktsize> [<teid>]",
    "sequence <seq#> <portlist> cos <cos> tos <tos>",
    "sequence <seq#> <portlist> vxlan <flags> gid <group_id> vid <vxlan_id>",
    "                                   - Set the sequence packet information, make sure the "
    "src-IP",
    "                                     has the netmask value eg 1.2.3.4/24",
    CLI_HELP_PAUSE,
    NULL};

static int
seq_cmd(int argc, char **argv)
{
    struct cli_map *m;

    m = cli_mapping(seq_map, argc, argv);
    if (!m)
        return cli_cmd_error("Sequence invalid command", "Sequence", argc, argv);

    switch (m->index) {
    case 10:
    case 11:
        seq_1_set_cmd(argc, argv);
        break;
    case 12:
    case 13:
        seq_2_set_cmd(argc, argv);
        break;
    case 15:
        seq_3_set_cmd(argc, argv);
        break;
    case 16:
    case 17:
        seq_4_set_cmd(argc, argv);
        break;
    default:
        return cli_cmd_error("Sequence invalid command", "Sequence", argc, argv);
    }
    return 0;
}

#ifdef LUA_ENABLED
/**
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
    lua_State *L = pktgen.ld->L;

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

/**
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
    lua_State *L = pktgen.ld->L;
    char buff[1024], *p;
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

    sz = sizeof(buff);
    memset(buff, 0, sz);
    sz--; /* Make sure a NULL is at the end of the string */
    n = 0;
    for (i = 1, p = buff; i < argc; i++) {
        if ((strlen(argv[i]) + 1) > (sz - n)) {
            cli_printf("Input line too long > %ld bytes\n", sizeof(buff));
            return -1;
        }
        n += snprintf(&p[n], sz - n, "%s ", argv[i]);
    }

    if (luaL_dostring(L, buff) != 0)
        pktgen_log_error("%s", lua_tostring(L, -1));
    return 0;
}
#endif

// clang-format off
static struct cli_map misc_map[] = {
    {10, "clear %P stats"},
    {20, "geometry"},
    {30, "load %s"},

#ifdef LUA_ENABLED
    {40, "script %l"},
    {50, "lua %l"},
#endif
    {60, "save %s"},
    {70, "redisplay"},
    {100, "reset %P"},
    {110, "restart %P"},
    {130, "port %d"},
    {135, "ports per page %d"},
    {140, "ping4 %P"},
#ifdef INCLUDE_PING6
    {141, "ping6 %P"},
#endif
    {-1, NULL}
};
// clang-format on
static const char *misc_help[] = {
    "",
    "save <path-to-file>                - Save a configuration file using the filename",
    "load <path-to-file>                - Load a command/script file from the given path",

#ifdef LUA_ENABLED
    "script <filename>                  - Execute the Lua script code in file (www.lua.org).",
    "lua 'lua string'                   - Execute the Lua code in the string needs quotes",
#endif
    "geometry                           - Show the display geometry Columns by Rows (ColxRow)",
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
    NULL};

static int
misc_cmd(int argc, char **argv)
{
    struct cli_map *m;
    portlist_t portlist;
    uint16_t rows, cols;
    int paused;

    m = cli_mapping(misc_map, argc, argv);
    if (!m)
        return cli_cmd_error("Misc invalid command", "Misc", argc, argv);

    switch (m->index) {
    case 10:
        portlist_parse(argv[1], pktgen.nb_ports, &portlist);
        foreach_port(portlist, pktgen_clear_stats(info));
        pktgen_clear_display();
        break;
    case 20:
        pktgen_display_get_geometry(&rows, &cols);
        break;
    case 30:
        paused = scrn_is_paused();
        scrn_pause();
        if (cli_execute_cmdfile(argv[1]))
            cli_printf("load command failed for %s\n", argv[1]);
        if (paused)
            pktgen_force_update();
        else
            scrn_resume();
        break;
#ifdef LUA_ENABLED
    case 40:
        script_cmd(argc, argv);
        break;
    case 50:
        exec_lua_cmd(argc, argv);
        break;
#endif
    case 60:
        pktgen_save(argv[1]);
        break;
    case 70:
        pktgen_clear_display();
        break;
    case 100:
        portlist_parse(argv[1], pktgen.nb_ports, &portlist);
        foreach_port(portlist, pktgen_reset(info));
        break;
    case 110:
        portlist_parse(argv[1], pktgen.nb_ports, &portlist);
        foreach_port(portlist, pktgen_port_restart(info));
        break;
    case 120:
    case 130:
        pktgen_set_port_number((uint16_t)atoi(argv[1]));
        break;
    case 140:
        portlist_parse(argv[1], pktgen.nb_ports, &portlist);
        foreach_port(portlist, pktgen_ping4(info));
        pktgen_force_update();
        break;
#ifdef INCLUDE_PING6
    case 141:
        portlist_parse(argv[1], pktgen.nb_ports, &portlist);
        foreach_port(portlist, pktgen_ping6(info));
        pktgen_update_display();
        break;
#endif
    default:
        return cli_cmd_error("Misc invalid command", "Misc", argc, argv);
    }
    return 0;
}

// clang-format off
static struct cli_map page_map[] = {
    {10, "page %d"},
    {11, "page %|main|range|config|cfg|pcap|cpu|next|sequence|seq|rnd|"
         "log|latency|lat|stats|xstats|rate|rate-pacing"},
    {-1, NULL}
};
// clang-format on

static const char *page_help[] = {
    "",
    "page [0-7]                         - Show the port pages or configuration or sequence page",
    "page main                          - Display page zero",
    "page range                         - Display the range packet page",
    "page config | cfg                  - Display the configuration page",
    "page pcap                          - Display the pcap page",
    "page cpu                           - Display some information about the CPU system",
    "page next                          - Display next page of PCAP packets.",
    "page sequence | seq                - sequence will display a set of packets for a given port",
    "                                     Note: use the 'port <number>' to display a new port "
    "sequence",
    "page rnd                           - Display the random bitfields to packets for a given port",
    "                                     Note: use the 'port <number>' to display a new port "
    "sequence",
    "page log                           - Display the log messages page",
    "page latency | lat                 - Display the latency page",
    "page stats                         - Display physical ports stats for all ports",
    "page xstats                        - Display port XSTATS values",
    "page rate                          - Display Rate Pacing values",
    CLI_HELP_PAUSE,
    NULL};

static int
page_cmd(int argc, char **argv)
{
    struct cli_map *m;

    m = cli_mapping(page_map, argc, argv);
    if (!m)
        return cli_cmd_error("Page invalid command", "Page", argc, argv);

    switch (m->index) {
    case 10:
    case 11:
        pktgen_set_page(argv[1]);
        break;
    default:
        return cli_cmd_error("Page invalid command", "Page", argc, argv);
    }
    return 0;
}

// clang-format off
static struct cli_map plugin_map[] = {
    {10, "plugin"},
    {20, "plugin load %s"},
    {21, "plugin load %s %s"},
    {30, "plugin %|rm|del|delete %s"},
    {-1, NULL}
};
// clang-format on

static const char *plugin_help[] = {
    "",
    "plugin                             - Show the plugins currently installed",
    "plugin load <filename>             - Load a plugin file",
    "plugin load <filename> <path>      - Load a plugin file at path",
    "plugin rm|delete <plugin>          - Remove or delete a plugin",
    CLI_HELP_PAUSE,
    NULL};

static int
plugin_cmd(int argc, char **argv)
{
    struct cli_map *m;
    int inst;

    m = cli_mapping(plugin_map, argc, argv);
    if (!m)
        return cli_cmd_error("Plugin invalid command", "Plugin", argc, argv);

    switch (m->index) {
    case 10:
        plugin_dump(stdout);
        break;
    case 20:
    case 21:
        if ((inst = plugin_create(argv[2], argv[3])) < 0) {
            printf("Plugin not loaded %s, %s\n", argv[2], argv[3]);
            return -1;
        }

        if (plugin_start(inst, NULL), 0) {
            plugin_destroy(inst);
            return -1;
        }
        break;
    case 30:
        inst = plugin_find_by_name(argv[2]);
        if (inst < 0)
            return -1;
        plugin_stop(inst);
        plugin_destroy(inst);
        break;
    default:
        return cli_cmd_error("Plugin invalid command", "Plugin", argc, argv);
    }
    return 0;
}

#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
// clang-format off
static struct cli_map bonding_map[] = {
    {10, "bonding %P show"},
    {20, "bonding show"},
    {-1, NULL}
};
// clang-format on

static const char *bonding_help[] = {
    "", "bonding <portlist> show          - Show the bonding configuration for <portlist>",
    "bonding show                     - Show all bonding configurations", CLI_HELP_PAUSE, NULL};

static void
bonding_dump(port_info_t *info)
{
    uint16_t pid;

    RTE_ETH_FOREACH_DEV(pid)
    {
        struct rte_eth_bond_8023ad_conf conf;

        if (info->pid != pid)
            continue;

        if (rte_eth_bond_8023ad_conf_get(pid, &conf) < 0)
            continue;

        printf("Port %d information:\n", pid);
        show_bonding_mode(info);
        printf("\n");
    }
}

static int
bonding_cmd(int argc, char **argv)
{
    struct cli_map *m;
    portlist_t portlist;

    m = cli_mapping(bonding_map, argc, argv);
    if (!m)
        return cli_cmd_error("bonding invalid command", "Bonding", argc, argv);

    switch (m->index) {
    case 10:
        portlist_parse(argv[1], pktgen.nb_ports, &portlist);
        foreach_port(portlist, bonding_dump(info));
        break;
    case 20:
        portlist = 0xFFFFFFFF;
        foreach_port(portlist, bonding_dump(info));
        break;
    default:
        return cli_cmd_error("Bonding invalid command", "Bonding", argc, argv);
    }
    return 0;
}
#endif

#define rate_types      \
    "count|"   /*  0 */ \
    "size|"    /*  1 */ \
    "burst|"   /*  2 */ \
    "sport|"   /*  3 */ \
    "dport|"   /*  4 */ \
    "ttl|"     /*  5 */ \
    "txburst|" /*  6 */ \
    "rxburst"  /*  7 */

// clang-format off
static struct cli_map rate_map[] = {
    {10, "rate %P %|" rate_types " %d"},
    {20, "rate %P type %|arp|ipv4|ipv6|ip4|ip6|vlan"},
    {21, "rate %P proto %|udp|tcp|icmp"},
    {22, "rate %P src mac %m"},
    {23, "rate %P dst mac %m"},
    {30, "rate %P src ip %4"},
    {31, "rate %P dst ip %4"},
    {32, "rate %P src ip %6"},
    {33, "rate %P dst ip %6"},
    {34, "rate %P tcp flag set %|urg|ack|psh|rst|syn|fin|all"},
    {35, "rate %P tcp flag clr %|urg|ack|psh|rst|syn|fin|all"},
    {36, "rate %P tcp seq %u"},
    {37, "rate %P tcp ack %u"},
    {40, "rate %P fps %d"},
    {45, "rate %P lines %d"},
    {46, "rate %P pixels %d"},
    {50, "rate %P color bits %d"},
    {60, "rate %P payload size %d"},
    {70, "rate %P overhead %d"},
    {-1, NULL}
};
// clang-format on

static const char *rate_help[] = {
    "",
    "rate <portlist> count <value>        - number of packets to transmit",
    "rate <portlist> size <value>         - size of the packet to transmit",
    "rate <portlist> rate <percent>       - Packet rate in percentage",
    "rate <portlist> txburst|burst <value> - number of packets in a Tx burst",
    "rate <portlist> rxburst <value>      - number of packets in a Rx burst",
    "rate <portlist> sport <value>        - Source port number for UDP/TCP",
    "rate <portlist> dport <value>        - Destination port number for UDP/TCP",
    "rate <portlist> ttl <value>          - Set the TTL value for the single port more",
    "rate <portlist> src|dst mac <addr>   - Set MAC addresses 00:11:22:33:44:55 or 0011:2233:4455 "
    "format",
    "rate <portlist> type ipv4|ipv6|vlan|arp - Set the packet type to IPv4 or IPv6 or VLAN",
    "rate <portlist> proto udp|tcp|icmp   - Set the packet protocol to UDP or TCP or ICMP per port",
    "rate <portlist> [src|dst] ip ipaddr  - Set IP addresses, Source must include network mask "
    "e.g. 10.1.2.3/24",

    "rate <portlist> tcp flag set urg|ack|psh|rst|syn|fin|all - Set a TCP flag",
    "rate <portlist> tcp flag clr urg|ack|psh|rst|syn|fin|all - Clear a TCP flag",
    "rate <portlist> tcp seq <sequence> - Set the TCP sequence number",
    "rate <portlist> tcp ack <acknowledge> - Set the TCP acknowledge number",

    "rate <portlist> fps <value>          - Set the frame per second value e.g. 60fps",
    "rate <portlist> lines <value>        - Set the number of video lines, e.g. 720",
    "rate <portlist> pixels <value>       - Set the number of pixels per line, e.g. 1280",
    "rate <portlist> color bits <value>   - Set the color bit size 8, 16, 24, ...",
    "rate <portlist> payload size <value> - Set the payload size",
    "rate <portlist> overhead <value>     - Set the packet overhead + payload = total packet size",
    CLI_HELP_PAUSE,
    NULL};

static int
rate_cmd(int argc, char **argv)
{
    struct cli_map *m;
    char *what, *p;
    struct rte_ether_addr mac;
    struct pg_ipaddr ip;
    portlist_t portlist;
    int value, n, ip_ver;

    m = cli_mapping(rate_map, argc, argv);
    if (!m)
        return cli_cmd_error("Rate invalid command", "Rate", argc, argv);

    portlist_parse(argv[1], pktgen.nb_ports, &portlist);

    what  = argv[2];
    value = atoi(argv[3]);

    switch (m->index) {
    case 10:
        n = cli_map_list_search(m->fmt, argv[2], 2);
        foreach_port(portlist, _do(switch (n) {
                         case 0:
                             rate_set_tx_count(info, value);
                             break;
                         case 1:
                             rate_set_pkt_size(info, valid_pkt_size(argv[3]));
                             break;
                         case 2:
                             rate_set_tx_burst(info, value);
                             break;
                         case 3:
                             rate_set_port_value(info, what[0], value);
                             break;
                         case 4:
                             rate_set_port_value(info, what[0], value);
                             break;
                         case 5:
                             rate_set_ttl_value(info, value);
                             break;
                         case 6:
                             rate_set_tx_burst(info, value);
                             break;
                         case 7:
                             rate_set_rx_burst(info, value);
                             break;
                         default:
                             return cli_cmd_error("Set rate command is invalid", "Rate", argc,
                                                  argv);
                     }));
        break;
    case 20:
        foreach_port(portlist, rate_set_pkt_type(info, argv[3]));
        break;
    case 21:
        foreach_port(portlist, rate_set_proto(info, argv[3]));
        break;
    case 22:
        if (pg_ether_aton(argv[4], &mac) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[4]);
            break;
        }
        foreach_port(portlist, rate_set_src_mac(info, &mac));
        break;
    case 23:
        if (pg_ether_aton(argv[4], &mac) == NULL) {
            cli_printf("Failed to parse MAC address from (%s)\n", argv[4]);
            break;
        }
        foreach_port(portlist, rate_set_dst_mac(info, &mac));
        break;
    case 30:
        p = strchr(argv[4], '/');
        if (!p)
            cli_printf("src IP address should contain /NN subnet value, default /32\n");

        ip_ver = _atoip(argv[4], PG_IPADDR_V4 | PG_IPADDR_NETWORK, &ip, sizeof(ip));
        foreach_port(portlist, rate_set_ipaddr(info, 's', &ip, ip_ver));
        break;
    case 31:
        /* Remove the /XX mask value if supplied */
        p = strchr(argv[4], '/');
        if (p) {
            cli_printf("Subnet mask not required, removing subnet mask value\n");
            *p = '\0';
        }
        ip_ver = _atoip(argv[4], PG_IPADDR_V4, &ip, sizeof(ip));
        foreach_port(portlist, rate_set_ipaddr(info, 'd', &ip, ip_ver));
        break;
    case 32:
        p = strchr(argv[4], '/');
        if (!p)
            cli_printf("src IP address should contain /NN subnet value, default /128\n");

        ip_ver = _atoip(argv[4], PG_IPADDR_V6 | PG_IPADDR_NETWORK, &ip, sizeof(ip));
        foreach_port(portlist, rate_set_ipaddr(info, 's', &ip, ip_ver));
        break;
    case 33:
        /* Remove the /XX mask value if supplied */
        p = strchr(argv[4], '/');
        if (p) {
            cli_printf("Subnet mask not required, removing subnet mask value\n");
            *p = '\0';
        }
        ip_ver = _atoip(argv[4], PG_IPADDR_V6, &ip, sizeof(ip));
        foreach_port(portlist, rate_set_ipaddr(info, 'd', &ip, ip_ver));
        break;
    case 34:
        foreach_port(portlist, rate_set_tcp_flag_set(info, argv[5]));
        break;
    case 35:
        foreach_port(portlist, rate_set_tcp_flag_clr(info, argv[5]));
        break;
    case 36:
        foreach_port(portlist, rate_set_tcp_seq(info, atoi(argv[4])));
        break;
    case 37:
        foreach_port(portlist, rate_set_tcp_ack(info, atoi(argv[4])));
        break;
    case 40: /* fps */
        foreach_port(portlist, rate_set_value(info, "fps", atoi(argv[3])));
        break;
    case 45: /* lines */
        foreach_port(portlist, rate_set_value(info, "lines", atoi(argv[3])));
        break;
    case 46: /* pixels */
        foreach_port(portlist, rate_set_value(info, "pixels", atoi(argv[3])));
        break;
    case 50: /* color bits */
        foreach_port(portlist, rate_set_value(info, "color", atoi(argv[4])));
        break;
    case 60: /* payload size */
        foreach_port(portlist, rate_set_value(info, "payload", atoi(argv[4])));
        break;
    case 70: /* overhead */
        foreach_port(portlist, rate_set_value(info, "overhead", atoi(argv[3])));
        break;
    default:
        return cli_cmd_error("Rate invalid command", "Rate", argc, argv);
    }

    pktgen_update_display();
    return 0;
}

// clang-format off
static struct cli_map latency_map[] = {
    {10, "latency %P rate %u"},
    {20, "latency %P entropy %u"},
    {-1, NULL}
};

static const char *latency_help[] = {
    "",
    "latency <portlist> rate <value>      - Rate in milli-seonds to send a latency packet",
    "latency <portlist> entropy <value>   - Entropy value to adjust the src-port by (SPORT + (i % N)) (default: 1)",
    "                                       e.eg. latency 0 entropy 16",
    CLI_HELP_PAUSE,
    NULL};
// clang-format on

static int
latency_cmd(int argc, char **argv)
{
    struct cli_map *m;
    portlist_t portlist;
    int value;

    m = cli_mapping(latency_map, argc, argv);
    if (!m)
        return cli_cmd_error("Latency invalid command", "Latency", argc, argv);

    portlist_parse(argv[1], pktgen.nb_ports, &portlist);

    value = atoi(argv[3]);

    switch (m->index) {
    case 10:
        foreach_port(portlist, latency_set_rate(info, value));
        break;
    case 20:
        foreach_port(portlist, latency_set_entropy(info, (uint16_t)value));
        break;
    default:
        return cli_cmd_error("Latency invalid command", "Latency", argc, argv);
    }

    pktgen_update_display();
    return 0;
}

/**********************************************************/
/**********************************************************/
/****** CONTEXT (list of instruction) */

static int help_cmd(int argc, char **argv);

static struct cli_tree default_tree[] = {
    c_dir("/pktgen/bin"),
    c_cmd("help", help_cmd, "help command"),

    c_cmd("clear", misc_cmd, "clear stats, ..."),
    c_alias("clr", "clear all stats", "clear all port stats"),
    c_cmd("geometry", misc_cmd, "show the screen geometry"),
    c_alias("geom", "geometry", "show the screen geometry"),
    c_cmd("load", misc_cmd, "load command file"),
#ifdef LUA_ENABLED
    c_cmd("script", misc_cmd, "run a Lua script"),
    c_cmd("lua", misc_cmd, "execute a Lua string"),
#endif
    c_cmd("save", misc_cmd, "save the current state"),
    c_cmd("redisplay", misc_cmd, "redisplay the screen"),
    c_alias("cls", "redisplay", "redraw screen"),
    c_cmd("reset", misc_cmd, "reset pktgen configuration"),
    c_alias("rst", "reset all", "reset all ports"),
    c_cmd("restart", misc_cmd, "restart port"),
    c_cmd("port", misc_cmd, "Switch between ports"),
    c_cmd("ping4", misc_cmd, "Send a ping packet for IPv4"),
#ifdef INCLUDE_PING6
    c_cmd("ping6", misc_cmd, "Send a ping packet for IPv6"),
#endif

    c_cmd("sequence", seq_cmd, "sequence command"),
    c_alias("seq", "sequence", "sequence command"),

    c_cmd("page", page_cmd, "change page displays"),
    c_cmd("theme", theme_cmd, "Set, save, show the theme"),
    c_cmd("range", range_cmd, "Range commands"),
    c_cmd("enable", en_dis_cmd, "enable features"),
    c_cmd("disable", en_dis_cmd, "disable features"),
    c_cmd("start", start_stop_cmd, "start features"),
    c_cmd("stop", start_stop_cmd, "stop features"),
    c_alias("str", "start all", "start all ports sending packets"),
    c_alias("stp", "stop all", "stop all ports sending packets"),
    c_cmd("pcap", pcap_cmd, "pcap commands"),
    c_cmd("set", set_cmd, "set a number of options"),
    c_cmd("dbg", dbg_cmd, "debug commands"),
    c_cmd("plugin", plugin_cmd, "Plugin a shared object file"),
#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
    c_cmd("bonding", bonding_cmd, "Bonding commands"),
#endif
    c_cmd("rate", rate_cmd, "Rate setup commands"),
    c_cmd("latency", latency_cmd, "Latency setup commands"),

    c_alias("on", "enable screen", "Enable screen updates"),
    c_alias("off", "disable screen", "Disable screen updates"),

    c_end()};

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
    cli_help_add("Debug", dbg_map, dbg_help);
    cli_help_add("Misc", misc_map, misc_help);
    cli_help_add("Theme", theme_map, theme_help);
    cli_help_add("Plugin", plugin_map, plugin_help);
    cli_help_add("Rate", rate_map, rate_help);
    cli_help_add("Latency", latency_map, latency_help);
#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
    cli_help_add("Bonding", bonding_map, bonding_help);
#endif
    cli_help_add("Status", NULL, status_help);

    /* Make sure the pktgen commands are executable in search path */
    if (cli_add_bin_path("/pktgen/bin"))
        return -1;

    return 0;
}

static int
my_prompt(int cont __rte_unused)
{
    int nb;

    pktgen_display_set_color("pktgen.prompt");
    nb = cli_printf("Pktgen:%s> ", cli_path_string(NULL, NULL));
    pktgen_display_set_color("stats.stat.values");

    return nb;
}

int
pktgen_cli_create(void)
{
    int ret = -1;

    if (!cli_create()) {
        if (!cli_setup_with_tree(init_tree)) {
            cli_set_prompt(my_prompt);
            ret = 0;
        }
    }
    return ret;
}

/**
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
