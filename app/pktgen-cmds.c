/*-
 * Copyright(c) <2010-2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <string.h>
#include <sys/stat.h>

#include <lua_config.h>

#include "pktgen.h"

#include "pktgen-cmds.h"

#include "pktgen-display.h"

#include <pg_delay.h>

#include <rte_net.h>
#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
#include <rte_eth_bond.h>
#include <rte_eth_bond_8023ad.h>
#endif

static char hash_line[] = "#######################################################################";
#define _cp(s) (strcmp(str, s) == 0)

static char *
convert_bitfield(bf_spec_t *bf)
{
    uint32_t mask;
    char *p;
    uint32_t i;
    static char rnd_bitmask[33];

    memset(rnd_bitmask, 0, sizeof(rnd_bitmask));
    memset(rnd_bitmask, '.', sizeof(rnd_bitmask) - 1);

    p = rnd_bitmask;
    for (i = 0; i < MAX_BITFIELD_SIZE; i++) {
        mask = (uint32_t)(1 << (MAX_BITFIELD_SIZE - i - 1));

        /* Need to check rndMask before andMask: for random bits, the
         * andMask is also 0. */
        p[i] = ((ntohl(bf->rndMask) & mask) != 0)   ? 'X'
               : ((ntohl(bf->andMask) & mask) == 0) ? '0'
               : ((ntohl(bf->orMask) & mask) != 0)  ? '1'
                                                    : '.';
    }

    return rnd_bitmask;
}

static void
pktgen_set_tx_update(port_info_t *info)
{
    uint16_t q;

    for (q = 0; q < get_port_txcnt(pktgen.l2p, info->pid); q++)
        pktgen_set_q_flags(info, q, CLEAR_FAST_ALLOC_FLAG);
}

/**
 * pktgen_save - Save a configuration as a startup script
 *
 * DESCRIPTION
 * Save a configuration as a startup script
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static int
pktgen_script_save(char *path)
{
    port_info_t *info;
    pkt_seq_t *pkt;
    range_info_t *range;
    rate_info_t *rate;
    latency_t *lat;
    uint32_t flags;
    char buff[64];
    FILE *fd;
    int i, j;
    uint64_t lcore;
    struct rte_ether_addr eaddr;

    fd = fopen(path, "w");
    if (fd == NULL)
        return -1;

    for (i = 0, lcore = 0; i < RTE_MAX_LCORE; i++)
        if (rte_lcore_is_enabled(i))
            lcore |= (1 << i);

    fprintf(fd, "#\n# %s\n", pktgen_version());
    fprintf(fd, "# %s, %s\n\n", copyright_msg(), powered_by());

    /* TODO: Determine DPDK arguments for rank and memory, default for now. */
    fprintf(fd, "# Command line arguments: (DPDK args are defaults)\n");
    fprintf(fd, "# %s -c %" PRIx64 " -n 3 -m 512 --proc-type %s -- ", pktgen.argv[0], lcore,
            (rte_eal_process_type() == RTE_PROC_PRIMARY) ? "primary" : "secondary");
    for (i = 1; i < pktgen.argc; i++)
        fprintf(fd, "%s ", pktgen.argv[i]);
    fprintf(fd, "\n\n%s\n", hash_line);

    fprintf(fd, "# Pktgen Configuration script information:\n");
    fprintf(fd, "#   GUI socket is %s\n",
            (pktgen.flags & ENABLE_GUI_FLAG) ? "Enabled" : "Not Enabled");
    fprintf(fd, "#   Flags %08x\n", pktgen.flags);
    fprintf(fd, "#   Number of ports: %d\n", pktgen.nb_ports);
    fprintf(fd, "#   Number ports per page: %d\n", pktgen.nb_ports_per_page);
    fprintf(fd, "#   Number descriptors: RX %d TX: %d\n", pktgen.nb_rxd, pktgen.nb_txd);
    fprintf(fd, "#   Promiscuous mode is %s\n\n",
            (pktgen.flags & PROMISCUOUS_ON_FLAG) ? "Enabled" : "Disabled");

    fprintf(fd, "\n# Global configuration:\n");
    uint16_t rows, cols;
    pktgen_display_get_geometry(&rows, &cols);
    fprintf(fd, "#   geometry %dx%d\n", cols, rows);
    fprintf(fd, "%s mac_from_arp\n\n", (pktgen.flags & MAC_FROM_ARP_FLAG) ? "enable" : "disable");

    for (i = 0; i < pktgen.nb_ports; i++) {
        info  = &pktgen.info[i];
        pkt   = &info->seq_pkt[SINGLE_PKT];
        range = &info->range;
        rate  = &info->rate;

        if (info->tx_burst == 0)
            continue;

        lat = &info->latency;

        fprintf(fd, "######################### Port %2d ##################################\n", i);
        if (rte_atomic64_read(&info->transmit_count) == 0)
            strcpy(buff, "Forever");
        else
            snprintf(buff, sizeof(buff), "%" PRIu64, rte_atomic64_read(&info->transmit_count));
        fprintf(fd, "#\n");
        flags = rte_atomic32_read(&info->port_flags);
        fprintf(fd, "# Port: %2d, Burst (Rx/Tx):%3d/%3d, Rate:%g%%, Flags:%08x, TX Count:%s\n",
                info->pid, info->rx_burst, info->tx_burst, info->tx_rate, flags, buff);
        fprintf(fd, "#           Sequence count:%d, Prime:%d VLAN ID:%04x, ", info->seqCnt,
                info->prime_cnt, info->vlanid);
        pktgen_link_state(info->pid, buff, sizeof(buff));
        fprintf(fd, "Link: %s\n", buff);

        fprintf(fd, "#\n# Set up the primary port information:\n");
        fprintf(fd, "set %d count %" PRIu64 "\n", info->pid,
                rte_atomic64_read(&info->transmit_count));
        fprintf(fd, "set %d size %d\n", info->pid, pkt->pktSize + RTE_ETHER_CRC_LEN);
        fprintf(fd, "set %d rate %g\n", info->pid, info->tx_rate);
        fprintf(fd, "set %d rxburst %d\n", info->pid, info->rx_burst);
        fprintf(fd, "set %d txburst %d\n", info->pid, info->tx_burst);
        fprintf(fd, "set %d sport %d\n", info->pid, pkt->sport);
        fprintf(fd, "set %d dport %d\n", info->pid, pkt->dport);
        fprintf(fd, "set %d prime %d\n", info->pid, info->prime_cnt);
        fprintf(fd, "set %d type %s\n", i,
                (pkt->ethType == RTE_ETHER_TYPE_IPV4)   ? "ipv4"
                : (pkt->ethType == RTE_ETHER_TYPE_IPV6) ? "ipv6"
                : (pkt->ethType == RTE_ETHER_TYPE_VLAN) ? "vlan"
                : (pkt->ethType == RTE_ETHER_TYPE_ARP)  ? "arp"
                                                        : "unknown");
        fprintf(fd, "set %d proto %s\n", i,
                (pkt->ipProto == PG_IPPROTO_TCP)    ? "tcp"
                : (pkt->ipProto == PG_IPPROTO_ICMP) ? "icmp"
                                                    : "udp");
        fprintf(
            fd, "set %d dst ip %s\n", i,
            (pkt->ethType == RTE_ETHER_TYPE_IPV6)
                ? inet_ntop6(buff, sizeof(buff), pkt->ip_dst_addr.addr.ipv6.s6_addr, PG_PREFIXMAX)
                : inet_ntop4(buff, sizeof(buff), ntohl(pkt->ip_dst_addr.addr.ipv4.s_addr),
                             0xFFFFFFFF));
        fprintf(fd, "set %d src ip %s\n", i,
                (pkt->ethType == RTE_ETHER_TYPE_IPV6)
                    ? inet_ntop6(buff, sizeof(buff), pkt->ip_src_addr.addr.ipv6.s6_addr,
                                 pkt->ip_src_addr.prefixlen)
                    : inet_ntop4(buff, sizeof(buff), ntohl(pkt->ip_src_addr.addr.ipv4.s_addr),
                                 pkt->ip_mask));

        fprintf(fd, "set %d tcp flag clr all\n", i);
        if (pkt->tcp_flags & URG_FLAG)
            fprintf(fd, "set %d tcp flag set %s\n", i, "urg");
        if (pkt->tcp_flags & ACK_FLAG)
            fprintf(fd, "set %d tcp flag set %s\n", i, "ack");
        if (pkt->tcp_flags & PSH_FLAG)
            fprintf(fd, "set %d tcp flag set %s\n", i, "psh");
        if (pkt->tcp_flags & RST_FLAG)
            fprintf(fd, "set %d tcp flag set %s\n", i, "rst");
        if (pkt->tcp_flags & SYN_FLAG)
            fprintf(fd, "set %d tcp flag set %s\n", i, "syn");
        if (pkt->tcp_flags & FIN_FLAG)
            fprintf(fd, "set %d tcp flag set %s\n", i, "fin");

        fprintf(fd, "set %d tcp seq %u\n", i, pkt->tcp_seq);
        fprintf(fd, "set %d tcp ack %u\n", i, pkt->tcp_ack);

        fprintf(fd, "set %d dst mac %s\n", info->pid,
                inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
        fprintf(fd, "set %d src mac %s\n", info->pid,
                inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));
        fprintf(fd, "set %d vlan %d\n\n", i, pkt->vlanid);

        fprintf(fd, "set %d pattern %s\n", i,
                (info->fill_pattern_type == ABC_FILL_PATTERN)    ? "abc"
                : (info->fill_pattern_type == NO_FILL_PATTERN)   ? "none"
                : (info->fill_pattern_type == ZERO_FILL_PATTERN) ? "zero"
                                                                 : "user");
        if ((info->fill_pattern_type == USER_FILL_PATTERN) && strlen(info->user_pattern)) {
            char buff[64];
            memset(buff, 0, sizeof(buff));
            snprintf(buff, sizeof(buff), "%s", info->user_pattern);
            fprintf(fd, "set %d user pattern %s\n", i, buff);
        }
        fprintf(fd, "\n");

        fprintf(fd, "set %d jitter %" PRIu64 "\n", i, lat->jitter_threshold_us);
        fprintf(fd, "%sable %d mpls\n", (flags & SEND_MPLS_LABEL) ? "en" : "dis", i);
        sprintf(buff, "0x%x", pkt->mpls_entry);
        fprintf(fd, "range %d mpls entry %s\n", i, buff);

        fprintf(fd, "%sable %d qinq\n", (flags & SEND_Q_IN_Q_IDS) ? "en" : "dis", i);
        fprintf(fd, "set %d qinqids %d %d\n", i, pkt->qinq_outerid, pkt->qinq_innerid);

        fprintf(fd, "%sable %d gre\n", (flags & SEND_GRE_IPv4_HEADER) ? "en" : "dis", i);
        fprintf(fd, "%sable %d gre_eth\n", (flags & SEND_GRE_ETHER_HEADER) ? "en" : "dis", i);

        fprintf(fd, "%sable %d vxlan\n", (flags & SEND_VXLAN_PACKETS) ? "en" : "dis", i);
        fprintf(fd, "set %d vxlan 0x%x %d %d\n", i, pkt->vni_flags, pkt->group_id, pkt->vxlan_id);

        fprintf(fd, "#\n# Port flag values:\n");
        fprintf(fd, "%sable %d icmp\n", (flags & ICMP_ECHO_ENABLE_FLAG) ? "en" : "dis", i);
        fprintf(fd, "%sable %d pcap\n", (flags & SEND_PCAP_PKTS) ? "en" : "dis", i);
        fprintf(fd, "%sable %d range\n", (flags & SEND_RANGE_PKTS) ? "en" : "dis", i);
        fprintf(fd, "%sable %d latency\n", (flags & ENABLE_LATENCY_PKTS) ? "en" : "dis", i);
        fprintf(fd, "%sable %d process\n", (flags & PROCESS_INPUT_PKTS) ? "en" : "dis", i);
        fprintf(fd, "%sable %d capture\n", (flags & CAPTURE_PKTS) ? "en" : "dis", i);
        fprintf(fd, "%sable %d rx_tap\n", (flags & PROCESS_RX_TAP_PKTS) ? "en" : "dis", i);
        fprintf(fd, "%sable %d tx_tap\n", (flags & PROCESS_TX_TAP_PKTS) ? "en" : "dis", i);
        fprintf(fd, "%sable %d vlan\n", (flags & SEND_VLAN_ID) ? "en" : "dis", i);
        fprintf(fd, "%sable %d rate\n\n", (flags & SEND_RATE_PACKETS) ? "en" : "dis", i);

        fprintf(fd, "#\n# Range packet information:\n");
        fprintf(fd, "range %d src mac start %s\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac, &eaddr)));
        fprintf(fd, "range %d src mac min %s\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac_min, &eaddr)));
        fprintf(fd, "range %d src mac max %s\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac_max, &eaddr)));
        fprintf(fd, "range %d src mac inc %s\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac_inc, &eaddr)));

        fprintf(fd, "\n");
        fprintf(fd, "range %d dst mac start %s\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac, &eaddr)));
        fprintf(fd, "range %d dst mac min %s\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac_min, &eaddr)));
        fprintf(fd, "range %d dst mac max %s\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac_max, &eaddr)));
        fprintf(fd, "range %d dst mac inc %s\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac_inc, &eaddr)));

        fprintf(fd, "\n");
        fprintf(fd, "range %d src ip start %s\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip), 0xFFFFFFFF));
        fprintf(fd, "range %d src ip min %s\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip_min), 0xFFFFFFFF));
        fprintf(fd, "range %d src ip max %s\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip_max), 0xFFFFFFFF));
        fprintf(fd, "range %d src ip inc %s\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip_inc), 0xFFFFFFFF));

        fprintf(fd, "\n");
        fprintf(fd, "range %d dst ip start %s\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip), 0xFFFFFFFF));
        fprintf(fd, "range %d dst ip min %s\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip_min), 0xFFFFFFFF));
        fprintf(fd, "range %d dst ip max %s\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip_max), 0xFFFFFFFF));
        fprintf(fd, "range %d dst ip inc %s\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip_inc), 0xFFFFFFFF));

        fprintf(fd, "\n");
        fprintf(fd, "range %d proto %s\n", i,
                (range->ip_proto == PG_IPPROTO_UDP)    ? "udp"
                : (range->ip_proto == PG_IPPROTO_ICMP) ? "icmp"
                                                       : "tcp");

        fprintf(fd, "\n");
        fprintf(fd, "range %d src port start %d\n", i, range->src_port);
        fprintf(fd, "range %d src port min %d\n", i, range->src_port_min);
        fprintf(fd, "range %d src port max %d\n", i, range->src_port_max);
        fprintf(fd, "range %d src port inc %d\n", i, range->src_port_inc);

        fprintf(fd, "\n");
        fprintf(fd, "range %d dst port start %d\n", i, range->dst_port);
        fprintf(fd, "range %d dst port min %d\n", i, range->dst_port_min);
        fprintf(fd, "range %d dst port max %d\n", i, range->dst_port_max);
        fprintf(fd, "range %d dst port inc %d\n", i, range->dst_port_inc);

        fprintf(fd, "\n");
        fprintf(fd, "range %d tcp flag clr all\n", i);
        if (range->tcp_flags & URG_FLAG)
            fprintf(fd, "range %d tcp flag set %s\n", i, "urg");
        if (range->tcp_flags & ACK_FLAG)
            fprintf(fd, "range %d tcp flag set %s\n", i, "ack");
        if (range->tcp_flags & PSH_FLAG)
            fprintf(fd, "range %d tcp flag set %s\n", i, "psh");
        if (range->tcp_flags & RST_FLAG)
            fprintf(fd, "range %d tcp flag set %s\n", i, "rst");
        if (range->tcp_flags & SYN_FLAG)
            fprintf(fd, "range %d tcp flag set %s\n", i, "syn");
        if (range->tcp_flags & FIN_FLAG)
            fprintf(fd, "range %d tcp flag set %s\n", i, "fin");

        fprintf(fd, "\n");
        fprintf(fd, "range %d tcp seq start %u\n", i, range->tcp_seq);
        fprintf(fd, "range %d tcp seq min %u\n", i, range->tcp_seq_min);
        fprintf(fd, "range %d tcp seq max %u\n", i, range->tcp_seq_max);
        fprintf(fd, "range %d tcp seq inc %u\n", i, range->tcp_seq_inc);

        fprintf(fd, "\n");
        fprintf(fd, "range %d tcp ack start %u\n", i, range->tcp_ack);
        fprintf(fd, "range %d tcp ack min %u\n", i, range->tcp_ack_min);
        fprintf(fd, "range %d tcp ack max %u\n", i, range->tcp_ack_max);
        fprintf(fd, "range %d tcp ack inc %u\n", i, range->tcp_ack_inc);

        fprintf(fd, "\n");
        fprintf(fd, "range %d ttl start %d\n", i, range->ttl);
        fprintf(fd, "range %d ttl min %d\n", i, range->ttl_min);
        fprintf(fd, "range %d ttl max %d\n", i, range->ttl_max);
        fprintf(fd, "range %d ttl inc %d\n", i, range->ttl_inc);

        fprintf(fd, "\n");
        fprintf(fd, "range %d vlan start %d\n", i, range->vlan_id);
        fprintf(fd, "range %d vlan min %d\n", i, range->vlan_id_min);
        fprintf(fd, "range %d vlan max %d\n", i, range->vlan_id_max);
        fprintf(fd, "range %d vlan inc %d\n", i, range->vlan_id_inc);

        fprintf(fd, "\n");
        fprintf(fd, "range %d cos start %d\n", i, range->cos);
        fprintf(fd, "range %d cos min %d\n", i, range->cos_min);
        fprintf(fd, "range %d cos max %d\n", i, range->cos_max);
        fprintf(fd, "range %d cos inc %d\n", i, range->cos_inc);

        fprintf(fd, "\n");
        fprintf(fd, "range %d tos start %d\n", i, range->tos);
        fprintf(fd, "range %d tos min %d\n", i, range->tos_min);
        fprintf(fd, "range %d tos max %d\n", i, range->tos_max);
        fprintf(fd, "range %d tos inc %d\n", i, range->tos_inc);
        fprintf(fd, "range %d gre key %d\n", i, pkt->gre_key);

        fprintf(fd, "\n");
        fprintf(fd, "range %d size start %d\n", i, range->pkt_size + RTE_ETHER_CRC_LEN);
        fprintf(fd, "range %d size min %d\n", i, range->pkt_size_min + RTE_ETHER_CRC_LEN);
        fprintf(fd, "range %d size max %d\n", i, range->pkt_size_max + RTE_ETHER_CRC_LEN);
        fprintf(fd, "range %d size inc %d\n\n", i, range->pkt_size_inc);

        fprintf(fd, "#\n# Set up the rate data for the port.\n");
        fprintf(fd, "rate %d fps %d\n", i, rate->fps);
        fprintf(fd, "rate %d lines %d\n", i, rate->vlines);
        fprintf(fd, "rate %d pixels %d\n", i, rate->pixels);
        fprintf(fd, "rate %d color bits %d\n", i, rate->color_bits);
        fprintf(fd, "rate %d payload size %d\n\n", i, rate->payload);
        fprintf(fd, "rate %d overhead %d\n", i, rate->overhead);

        fprintf(fd, "#\n# Set up the sequence data for the port.\n");
        fprintf(fd, "set %d seq_cnt %d\n", info->pid, info->seqCnt);
        for (j = 0; j < info->seqCnt; j++) {
            pkt = &info->seq_pkt[j];
            fprintf(fd, "seq %d %d %s ", j, i, inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
            fprintf(fd, "%s ", inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));
            fprintf(fd, "%s ",
                    (pkt->ethType == RTE_ETHER_TYPE_IPV6)
                        ? inet_ntop6(buff, sizeof(buff), pkt->ip_dst_addr.addr.ipv6.s6_addr,
                                     PG_PREFIXMAX)
                        : inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_dst_addr.addr.ipv4.s_addr),
                                     0xFFFFFFFF));
            fprintf(fd, "%s ",
                    (pkt->ethType == RTE_ETHER_TYPE_IPV6)
                        ? inet_ntop6(buff, sizeof(buff), pkt->ip_src_addr.addr.ipv6.s6_addr,
                                     pkt->ip_src_addr.prefixlen)
                        : inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_src_addr.addr.ipv4.s_addr),
                                     pkt->ip_mask));
            fprintf(fd, "%d %d %s %s %d %d %d\n", pkt->sport, pkt->dport,
                    (pkt->ethType == RTE_ETHER_TYPE_IPV4)   ? "ipv4"
                    : (pkt->ethType == RTE_ETHER_TYPE_IPV6) ? "ipv6"
                    : (pkt->ethType == RTE_ETHER_TYPE_VLAN) ? "vlan"
                                                            : "Other",
                    (pkt->ipProto == PG_IPPROTO_TCP)    ? "tcp"
                    : (pkt->ipProto == PG_IPPROTO_ICMP) ? "icmp"
                                                        : "udp",
                    pkt->vlanid, pkt->pktSize + RTE_ETHER_CRC_LEN, pkt->gtpu_teid);
        }

        if (pktgen.info[i].pcap) {
            fprintf(fd, "#\n# PCAP port %d\n", i);
            fprintf(fd, "#    Packet count: %d\n", pktgen.info[i].pcap->pkt_count);
            fprintf(fd, "#    Filename    : %s\n", pktgen.info[i].pcap->filename);
        }
        fprintf(fd, "\n");

        if (info->rnd_bitfields && info->rnd_bitfields->active_specs) {
            uint32_t active = info->rnd_bitfields->active_specs;
            bf_spec_t *bf;
            fprintf(fd, "\n-- Rnd bitfeilds\n");
            for (j = 0; j < MAX_RND_BITFIELDS; j++) {
                if ((active & (1 << j)) == 0)
                    continue;
                bf = &info->rnd_bitfields->specs[j];
                fprintf(fd, "set %d rnd %d %d %s\n", i, j, bf->offset, convert_bitfield(bf));
            }
            fprintf(fd, "\n");
        }
    }
    fprintf(fd, "# Enable screen updates and cleanup\n");
    fprintf(fd, "on\n");
    fprintf(fd, "cls\n\n");
    fprintf(fd, "################################ Done #################################\n");

    fchmod(fileno(fd), 0666);
    fclose(fd);
    return 0;
}

/**
 *
 * pktgen_lua_save - Save a configuration as a Lua script
 *
 * DESCRIPTION
 * Save a configuration as a Lua script
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static int
pktgen_lua_save(char *path)
{
    port_info_t *info;
    pkt_seq_t *pkt;
    range_info_t *range;
    latency_t *lat;
    uint32_t flags;
    char buff[64];
    FILE *fd;
    int i, j;
    uint64_t lcore;
    struct rte_ether_addr eaddr;

    fd = fopen(path, "w");
    if (fd == NULL)
        return -1;

    for (i = 0, lcore = 0; i < RTE_MAX_LCORE; i++)
        if (rte_lcore_is_enabled(i))
            lcore |= (1 << i);

    fprintf(fd, "--\n-- %s\n", pktgen_version());
    fprintf(fd, "-- %s, %s\n\n", copyright_msg(), powered_by());

    fprintf(fd, "package.path = package.path ..\";?.lua;test/?.lua;app/?.lua;\"\n");
    fprintf(fd, "require \"Pktgen\"\n\n");

    /* TODO: Determine DPDK arguments for rank and memory, default for now. */
    fprintf(fd, "-- Command line arguments: (DPDK args are defaults)\n");
    fprintf(fd, "-- %s -c %" PRIx64 " -n 3 -m 512 --proc-type %s -- ", pktgen.argv[0], lcore,
            (rte_eal_process_type() == RTE_PROC_PRIMARY) ? "primary" : "secondary");
    for (i = 1; i < pktgen.argc; i++)
        fprintf(fd, "%s ", pktgen.argv[i]);
    fprintf(fd, "\n\n-- %s\n", hash_line);

    fprintf(fd, "-- Pktgen Configuration script information:\n");
    fprintf(fd, "--   GUI socket is %s\n",
            (pktgen.flags & ENABLE_GUI_FLAG) ? "Enabled" : "Not Enabled");
    fprintf(fd, "--   Flags %08x\n", pktgen.flags);
    fprintf(fd, "--   Number of ports: %d\n", pktgen.nb_ports);
    fprintf(fd, "--   Number ports per page: %d\n", pktgen.nb_ports_per_page);
    fprintf(fd, "--   Number descriptors: RX %d TX: %d\n", pktgen.nb_rxd, pktgen.nb_txd);
    fprintf(fd, "--   Promiscuous mode is %s\n\n",
            (pktgen.flags & PROMISCUOUS_ON_FLAG) ? "Enabled" : "Disabled");

    fprintf(fd, "\n--%s\n", hash_line);

    fprintf(fd, "-- Global configuration:\n");
    uint16_t rows, cols;
    pktgen_display_get_geometry(&rows, &cols);
    fprintf(fd, "--   geometry %dx%d\n", cols, rows);
    fprintf(fd, "pktgen.mac_from_arp(\"%s\");\n\n",
            (pktgen.flags & MAC_FROM_ARP_FLAG) ? "enable" : "disable");

    for (i = 0; i < RTE_MAX_ETHPORTS; i++) {
        info  = &pktgen.info[i];
        pkt   = &info->seq_pkt[SINGLE_PKT];
        range = &info->range;

        if (info->tx_burst == 0)
            continue;

        lat = &info->latency;

        fprintf(fd, "-- ######################### Port %2d ##################################\n",
                i);
        if (rte_atomic64_read(&info->transmit_count) == 0)
            strcpy(buff, "Forever");
        else
            snprintf(buff, sizeof(buff), "%" PRIu64, rte_atomic64_read(&info->transmit_count));
        fprintf(fd, "-- \n");
        flags = rte_atomic32_read(&info->port_flags);
        fprintf(fd, "-- Port: %2d, Burst (Rx/Tx):%3d/%3d, Rate:%g%%, Flags:%08x, TX Count:%s\n",
                info->pid, info->rx_burst, info->tx_burst, info->tx_rate, flags, buff);
        fprintf(fd, "--           Sequence Count:%d, Prime:%d VLAN ID:%04x, ", info->seqCnt,
                info->prime_cnt, info->vlanid);
        pktgen_link_state(info->pid, buff, sizeof(buff));
        fprintf(fd, "Link: %s\n", buff);

        fprintf(fd, "--\n-- Set up the primary port information:\n");
        fprintf(fd, "pktgen.set('%d', 'count', %" PRIu64 ");\n", info->pid,
                rte_atomic64_read(&info->transmit_count));
        fprintf(fd, "pktgen.set('%d', 'size', %d);\n", info->pid, pkt->pktSize + RTE_ETHER_CRC_LEN);
        fprintf(fd, "pktgen.set('%d', 'rate', %g);\n", info->pid, info->tx_rate);
        fprintf(fd, "pktgen.set('%d', 'txburst', %d);\n", info->pid, info->tx_burst);
        fprintf(fd, "pktgen.set('%d', 'rxburst', %d);\n", info->pid, info->rx_burst);
        fprintf(fd, "pktgen.set('%d', 'sport', %d);\n", info->pid, pkt->sport);
        fprintf(fd, "pktgen.set('%d', 'dport', %d);\n", info->pid, pkt->dport);
        fprintf(fd, "pktgen.set('%d', 'prime', %d);\n", info->pid, info->prime_cnt);
        fprintf(fd, "pktgen.set_type('%d', '%s');\n", i,
                (pkt->ethType == RTE_ETHER_TYPE_IPV4)   ? "ipv4"
                : (pkt->ethType == RTE_ETHER_TYPE_IPV6) ? "ipv6"
                : (pkt->ethType == RTE_ETHER_TYPE_VLAN) ? "vlan"
                : (pkt->ethType == RTE_ETHER_TYPE_ARP)  ? "arp"
                                                        : "unknown");
        fprintf(fd, "pktgen.set_proto('%d', '%s');\n", i,
                (pkt->ipProto == PG_IPPROTO_TCP)    ? "tcp"
                : (pkt->ipProto == PG_IPPROTO_ICMP) ? "icmp"
                                                    : "udp");
        fprintf(
            fd, "pktgen.set_ipaddr('%d', 'dst', '%s');\n", i,
            (pkt->ethType == RTE_ETHER_TYPE_IPV6)
                ? inet_ntop6(buff, sizeof(buff), pkt->ip_dst_addr.addr.ipv6.s6_addr, PG_PREFIXMAX)
                : inet_ntop4(buff, sizeof(buff), ntohl(pkt->ip_dst_addr.addr.ipv4.s_addr),
                             0xFFFFFFFF));
        fprintf(fd, "pktgen.set_ipaddr('%d', 'src','%s');\n", i,
                (pkt->ethType == RTE_ETHER_TYPE_IPV6)
                    ? inet_ntop6(buff, sizeof(buff), pkt->ip_src_addr.addr.ipv6.s6_addr,
                                 pkt->ip_src_addr.prefixlen)
                    : inet_ntop4(buff, sizeof(buff), ntohl(pkt->ip_src_addr.addr.ipv4.s_addr),
                                 pkt->ip_mask));
        fprintf(fd, "pktgen.set_mac('%d', 'dst', '%s');\n", info->pid,
                inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
        fprintf(fd, "pktgen.set_mac('%d', 'src', '%s');\n", info->pid,
                inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));
        fprintf(fd, "pktgen.vlanid('%d', %d);\n\n", i, pkt->vlanid);

        fprintf(fd, "pktgen.pattern('%d', '%s');\n", i,
                (info->fill_pattern_type == ABC_FILL_PATTERN)    ? "abc"
                : (info->fill_pattern_type == NO_FILL_PATTERN)   ? "none"
                : (info->fill_pattern_type == ZERO_FILL_PATTERN) ? "zero"
                                                                 : "user");
        if ((info->fill_pattern_type == USER_FILL_PATTERN) && strlen(info->user_pattern)) {
            char buff[64];
            memset(buff, 0, sizeof(buff));
            snprintf(buff, sizeof(buff), "%s", info->user_pattern);
            fprintf(fd, "pktgen.userPattern('%d', '%s');\n", i, buff);
        }
        fprintf(fd, "\n");

        fflush(fd);
        fprintf(fd, "pktgen.jitter('%d', %lu);\n", i, lat->jitter_threshold_us);
        fprintf(fd, "pktgen.mpls('%d', '%sable');\n", i, (flags & SEND_MPLS_LABEL) ? "en" : "dis");
        sprintf(buff, "0x%x", pkt->mpls_entry);
        fprintf(fd, "pktgen.mpls_entry('%d', '%s');\n", i, buff);

        fprintf(fd, "pktgen.qinq('%d', '%sable');\n", i, (flags & SEND_Q_IN_Q_IDS) ? "en" : "dis");
        fprintf(fd, "pktgen.qinqids('%d', %d, %d);\n", i, pkt->qinq_outerid, pkt->qinq_innerid);

        fprintf(fd, "pktgen.gre('%d', '%sable');\n", i,
                (flags & SEND_GRE_IPv4_HEADER) ? "en" : "dis");
        fprintf(fd, "pktgen.gre_eth('%d', '%sable');\n", i,
                (flags & SEND_GRE_ETHER_HEADER) ? "en" : "dis");
        fprintf(fd, "pktgen.gre_key('%d', %d);\n", i, pkt->gre_key);

        fprintf(fd, "pkrgen.vxlan('%d', '%sable');\n", i,
                (flags & SEND_VXLAN_PACKETS) ? "en" : "dis");
        fprintf(fd, "pktgen.vxlan_id('%d', '0x%x', '%d', '%d');\n", i, pkt->vni_flags,
                pkt->group_id, pkt->vxlan_id);

        fprintf(fd, "--\n-- Port flag values:\n");
        fprintf(fd, "pktgen.icmp_echo('%d', '%sable');\n", i,
                (flags & ICMP_ECHO_ENABLE_FLAG) ? "en" : "dis");
        fprintf(fd, "pktgen.pcap('%d', '%sable');\n", i, (flags & SEND_PCAP_PKTS) ? "en" : "dis");
        fprintf(fd, "pktgen.set_range('%d', '%sable');\n", i,
                (flags & SEND_RANGE_PKTS) ? "en" : "dis");
        fprintf(fd, "pktgen.latency('%d', '%sable');\n", i,
                (flags & ENABLE_LATENCY_PKTS) ? "en" : "dis");
        fprintf(fd, "pktgen.process('%d', '%sable');\n", i,
                (flags & PROCESS_INPUT_PKTS) ? "en" : "dis");
        fprintf(fd, "pktgen.capture('%d', '%sable');\n", i, (flags & CAPTURE_PKTS) ? "en" : "dis");
        fprintf(fd, "pktgen.rxtap('%d', '%sable');\n", i,
                (flags & PROCESS_RX_TAP_PKTS) ? "en" : "dis");
        fprintf(fd, "pktgen.txtap('%d', '%sable');\n", i,
                (flags & PROCESS_TX_TAP_PKTS) ? "en" : "dis");
        fprintf(fd, "pktgen.vlan('%d', '%sable');\n\n", i, (flags & SEND_VLAN_ID) ? "en" : "dis");
        fflush(fd);
        fprintf(fd, "--\n-- Range packet information:\n");
        fprintf(fd, "pktgen.src_mac('%d', 'start', '%s');\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac, &eaddr)));
        fprintf(fd, "pktgen.src_mac('%d', 'min', '%s');\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac_min, &eaddr)));
        fprintf(fd, "pktgen.src_mac('%d', 'max', '%s');\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac_max, &eaddr)));
        fprintf(fd, "pktgen.src_mac('%d', 'inc', '%s');\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac_inc, &eaddr)));

        fprintf(fd, "pktgen.dst_mac('%d', 'start', '%s');\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac, &eaddr)));
        fprintf(fd, "pktgen.dst_mac('%d', 'min', '%s');\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac_min, &eaddr)));
        fprintf(fd, "pktgen.dst_mac('%d', 'max', '%s');\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac_max, &eaddr)));
        fprintf(fd, "pktgen.dst_mac('%d', 'inc', '%s');\n", i,
                inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac_inc, &eaddr)));

        fprintf(fd, "\n");
        fprintf(fd, "pktgen.src_ip('%d', 'start', '%s');\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip), 0xFFFFFFFF));
        fprintf(fd, "pktgen.src_ip('%d', 'min', '%s');\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip_min), 0xFFFFFFFF));
        fprintf(fd, "pktgen.src_ip('%d', 'max', '%s');\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip_max), 0xFFFFFFFF));
        fprintf(fd, "pktgen.src_ip('%d', 'inc', '%s');\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip_inc), 0xFFFFFFFF));

        fprintf(fd, "\n");
        fprintf(fd, "pktgen.dst_ip('%d', 'start', '%s');\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip), 0xFFFFFFFF));
        fprintf(fd, "pktgen.dst_ip('%d', 'min', '%s');\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip_min), 0xFFFFFFFF));
        fprintf(fd, "pktgen.dst_ip('%d', 'max', '%s');\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip_max), 0xFFFFFFFF));
        fprintf(fd, "pktgen.dst_ip('%d', 'inc', '%s');\n", i,
                inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip_inc), 0xFFFFFFFF));

        fprintf(fd, "\n");
        fprintf(fd, "pktgen.ip_proto('%d', '%s');\n", i,
                (range->ip_proto == PG_IPPROTO_UDP)    ? "udp"
                : (range->ip_proto == PG_IPPROTO_ICMP) ? "icmp"
                                                       : "tcp");

        fprintf(fd, "\n");
        fprintf(fd, "pktgen.src_port('%d', 'start', %d);\n", i, range->src_port);
        fprintf(fd, "pktgen.src_port('%d', 'min', %d);\n", i, range->src_port_min);
        fprintf(fd, "pktgen.src_port('%d', 'max', %d);\n", i, range->src_port_max);
        fprintf(fd, "pktgen.src_port('%d', 'inc', %d);\n", i, range->src_port_inc);

        fprintf(fd, "\n");
        fprintf(fd, "pktgen.dst_port('%d', 'start', %d);\n", i, range->dst_port);
        fprintf(fd, "pktgen.dst_port('%d', 'min', %d);\n", i, range->dst_port_min);
        fprintf(fd, "pktgen.dst_port('%d', 'max', %d);\n", i, range->dst_port_max);
        fprintf(fd, "pktgen.dst_port('%d', 'inc', %d);\n", i, range->dst_port_inc);

        fprintf(fd, "\n");
        fprintf(fd, "pktgen.ttl('%d', 'start', %d);\n", i, range->ttl);
        fprintf(fd, "pktgen.ttl('%d', 'min', %d);\n", i, range->ttl_min);
        fprintf(fd, "pktgen.ttl('%d', 'max', %d);\n", i, range->ttl_max);
        fprintf(fd, "pktgen.ttl('%d', 'inc', %d);\n", i, range->ttl_inc);

        fprintf(fd, "\n");
        fprintf(fd, "pktgen.vlan_id('%d', 'start', %d);\n", i, range->vlan_id);
        fprintf(fd, "pktgen.vlan_id('%d', 'min', %d);\n", i, range->vlan_id_min);
        fprintf(fd, "pktgen.vlan_id('%d', 'max', %d);\n", i, range->vlan_id_max);
        fprintf(fd, "pktgen.vlan_id('%d', 'inc', %d);\n", i, range->vlan_id_inc);

        fprintf(fd, "\n");
        fprintf(fd, "pktgen.cos('%d', 'start', %d);\n", i, range->cos);
        fprintf(fd, "pktgen.cos('%d', 'min', %d);\n", i, range->cos_min);
        fprintf(fd, "pktgen.cos('%d', 'max', %d);\n", i, range->cos_max);
        fprintf(fd, "pktgen.cos('%d', 'inc', %d);\n", i, range->cos_inc);

        fprintf(fd, "\n");
        fprintf(fd, "pktgen.tos('%d', 'start', %d);\n", i, range->tos);
        fprintf(fd, "pktgen.tos('%d', 'min', %d);\n", i, range->tos_min);
        fprintf(fd, "pktgen.tos('%d', 'max', %d);\n", i, range->tos_max);
        fprintf(fd, "pktgen.tos('%d', 'inc', %d);\n", i, range->tos_inc);

        fprintf(fd, "\n");
        fprintf(fd, "pktgen.pkt_size('%d', 'start', %d);\n", i,
                range->pkt_size + RTE_ETHER_CRC_LEN);
        fprintf(fd, "pktgen.pkt_size('%d', 'min', %d);\n", i,
                range->pkt_size_min + RTE_ETHER_CRC_LEN);
        fprintf(fd, "pktgen.pkt_size('%d', 'max', %d);\n", i,
                range->pkt_size_max + RTE_ETHER_CRC_LEN);
        fprintf(fd, "pktgen.pkt_size('%d', 'inc', %d);\n\n", i, range->pkt_size_inc);

        fprintf(fd, "--\n-- Set up the sequence data for the port.\n");
        fprintf(fd, "pktgen.set('%d', 'seq_cnt', %d);\n\n", info->pid, info->seqCnt);
        fflush(fd);
        if (info->seqCnt) {
            fprintf(fd, "-- (seqnum, port, dst_mac, src_mac, ip_dst, ip_src, sport, dport, "
                        "ethType, proto, vlanid, pktSize, gtpu_teid)\n");
            for (j = 0; j < info->seqCnt; j++) {
                pkt = &info->seq_pkt[j];
                fprintf(fd, "-- pktgen.seq(%d, '%d', '%s' ", j, i,
                        inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
                fprintf(fd, "'%s', ", inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));
                fprintf(fd, "'%s', ",
                        (pkt->ethType == RTE_ETHER_TYPE_IPV6)
                            ? inet_ntop6(buff, sizeof(buff), pkt->ip_dst_addr.addr.ipv6.s6_addr,
                                         PG_PREFIXMAX)
                            : inet_ntop4(buff, sizeof(buff),
                                         htonl(pkt->ip_dst_addr.addr.ipv4.s_addr), 0xFFFFFFFF));
                fprintf(fd, "'%s', ",
                        (pkt->ethType == RTE_ETHER_TYPE_IPV6)
                            ? inet_ntop6(buff, sizeof(buff), pkt->ip_src_addr.addr.ipv6.s6_addr,
                                         pkt->ip_src_addr.prefixlen)
                            : inet_ntop4(buff, sizeof(buff),
                                         htonl(pkt->ip_src_addr.addr.ipv4.s_addr), pkt->ip_mask));
                fprintf(fd, "%d, %d, '%s', '%s', %d, %d, %d);\n", pkt->sport, pkt->dport,
                        (pkt->ethType == RTE_ETHER_TYPE_IPV4)   ? "ipv4"
                        : (pkt->ethType == RTE_ETHER_TYPE_IPV6) ? "ipv6"
                        : (pkt->ethType == RTE_ETHER_TYPE_VLAN) ? "vlan"
                                                                : "Other",
                        (pkt->ipProto == PG_IPPROTO_TCP)    ? "tcp"
                        : (pkt->ipProto == PG_IPPROTO_ICMP) ? "icmp"
                                                            : "udp",
                        pkt->vlanid, pkt->pktSize + RTE_ETHER_CRC_LEN, pkt->gtpu_teid);
            }
            fflush(fd);
            fprintf(fd, "local seq_table = {}\n");
            for (j = 0; j < info->seqCnt; j++) {
                pkt = &info->seq_pkt[j];
                fprintf(fd, "seq_table[%d] = {\n", j);
                fprintf(fd, "  ['eth_dst_addr'] = '%s',\n",
                        inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
                fprintf(fd, "  ['eth_src_addr'] = '%s',\n",
                        inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));
                fprintf(fd, "  ['ip_dst_addr'] = '%s',\n",
                        (pkt->ethType == RTE_ETHER_TYPE_IPV6)
                            ? inet_ntop6(buff, sizeof(buff), pkt->ip_dst_addr.addr.ipv6.s6_addr,
                                         PG_PREFIXMAX)
                            : inet_ntop4(buff, sizeof(buff),
                                         htonl(pkt->ip_dst_addr.addr.ipv4.s_addr), 0xFFFFFFFF));
                fprintf(fd, "  ['ip_src_addr'] = '%s',\n",
                        (pkt->ethType == RTE_ETHER_TYPE_IPV6)
                            ? inet_ntop6(buff, sizeof(buff), pkt->ip_src_addr.addr.ipv6.s6_addr,
                                         pkt->ip_src_addr.prefixlen)
                            : inet_ntop4(buff, sizeof(buff),
                                         htonl(pkt->ip_src_addr.addr.ipv4.s_addr), 0xFFFFFFFF));
                fprintf(fd, "  ['sport'] = %d,\n", pkt->sport);
                fprintf(fd, "  ['dport'] = %d,\n", pkt->dport);
                fprintf(fd, "  ['ethType'] = '%s',\n",
                        (pkt->ethType == RTE_ETHER_TYPE_IPV4)   ? "ipv4"
                        : (pkt->ethType == RTE_ETHER_TYPE_IPV6) ? "ipv6"
                        : (pkt->ethType == RTE_ETHER_TYPE_VLAN) ? "vlan"
                                                                : "Other");
                fprintf(fd, "  ['ipProto'] = '%s',\n",
                        (pkt->ipProto == PG_IPPROTO_TCP)    ? "tcp"
                        : (pkt->ipProto == PG_IPPROTO_ICMP) ? "icmp"
                                                            : "udp");
                fprintf(fd, "  ['vlanid'] = %d,\n", pkt->vlanid);
                fprintf(fd, "  ['pktSize'] = %d,\n", pkt->pktSize);
                fprintf(fd, "  ['gtpu_teid'] = %d\n", pkt->gtpu_teid);
                fprintf(fd, "}\n");
            }
            fflush(fd);
            for (j = 0; j < info->seqCnt; j++)
                fprintf(fd, "pktgen.seqTable(%d, '%d', seq_table[%d]);\n", j, i, j);
        }
        fflush(fd);
        if (pktgen.info[i].pcap) {
            fprintf(fd, "--\n-- PCAP port %d\n", i);
            fprintf(fd, "--    Packet count: %d\n", pktgen.info[i].pcap->pkt_count);
            fprintf(fd, "--    Filename    : %s\n", pktgen.info[i].pcap->filename);
        }
        fprintf(fd, "\n");
        fflush(fd);
        if (info->rnd_bitfields && info->rnd_bitfields->active_specs) {
            uint32_t active = info->rnd_bitfields->active_specs;
            bf_spec_t *bf;
            fprintf(fd, "\n-- Rnd bitfeilds\n");
            fflush(fd);
            for (j = 0; j < MAX_RND_BITFIELDS; j++) {
                if ((active & (1 << j)) == 0)
                    continue;
                bf = &info->rnd_bitfields->specs[j];
                fprintf(fd, "pktgen.rnd('%d', %d, %d, '%s');\n", i, j, bf->offset,
                        convert_bitfield(bf));
            }
            fprintf(fd, "\n");
        }
    }
    fprintf(fd, "pktgen.screen('on');\n");
    fprintf(fd, "pktgen.cls();\n\n");
    fprintf(fd, "-- ################################ Done #################################\n");
    fflush(fd);

    fchmod(fileno(fd), 0666);
    fclose(fd);
    return 0;
}

/**
 *
 * pktgen_save - Save a configuration as a startup script
 *
 * DESCRIPTION
 * Save a configuration as a startup script
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
int
pktgen_save(char *path)
{
    if (strcasestr(path, ".lua") != NULL)
        return pktgen_lua_save(path);
    else
        return pktgen_script_save(path);
}

/**
 *
 * pktgen_port_transmitting - Is the port transmitting packets?
 *
 * DESCRIPTION
 * Is the port transmitting packets.
 *
 * RETURNS: 1 for yes and 0 for no.
 *
 * SEE ALSO:
 */
int
pktgen_port_transmitting(int port)
{
    return pktgen_tst_port_flags(&pktgen.info[port], SENDING_PACKETS);
}

/**
 *
 * pktgen_link_state - Get the ASCII string for the port state.
 *
 * DESCRIPTION
 * Return the port state string for a given port.
 *
 * RETURNS: String pointer to link state
 *
 * SEE ALSO:
 */

char *
pktgen_link_state(int port, char *buff, int len)
{
    port_info_t *info = &pktgen.info[port];

    if (info->link.link_status)
        snprintf(buff, len, "<UP-%u-%s>", (uint32_t)info->link.link_speed,
                 (info->link.link_duplex == RTE_ETH_LINK_FULL_DUPLEX) ? ("FD") : ("HD"));
    else
        snprintf(buff, len, "<--Down-->");

    return buff;
}

/**
 *
 * pktgen_transmit_count_rate - Get a string for the current transmit count and rate
 *
 * DESCRIPTION
 * Current value of the transmit count/%rate as a string.
 *
 * RETURNS: String pointer to transmit count/%rate.
 *
 * SEE ALSO:
 */

char *
pktgen_transmit_count_rate(int port, char *buff, int len)
{
    port_info_t *info = &pktgen.info[port];

    if (rte_atomic64_read(&info->transmit_count) == 0)
        snprintf(buff, len, "Forever /%g%%", info->tx_rate);
    else
        snprintf(buff, len, "%" PRIu64 " /%g%%", rte_atomic64_read(&info->transmit_count),
                 info->tx_rate);

    return buff;
}

/**
 *
 * rate_transmit_count_rate - Get a string for the current transmit count and rate
 *
 * DESCRIPTION
 * Current value of the transmit count/%rate as a string.
 *
 * RETURNS: String pointer to transmit count/%rate.
 *
 * SEE ALSO:
 */

char *
rate_transmit_count_rate(int port, char *buff, int len)
{
    port_info_t *info = &pktgen.info[port];

    if (rte_atomic64_read(&info->transmit_count) == 0)
        snprintf(buff, len, "Forever");
    else
        snprintf(buff, len, "%" PRIu64 "", rte_atomic64_read(&info->transmit_count));

    return buff;
}

/**
 *
 * pktgen_pkt_sizes - Current stats for all port sizes
 *
 * DESCRIPTION
 * Structure returned with all of the counts for each port size.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

int
pktgen_pkt_sizes(int port, pkt_sizes_t *psizes)
{
    port_info_t *info = &pktgen.info[port];

    if (!psizes)
        return -1;

    for (int qid = 0; qid < NUM_Q; qid++) {
        psizes->broadcast += info->pkt_sizes.broadcast;
        psizes->jumbo += info->pkt_sizes.jumbo;
        psizes->multicast += info->pkt_sizes.multicast;
        psizes->runt += info->pkt_sizes.runt;
        psizes->unknown += info->pkt_sizes.unknown;
        psizes->_64 += info->pkt_sizes._64;
        psizes->_65_127 += info->pkt_sizes._65_127;
        psizes->_128_255 += info->pkt_sizes._128_255;
        psizes->_256_511 += info->pkt_sizes._256_511;
        psizes->_512_1023 += info->pkt_sizes._512_1023;
        psizes->_1024_1518 += info->pkt_sizes._1024_1518;
    }
    return 0;
}

/**
 *
 * pktgen_pkt_stats - Get the packet stats structure.
 *
 * DESCRIPTION
 * Return the packet statistics values.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

int
pktgen_pkt_stats(int port, pkt_stats_t *pstats)
{
    port_info_t *info = &pktgen.info[port];

    if (!pstats)
        return -1;

    for (int qid = 0; qid < NUM_Q; qid++) {
        pstats->arp_pkts += info->pkt_stats.arp_pkts;
        pstats->dropped_pkts += info->pkt_stats.dropped_pkts;
        pstats->echo_pkts += info->pkt_stats.echo_pkts;
        pstats->ibadcrc += info->pkt_stats.ibadcrc;
        pstats->ibadlen += info->pkt_stats.ibadlen;
        pstats->imissed += info->pkt_stats.imissed;
        pstats->ip_pkts += info->pkt_stats.ip_pkts;
        pstats->ipv6_pkts += info->pkt_stats.ipv6_pkts;
        pstats->rx_nombuf += info->pkt_stats.rx_nombuf;
        pstats->tx_failed += info->pkt_stats.tx_failed;
        pstats->unknown_pkts += info->pkt_stats.unknown_pkts;
        pstats->vlan_pkts += info->pkt_stats.vlan_pkts;
    }
    return 0;
}

/**
 *
 * pktgen_port_stats - Get the port or rate stats for a given port
 *
 * DESCRIPTION
 * Get the ports or rate stats from a given port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

int
pktgen_port_stats(int port, const char *name, eth_stats_t *pstats)
{
    port_info_t *info = &pktgen.info[port];

    if (strcmp(name, "port") == 0)
        *pstats = info->prev_stats;
    else if (strcmp(name, "rate") == 0)
        *pstats = info->rate_stats;

    return 0;
}

/**
 *
 * pktgen_flags_string - Return the flags string for display
 *
 * DESCRIPTION
 * Return the current flags string for display for a port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

char *
pktgen_flags_string(port_info_t *info)
{
    static char buff[32];
    uint32_t flags = rte_atomic32_read(&info->port_flags);

    snprintf(buff, sizeof(buff), "%c%c%c%c%c%c%c%-6s%6s",
             (pktgen.flags & PROMISCUOUS_ON_FLAG) ? 'P' : '-',
             (flags & ICMP_ECHO_ENABLE_FLAG) ? 'E' : '-', (flags & BONDING_TX_PACKETS) ? 'B' : '-',
             (flags & PROCESS_INPUT_PKTS) ? 'I' : '-', (flags & ENABLE_LATENCY_PKTS) ? 'L' : '-',
             "-rt*"[(flags & (PROCESS_RX_TAP_PKTS | PROCESS_TX_TAP_PKTS)) >> 9],
             (flags & CAPTURE_PKTS) ? 'c' : '-',

             (flags & SEND_VLAN_ID)            ? "VLAN"
             : (flags & SEND_VXLAN_PACKETS)    ? "VxLan"
             : (flags & SEND_MPLS_LABEL)       ? "MPLS"
             : (flags & SEND_Q_IN_Q_IDS)       ? "QnQ"
             : (flags & SEND_GRE_IPv4_HEADER)  ? "GREip"
             : (flags & SEND_GRE_ETHER_HEADER) ? "GREet"
                                               : "",

             (flags & SEND_PCAP_PKTS)      ? "PCAP"
             : (flags & SEND_SEQ_PKTS)     ? "Seq"
             : (flags & SEND_RANGE_PKTS)   ? "Range"
             : (flags & SEND_RANDOM_PKTS)  ? "Random"
             : (flags & SEND_RATE_PACKETS) ? "Rate"
                                           : "Single");

    /* single, range, sequence, random, pcap, latency, rate */

    return buff;
}

/**
 *
 * pktgen_update_display - Update the display data and static data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_update_display(void)
{
    pktgen.flags |= PRINT_LABELS_FLAG;
    pktgen.flags |= UPDATE_DISPLAY_FLAG;
}

/**
 *
 * pktgen_clear_display - clear the screen.
 *
 * DESCRIPTION
 * clear the screen and redisplay data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_clear_display(void)
{
    if (!scrn_is_paused()) {
        scrn_pause();

        scrn_cls();
        scrn_pos(this_scrn->nrows + 1, 1);

        pktgen_update_display();

        scrn_resume();

        pktgen_page_display();
    }
}

/**
 *
 * pktgen_force_update - Force the screen to update data and static data.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_force_update(void)
{
    pktgen.flags |= UPDATE_DISPLAY_FLAG;

    if (!scrn_is_paused())
        pktgen_page_display();
}

/**
 *
 * pktgen_set_page_size - Set the number of ports per page.
 *
 * DESCRIPTION
 * Set the max number of ports per page.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_set_page_size(uint32_t page_size)
{
    if ((page_size > 0) && (page_size <= pktgen.nb_ports) && (page_size <= 6)) {
        pktgen.nb_ports_per_page = page_size;
        pktgen.ending_port       = pktgen.starting_port + page_size;
        if (pktgen.ending_port >= (pktgen.starting_port + pktgen.nb_ports))
            pktgen.ending_port = (pktgen.starting_port + pktgen.nb_ports);
        pktgen_clear_display();
    }
}

/**
 *
 * pktgen_screen - Enable or Disable screen updates.
 *
 * DESCRIPTION
 * Enable or disable screen updates.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_screen(int state)
{
    uint16_t rows;

    pktgen_display_get_geometry(&rows, NULL);

    if (state == DISABLE_STATE) {
        if (!scrn_is_paused()) {
            scrn_pause();
            scrn_cls();
            scrn_setw(1);
            scrn_pos(rows + 1, 1);
        }
    } else {
        scrn_cls();
        scrn_setw(pktgen.last_row + 1);
        scrn_resume();
        scrn_pos(rows + 1, 1);
        pktgen_force_update();
    }
}

/**
 *
 * pktgen_set_port_number - Set the current port number for sequence and range pages
 *
 * DESCRIPTION
 * Set the current port number for sequence and range pages.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_set_port_number(uint16_t port_number)
{
    if (port_number < pktgen.nb_ports) {
        pktgen.portNum = port_number;
        pktgen_clear_display();
    }
}

/**
 *
 * pktgen_set_icmp_echo - Set the ICMP echo response flag on a port
 *
 * DESCRIPTION
 * Enable or disable the ICMP echo response flags for the given ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_icmp_echo(port_info_t *info, uint32_t onOff)
{
    if (onOff == ENABLE_STATE)
        pktgen_set_port_flags(info, ICMP_ECHO_ENABLE_FLAG);
    else
        pktgen_clr_port_flags(info, ICMP_ECHO_ENABLE_FLAG);
}

/**
 *
 * enable_rx_tap - Enable or disable the Rx TAP interface
 *
 * DESCRIPTION
 * Create and setup the Rx TAP interface.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_rx_tap(port_info_t *info, uint32_t onOff)
{
    if (onOff == ENABLE_STATE) {
        struct ifreq ifr;
        int sockfd, i;
        static const char *tapdevs[] = {"/dev/net/tun", "/dev/tun", NULL};

        for (i = 0; tapdevs[i]; i++)
            if ((info->rx_tapfd = open(tapdevs[i], O_RDWR)) >= 0)
                break;
        if (tapdevs[i] == NULL) {
            pktgen_log_error("Unable to create TUN/TAP interface");
            return;
        }
        memset(&ifr, 0, sizeof(struct ifreq));

        ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

        snprintf(ifr.ifr_name, IFNAMSIZ, "%s%d", "pg_rxtap", info->pid);
        if (ioctl(info->rx_tapfd, TUNSETIFF, (void *)&ifr) < 0) {
            pktgen_log_error("Unable to set TUNSETIFF for %s", ifr.ifr_name);
            close(info->rx_tapfd);
            info->rx_tapfd = 0;
            return;
        }

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        ifr.ifr_flags = IFF_UP | IFF_RUNNING;
        if (ioctl(sockfd, SIOCSIFFLAGS, (void *)&ifr) < 0) {
            pktgen_log_error("Unable to set SIOCSIFFLAGS for %s",

                             ifr.ifr_name);
            close(sockfd);
            close(info->rx_tapfd);
            info->rx_tapfd = 0;
            return;
        }
        close(sockfd);
        pktgen_set_port_flags(info, PROCESS_RX_TAP_PKTS);
    } else {
        if (rte_atomic32_read(&info->port_flags) & PROCESS_RX_TAP_PKTS) {
            close(info->rx_tapfd);
            info->rx_tapfd = 0;
        }
        pktgen_clr_port_flags(info, PROCESS_RX_TAP_PKTS);
    }
}

/**
 *
 * enable_tx_tap - Enable or disable the Tx TAP interface
 *
 * DESCRIPTION
 * Create and setup the Tx TAP interface.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_tx_tap(port_info_t *info, uint32_t onOff)
{
    if (onOff == ENABLE_STATE) {
        struct ifreq ifr;
        int sockfd, i;
        static const char *tapdevs[] = {"/dev/net/tun", "/dev/tun", NULL};

        for (i = 0; tapdevs[i]; i++)
            if ((info->tx_tapfd = open(tapdevs[i], O_RDWR)) >= 0)
                break;
        if (tapdevs[i] == NULL) {
            pktgen_log_error("Unable to create TUN/TAP interface.");
            return;
        }
        memset(&ifr, 0, sizeof(struct ifreq));

        ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

        snprintf(ifr.ifr_name, IFNAMSIZ, "%s%d", "pg_txtap", info->pid);
        if (ioctl(info->tx_tapfd, TUNSETIFF, (void *)&ifr) < 0) {
            pktgen_log_error("Unable to set TUNSETIFF for %s", ifr.ifr_name);
            close(info->tx_tapfd);
            info->tx_tapfd = 0;
            return;
        }

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        ifr.ifr_flags = IFF_UP | IFF_RUNNING;
        if (ioctl(sockfd, SIOCSIFFLAGS, (void *)&ifr) < 0) {
            pktgen_log_error("Unable to set SIOCSIFFLAGS for %s", ifr.ifr_name);
            close(sockfd);
            close(info->tx_tapfd);
            info->tx_tapfd = 0;
            return;
        }
        close(sockfd);
        pktgen_set_port_flags(info, PROCESS_TX_TAP_PKTS);
    } else {
        if (rte_atomic32_read(&info->port_flags) & PROCESS_TX_TAP_PKTS) {
            close(info->tx_tapfd);
            info->tx_tapfd = 0;
        }
        pktgen_clr_port_flags(info, PROCESS_TX_TAP_PKTS);
    }
}

/**
 *
 * enable_mac_from_arp - Enable or disable getting MAC from ARP requests.
 *
 * DESCRIPTION
 * Enable or disable getting the MAC address from the ARP request packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_mac_from_arp(uint32_t onOff)
{
    if (onOff == ENABLE_STATE)
        pktgen.flags |= MAC_FROM_ARP_FLAG;
    else
        pktgen.flags &= ~MAC_FROM_ARP_FLAG;
}

/**
 *
 * enable_random - Enable/disable random bitfield mode
 *
 * DESCRIPTION
 * Enable/disable random bitfield mode
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_random(port_info_t *info, uint32_t onOff)
{
    if (onOff == ENABLE_STATE) {
        pktgen_clr_port_flags(info, EXCLUSIVE_MODES);
        pktgen_set_port_flags(info, SEND_RANDOM_PKTS);
    } else
        pktgen_clr_port_flags(info, SEND_RANDOM_PKTS);
}

void
enable_clock_gettime(uint32_t onOff)
{
    if (onOff == ENABLE_STATE)
        pktgen.flags |= CLOCK_GETTIME_FLAG;
    else
        pktgen.flags &= ~CLOCK_GETTIME_FLAG;

    pktgen.hz            = pktgen_get_timer_hz();
    pktgen.tx_next_cycle = pktgen_get_time();
    pktgen.tx_bond_cycle = pktgen_get_time();
    pktgen.page_timeout  = UPDATE_DISPLAY_TICK_RATE;
    pktgen.stats_timeout = pktgen.hz;
}

void
debug_tx_rate(port_info_t *info)
{
    printf("  %d: rate %.2f, tx_cycles %'ld, tx_pps %'ld, link %s-%d-%s, hz %'ld\n", info->pid,
           info->tx_rate, info->tx_cycles, info->tx_pps, (info->link.link_status) ? "UP" : "Down",
           info->link.link_speed,
           (info->link.link_duplex == RTE_ETH_LINK_FULL_DUPLEX) ? "FD" : "HD", pktgen.hz);
}

/*
 * Local wrapper function to test mp is NULL and return or continue
 * to call rte_mempool_dump() routine.
 */
static void
__mempool_dump(FILE *f, struct rte_mempool *mp)
{
    if (mp == NULL)
        return;
    rte_mempool_dump(f, mp);
}

/**
 *
 * debug_mempool_dump - Display the mempool information
 *
 * DESCRIPTION
 * Dump out the mempool information.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
debug_mempool_dump(port_info_t *info, char *name)
{
    int all;
    uint16_t q;

    all = !strcmp(name, "all");

    if (info->q[0].tx_mp == NULL)
        return;

    for (q = 0; q < get_port_rxcnt(pktgen.l2p, info->pid); q++)
        if (all || !strcmp(name, "rx"))
            rte_mempool_dump(stdout, info->q[q].rx_mp);

    for (q = 0; q < get_port_txcnt(pktgen.l2p, info->pid); q++) {
        if (all || (!strcmp(name, "tx") && (q < get_port_txcnt(pktgen.l2p, info->pid))))
            __mempool_dump(stdout, info->q[q].tx_mp);
        if (all || !strcmp(name, "range"))
            __mempool_dump(stdout, info->q[q].range_mp);
        if (all || !strcmp(name, "rate"))
            __mempool_dump(stdout, info->q[q].rate_mp);
        if (all || !strcmp(name, "seq"))
            __mempool_dump(stdout, info->q[q].seq_mp);
        if (all || !strcmp(name, "pcap"))
            __mempool_dump(stdout, info->q[q].pcap_mp);
    }
    if (all || !strcmp(name, "arp"))
        __mempool_dump(stdout, info->special_mp);
}

/**
 *
 * pktgen_start_transmitting - Start a port transmitting packets.
 *
 * DESCRIPTION
 * Start the given ports sending packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_start_transmitting(port_info_t *info)
{
    uint8_t q;

    if (!pktgen_tst_port_flags(info, SENDING_PACKETS)) {
        for (q = 0; q < get_port_txcnt(pktgen.l2p, info->pid); q++)
            pktgen_set_q_flags(info, q, CLEAR_FAST_ALLOC_FLAG);

        rte_atomic64_set(&info->current_tx_count, rte_atomic64_read(&info->transmit_count));

        if (rte_atomic64_read(&info->current_tx_count) == 0)
            pktgen_set_port_flags(info, SEND_FOREVER);

        pktgen_set_port_flags(info, SENDING_PACKETS);
    }
}

/**
 *
 * pktgen_stop_transmitting - Stop port transmitting packets.
 *
 * DESCRIPTION
 * Stop the given ports from sending traffic.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_stop_transmitting(port_info_t *info)
{
    if (pktgen_tst_port_flags(info, SENDING_PACKETS)) {
        pktgen_clr_port_flags(info, (SENDING_PACKETS | SEND_FOREVER));
        for (uint8_t q = 0; q < get_port_txcnt(pktgen.l2p, info->pid); q++)
            pktgen_set_q_flags(info, q, DO_TX_FLUSH);
    }
}

static void
pktgen_set_receive_state(port_info_t *info, int state)
{
    if (state)
        pktgen_set_port_flags(info, STOP_RECEIVING_PACKETS);
    else
        pktgen_clr_port_flags(info, STOP_RECEIVING_PACKETS);
}

/**
 *
 * pktgen_start_stop_latency_sampler - Starts or stops latency sampler.
 *
 * DESCRIPTION
 * Starts or stops latency sampler.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_start_stop_latency_sampler(port_info_t *info, uint32_t state)
{
    if (state == ENABLE_STATE)
        pktgen_start_latency_sampler(info);
    else if (state == DISABLE_STATE)
        pktgen_stop_latency_sampler(info);
}

/**
 *
 * start_latency_sampler - Starts latency sampler.
 *
 * DESCRIPTION
 * Starts latency sampler.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_start_latency_sampler(port_info_t *info)
{
    uint16_t q, rxq;

    /* Start sampler */
    if (pktgen_tst_port_flags(info, SAMPLING_LATENCIES)) {
        pktgen_log_info("Latency sampler is already running, stop it first!");
        return;
    }

    if (info->latsamp_rate == 0 || info->latsamp_type == LATSAMPLER_UNSPEC ||
        info->latsamp_num_samples == 0) {
        pktgen_log_error("Set proper sampling type, number, rate and outfile!");
        return;
    }

    rxq = get_port_rxcnt(pktgen.l2p, info->pid);
    if (rxq == 0 || rxq > MAX_LATENCY_QUEUES) {
        pktgen_log_error("no rx queues or rx queues over limit (%d) to sample on this port!",
                         MAX_LATENCY_QUEUES);
        return;
    }

    for (q = 0; q < rxq; q++) {
        info->latsamp_stats[q].pkt_counter = 0;
        info->latsamp_stats[q].next        = 0;
        info->latsamp_stats[q].idx         = 0;
        info->latsamp_stats[q].num_samples = info->latsamp_num_samples / rxq;
        pktgen_log_info("Assigning %d sample latencies to queue %d",
                        info->latsamp_num_samples / rxq, q);
    }

    if (info->seq_pkt[LATENCY_PKT].pktSize <
        (RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN) + sizeof(tstamp_t))
        info->seq_pkt[LATENCY_PKT].pktSize += sizeof(tstamp_t);

    info->seq_pkt[LATENCY_PKT].ipProto = PG_IPPROTO_UDP;
    pktgen_packet_ctor(info, LATENCY_PKT, -1);
    pktgen_set_tx_update(info);

    /* Start sampling */
    pktgen_set_port_flags(info, SAMPLING_LATENCIES);
}

/**
 *
 * stop_latency_sampler - Stops latency sampler
 *
 * DESCRIPTION
 * Stops latency sampler
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_stop_latency_sampler(port_info_t *info)
{
    FILE *outfile;
    uint32_t i, count;
    uint16_t q, rxq = get_port_rxcnt(pktgen.l2p, info->pid);

    if (pktgen_tst_port_flags(info, SAMPLING_LATENCIES) == 0) {
        pktgen_log_info("Latency sampler is not running, nothing to do!");
        return;
    }

    /* Stop sampling */
    pktgen_clr_port_flags(info, SAMPLING_LATENCIES);

    /* Dump stats to file */
    outfile = fopen(info->latsamp_outfile, "w");
    if (outfile == NULL)
        pktgen_log_error("Cannot open the latcol outfile!");
    else {
        pktgen_log_info("Writing to file %s", info->latsamp_outfile);
        fprintf(outfile, "Latency\n");
        for (q = 0, count = 0; q < rxq; q++) {
            pktgen_log_info("Writing sample latencies of queue %d", q);
            for (i = 0; i < info->latsamp_stats[q].idx; i++) {
                fprintf(outfile, "%" PRIu64 "\n", info->latsamp_stats[q].data[i]);
                count++;
            }
        }
        fclose(outfile);
        pktgen_log_warning("Wrote %d sample latencies to file %s", count, info->latsamp_outfile);
    }

    /* Reset stats data */
    for (q = 0; q < rxq; q++) {
        info->latsamp_stats[q].pkt_counter = 0;
        info->latsamp_stats[q].next        = 0;
        info->latsamp_stats[q].idx         = 0;
        info->latsamp_stats[q].num_samples = 0;
    }

    info->seq_pkt[LATENCY_PKT].ipProto = PG_IPPROTO_UDP;
    pktgen_packet_ctor(info, LATENCY_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * pktgen_prime_ports - Send a small number of packets to setup forwarding tables
 *
 * DESCRIPTION
 * Send a small number of packets from a port to setup the forwarding tables in
 * the device under test.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_prime_ports(port_info_t *info)
{
    uint8_t q;

    for (q = 0; q < get_port_txcnt(pktgen.l2p, info->pid); q++)
        pktgen_set_q_flags(info, q, CLEAR_FAST_ALLOC_FLAG);
    rte_atomic64_set(&info->current_tx_count, info->prime_cnt);
    pktgen_set_port_flags(info, SENDING_PACKETS);
    rte_delay_us_sleep(300 * 1000);
    for (q = 0; q < get_port_txcnt(pktgen.l2p, info->pid); q++)
        pktgen_set_q_flags(info, q, DO_TX_FLUSH);
}

/**
 *
 * single_set_proto - Set up the protocol type for a port/packet.
 *
 * DESCRIPTION
 * Setup all single packets with a protocol types with the port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_proto(port_info_t *info, char *type)
{
    info->seq_pkt[SINGLE_PKT].ipProto = (type[0] == 'u')   ? PG_IPPROTO_UDP
                                        : (type[0] == 'i') ? PG_IPPROTO_ICMP
                                        : (type[0] == 't')
                                            ? PG_IPPROTO_TCP
                                            :
                                            /* TODO print error: unknown type */ PG_IPPROTO_TCP;

    /* ICMP only works on IPv4 packets. */
    if (type[0] == 'i')
        info->seq_pkt[SINGLE_PKT].ethType = RTE_ETHER_TYPE_IPV4;

    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * rate_set_proto - Set up the protocol type for a port/packet.
 *
 * DESCRIPTION
 * Setup all rate packets with a protocol types with the port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_proto(port_info_t *info, char *type)
{
    info->seq_pkt[RATE_PKT].ipProto = (type[0] == 'u')   ? PG_IPPROTO_UDP
                                      : (type[0] == 'i') ? PG_IPPROTO_ICMP
                                      : (type[0] == 't')
                                          ? PG_IPPROTO_TCP
                                          :
                                          /* TODO print error: unknown type */ PG_IPPROTO_TCP;

    /* ICMP only works on IPv4 packets. */
    if (type[0] == 'i')
        info->seq_pkt[RATE_PKT].ethType = RTE_ETHER_TYPE_IPV4;

    pktgen_packet_ctor(info, RATE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * range_set_proto - Set up the protocol type for a port/packet.
 *
 * DESCRIPTION
 * Setup all range packets with a protocol types with the port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_proto(port_info_t *info, const char *type)
{
    info->seq_pkt[RANGE_PKT].ipProto = (type[0] == 'u')   ? PG_IPPROTO_UDP
                                       : (type[0] == 'i') ? PG_IPPROTO_ICMP
                                       : (type[0] == 't')
                                           ? PG_IPPROTO_TCP
                                           :
                                           /* TODO print error: unknown type */ PG_IPPROTO_TCP;
    info->range.ip_proto             = info->seq_pkt[RANGE_PKT].ipProto;

    /* ICMP only works on IPv4 packets. */
    if (type[0] == 'i')
        info->seq_pkt[RANGE_PKT].ethType = RTE_ETHER_TYPE_IPV4;
}

/**
 *
 * enable_pcap - Enable or disable PCAP sending of packets.
 *
 * DESCRIPTION
 * Enable or disable PCAP packet sending.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_pcap(port_info_t *info, uint32_t state)
{
    if ((info->pcap != NULL) && (info->pcap->pkt_count != 0)) {
        if (state == ENABLE_STATE) {
            pktgen_clr_port_flags(info, EXCLUSIVE_MODES);
            pktgen_set_port_flags(info, SEND_PCAP_PKTS);
        } else
            pktgen_clr_port_flags(info, SEND_PCAP_PKTS);
        info->tx_cycles = 0;
    }
}

/**
 *
 *
 * DESCRIPTION
 * Enable or disable Rate packet sending.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_rate(port_info_t *info, uint32_t state)
{
    if (state == ENABLE_STATE) {
        pktgen_clr_port_flags(info, EXCLUSIVE_MODES);
        pktgen_set_port_flags(info, SEND_RATE_PACKETS);

        single_set_proto(info, (char *)(uintptr_t) "udp");
        update_rate_values(info);
    } else {
        pkt_seq_t *pkt = &info->seq_pkt[RATE_PKT];

        pktgen_clr_port_flags(info, SEND_RATE_PACKETS);
        info->rx_burst = DEFAULT_PKT_RX_BURST;
        info->tx_burst = DEFAULT_PKT_TX_BURST;
        pkt->pktSize   = RTE_ETHER_MIN_LEN - RTE_ETHER_CRC_LEN;

        pktgen_packet_rate(info);
    }
}

/**
 *
 * pcap_filter - Compile a PCAP filter for a portlist
 *
 * DESCRIPTION
 * Compile a pcap filter for a portlist
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pcap_filter(port_info_t *info, char *str)
{
    pcap_t *pc = pcap_open_dead(DLT_EN10MB, 65535);

    info->pcap_result = pcap_compile(pc, &info->pcap_program, str, 1, PCAP_NETMASK_UNKNOWN);

    pcap_close(pc);
}

/**
 *
 * debug_blink - Enable or disable a port from blinking.
 *
 * DESCRIPTION
 * Enable or disable the given ports from blinking.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
debug_blink(port_info_t *info, uint32_t state)
{
    if (state == ENABLE_STATE)
        pktgen.blinklist |= (1 << info->pid);
    else {
        pktgen.blinklist &= ~(1 << info->pid);
        rte_eth_led_on(info->pid);
    }
}

/**
 *
 * enable_process - Enable or disable input packet processing.
 *
 * DESCRIPTION
 * Enable or disable input packet processing of ICMP, ARP, ...
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_process(port_info_t *info, int state)
{
    if (state == ENABLE_STATE)
        pktgen_set_port_flags(info, PROCESS_INPUT_PKTS);
    else
        pktgen_clr_port_flags(info, PROCESS_INPUT_PKTS);
}

/**
 *
 * enable_capture - Enable or disable capture packet processing.
 *
 * DESCRIPTION
 * Enable or disable capture packet processing of ICMP, ARP, ...
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_capture(port_info_t *info, uint32_t state)
{
    pktgen_set_capture(info, state);
}

#if defined(RTE_LIBRTE_PMD_BOND) || defined(RTE_NET_BOND)
/**
 *
 * enable_bonding - Enable or disable bonding TX zero packet processing.
 *
 * DESCRIPTION
 * Enable or disable calling TX with zero packets for bonding PMD
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_bonding(port_info_t *info, uint32_t state)
{
    struct rte_eth_bond_8023ad_conf conf;
    uint16_t workers[RTE_MAX_ETHPORTS];
    uint16_t active_workers[RTE_MAX_ETHPORTS];
    int i, num_workers, num_active_workers;

    if (rte_eth_bond_8023ad_conf_get(info->pid, &conf) < 0) {
        printf("Port %d is not a bonding port\n", info->pid);
        return;
    }

    num_workers = rte_eth_bond_members_get(info->pid, workers, RTE_MAX_ETHPORTS);
    if (num_workers < 0) {
        printf("Failed to get worker list for port = %d\n", info->pid);
        return;
    }

    num_active_workers =
        rte_eth_bond_active_members_get(info->pid, active_workers, RTE_MAX_ETHPORTS);
    if (num_active_workers < 0) {
        printf("Failed to get active worker list for port = %d\n", info->pid);
        return;
    }

    printf("Port %d:\n", info->pid);
    for (i = 0; i < num_workers; i++) {
        if (state == ENABLE_STATE) {
            pktgen_set_port_flags(info, BONDING_TX_PACKETS);
            rte_eth_bond_8023ad_ext_distrib(info->pid, workers[i], 1);
            printf("   Enable worker %u 802.3ad distributing\n", workers[i]);
            rte_eth_bond_8023ad_ext_collect(info->pid, workers[i], 1);
            printf("   Enable worker %u 802.3ad collecting\n", workers[i]);
        } else {
            pktgen_clr_port_flags(info, BONDING_TX_PACKETS);
            rte_eth_bond_8023ad_ext_distrib(info->pid, workers[i], 0);
            printf("   Disable worker %u 802.3ad distributing\n", workers[i]);
            rte_eth_bond_8023ad_ext_collect(info->pid, workers[i], 1);
            printf("   Enable worker %u 802.3ad collecting\n", workers[i]);
        }
    }
}

static void
show_states(uint8_t state)
{
    const char *states[] = {"LACP_Active", "LACP_Short_timeout", "Aggregation", "Synchronization",
                            "Collecting",  "Distributing",       "Defaulted",   "Expired",
                            NULL};
    int j;

    for (j = 0; states[j]; j++) {
        if (state & (1 << j))
            printf("%s ", states[j]);
    }
}

void
show_bonding_mode(port_info_t *info)
{
    int bonding_mode, agg_mode;
    uint16_t workers[RTE_MAX_ETHPORTS];
    int num_workers, num_active_workers;
    int primary_id;
    int i;
    uint16_t port_id = info->pid;

    /* Display the bonding mode.*/
    bonding_mode = rte_eth_bond_mode_get(port_id);
    if (bonding_mode < 0) {
        printf("Failed to get bonding mode for port = %d\n", port_id);
        return;
    } else
        printf("\tBonding mode: %d, ", bonding_mode);

    if (bonding_mode == BONDING_MODE_BALANCE) {
        int balance_xmit_policy;

        balance_xmit_policy = rte_eth_bond_xmit_policy_get(port_id);
        if (balance_xmit_policy < 0) {
            printf("\nFailed to get balance xmit policy for port = %d\n", port_id);
            return;
        } else {
            printf("Balance Xmit Policy: ");

            switch (balance_xmit_policy) {
            case BALANCE_XMIT_POLICY_LAYER2:
                printf("BALANCE_XMIT_POLICY_LAYER2");
                break;
            case BALANCE_XMIT_POLICY_LAYER23:
                printf("BALANCE_XMIT_POLICY_LAYER23");
                break;
            case BALANCE_XMIT_POLICY_LAYER34:
                printf("BALANCE_XMIT_POLICY_LAYER34");
                break;
            }
            printf(", ");
        }
    }

    if (bonding_mode == BONDING_MODE_8023AD) {
        agg_mode = rte_eth_bond_8023ad_agg_selection_get(port_id);
        printf("IEEE802.3AD Aggregator Mode: ");
        switch (agg_mode) {
        case AGG_BANDWIDTH:
            printf("bandwidth");
            break;
        case AGG_STABLE:
            printf("stable");
            break;
        case AGG_COUNT:
            printf("count");
            break;
        }
        printf("\n");
    }

    num_workers = rte_eth_bond_members_get(port_id, workers, RTE_MAX_ETHPORTS);

    if (num_workers < 0) {
        printf("\tFailed to get worker list for port = %d\n", port_id);
        return;
    }
    if (num_workers > 0) {
        printf("\tSlaves (%d): [", num_workers);
        for (i = 0; i < num_workers - 1; i++)
            printf("%d ", workers[i]);

        printf("%d]\n", workers[num_workers - 1]);
    } else {
        printf("\tSlaves: []\n");
    }

    num_active_workers = rte_eth_bond_active_members_get(port_id, workers, RTE_MAX_ETHPORTS);

    if (num_active_workers < 0) {
        printf("\tFailed to get active worker list for port = %d\n", port_id);
        return;
    }
    if (num_active_workers > 0) {
        printf("\tActive Slaves (%d): [", num_active_workers);
        for (i = 0; i < num_active_workers - 1; i++)
            printf("%d ", workers[i]);

        printf("%d]\n", workers[num_active_workers - 1]);

    } else {
        printf("\tActive Slaves: []\n");
    }

    for (i = 0; i < num_active_workers; i++) {
        struct rte_eth_bond_8023ad_member_info conf;

        printf("\t\tSlave %u\n", workers[i]);
        rte_eth_bond_8023ad_member_info(info->pid, workers[i], &conf);
        printf("\t\t  %sSelected\n\t\t  Actor States  ( ", conf.selected ? "" : "Not ");
        show_states(conf.actor_state);
        printf(")\n\t\t  Partner States( ");
        show_states(conf.partner_state);
        printf(")\n\t\t  AGG Port %u\n", conf.agg_port_id);
    }

    primary_id = rte_eth_bond_primary_get(port_id);
    if (primary_id < 0) {
        printf("\tFailed to get primary worker for port = %d\n", port_id);
        return;
    } else
        printf("\tPrimary: [%d]\n", primary_id);
}
#endif

/**
 *
 * range_set_pkt_type - Set the packet type value for range packets.
 *
 * DESCRIPTION
 * Set the packet type value for the given port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_pkt_type(port_info_t *info, const char *type)
{
    info->seq_pkt[RANGE_PKT].ethType = (type[0] == 'a')   ? RTE_ETHER_TYPE_ARP
                                       : (type[3] == '4') ? RTE_ETHER_TYPE_IPV4
                                       : (type[3] == '6')
                                           ? RTE_ETHER_TYPE_IPV6
                                           :
                                           /* TODO print error: unknown type */ RTE_ETHER_TYPE_IPV4;
    if (info->seq_pkt[RANGE_PKT].ethType == RTE_ETHER_TYPE_IPV6) {
        if (info->range.pkt_size < MIN_v6_PKT_SIZE)
            info->range.pkt_size = MIN_v6_PKT_SIZE;
        if (info->range.pkt_size_min < MIN_v6_PKT_SIZE)
            info->range.pkt_size_min = MIN_v6_PKT_SIZE;
        if (info->range.pkt_size_max < MIN_v6_PKT_SIZE)
            info->range.pkt_size_max = MIN_v6_PKT_SIZE;
    }
}

/**
 *
 * single_set_pkt_type - Set the packet type value.
 *
 * DESCRIPTION
 * Set the packet type value for the given port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_pkt_type(port_info_t *info, const char *type)
{
    pkt_seq_t *pkt   = &info->seq_pkt[SINGLE_PKT];
    uint16_t ethtype = pkt->ethType;

    pkt->ethType = (type[0] == 'a')   ? RTE_ETHER_TYPE_ARP
                   : (type[3] == '4') ? RTE_ETHER_TYPE_IPV4
                   : (type[3] == '6') ? RTE_ETHER_TYPE_IPV6
                   : (type[2] == '4') ? RTE_ETHER_TYPE_IPV4
                   : (type[2] == '6') ? RTE_ETHER_TYPE_IPV6
                                      :
                                      /* TODO print error: unknown type */ RTE_ETHER_TYPE_IPV4;

    if ((ethtype == RTE_ETHER_TYPE_IPV6) && (pkt->ethType == RTE_ETHER_TYPE_IPV4)) {
        if (pkt->pktSize >= MIN_v6_PKT_SIZE)
            pkt->pktSize = MIN_PKT_SIZE + (pkt->pktSize - MIN_v6_PKT_SIZE);
    }
    if ((ethtype == RTE_ETHER_TYPE_IPV4) && (pkt->ethType == RTE_ETHER_TYPE_IPV6)) {
        if (pkt->pktSize < MIN_v6_PKT_SIZE)
            pkt->pktSize = MIN_v6_PKT_SIZE + (pkt->pktSize - MIN_PKT_SIZE);
    }

    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * rate_set_pkt_type - Set the packet type value.
 *
 * DESCRIPTION
 * Set the packet type value for the given port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_pkt_type(port_info_t *info, const char *type)
{
    pkt_seq_t *pkt   = &info->seq_pkt[RATE_PKT];
    uint16_t ethtype = pkt->ethType;

    pkt->ethType = (type[0] == 'a')   ? RTE_ETHER_TYPE_ARP
                   : (type[3] == '4') ? RTE_ETHER_TYPE_IPV4
                   : (type[3] == '6') ? RTE_ETHER_TYPE_IPV6
                   : (type[2] == '4') ? RTE_ETHER_TYPE_IPV4
                   : (type[2] == '6') ? RTE_ETHER_TYPE_IPV6
                                      :
                                      /* TODO print error: unknown type */ RTE_ETHER_TYPE_IPV4;

    if ((ethtype == RTE_ETHER_TYPE_IPV6) && (pkt->ethType == RTE_ETHER_TYPE_IPV4)) {
        if (pkt->pktSize >= MIN_v6_PKT_SIZE)
            pkt->pktSize = MIN_PKT_SIZE + (pkt->pktSize - MIN_v6_PKT_SIZE);
    }
    if ((ethtype == RTE_ETHER_TYPE_IPV4) && (pkt->ethType == RTE_ETHER_TYPE_IPV6)) {
        if (pkt->pktSize < MIN_v6_PKT_SIZE)
            pkt->pktSize = MIN_v6_PKT_SIZE + (pkt->pktSize - MIN_PKT_SIZE);
    }

    pktgen_packet_ctor(info, RATE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * enable_vxlan - Set the port to send a VxLAN ID
 *
 * DESCRIPTION
 * Set the given port list to send VxLAN ID packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_vxlan(port_info_t *info, uint32_t onOff)
{
    if (onOff == ENABLE_STATE) {
        pktgen_clr_port_flags(info, EXCLUSIVE_PKT_MODES);
        pktgen_set_port_flags(info, SEND_VXLAN_PACKETS);
    } else
        pktgen_clr_port_flags(info, SEND_VXLAN_PACKETS);
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * enable_vlan - Set the port to send a VLAN ID
 *
 * DESCRIPTION
 * Set the given port list to send VLAN ID packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_vlan(port_info_t *info, uint32_t onOff)
{
    if (onOff == ENABLE_STATE) {
        pktgen_clr_port_flags(info, EXCLUSIVE_PKT_MODES);
        pktgen_set_port_flags(info, SEND_VLAN_ID);
    } else
        pktgen_clr_port_flags(info, SEND_VLAN_ID);
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_vlan_id - Set the port VLAN ID value
 *
 * DESCRIPTION
 * Set the given port list with the given VLAN ID.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_vlan_id(port_info_t *info, uint16_t vlanid)
{
    info->vlanid                     = vlanid;
    info->seq_pkt[SINGLE_PKT].vlanid = info->vlanid;
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_prio - Set the port 802.1p cos value
 *
 * DESCRIPTION
 * Set the given port list with the given 802.1p cos
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_cos(port_info_t *info, uint8_t cos)
{
    info->cos                     = cos;
    info->seq_pkt[SINGLE_PKT].cos = info->cos;
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_tos - Set the port tos value
 *
 * DESCRIPTION
 * Set the given port list with the given tos
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_tos(port_info_t *info, uint8_t tos)
{
    info->tos                     = tos;
    info->seq_pkt[SINGLE_PKT].tos = info->tos;
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_vxlan - Set the port vxlan value
 *
 * DESCRIPTION
 * Set the given port list with the given vxlan
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_vxlan(port_info_t *info, uint16_t flags, uint16_t group_id, uint32_t vxlan_id)
{
    info->vni_flags                     = flags;
    info->group_id                      = group_id;
    info->vxlan_id                      = vxlan_id;
    info->seq_pkt[SINGLE_PKT].vni_flags = info->vni_flags;
    info->seq_pkt[SINGLE_PKT].group_id  = info->group_id;
    info->seq_pkt[SINGLE_PKT].vxlan_id  = info->vxlan_id;
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_latsamp_params - Set the port latency sampler parameters
 *
 * DESCRIPTION
 * Set the given port list with the given latency sampler parameters
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_latsampler_params(port_info_t *info, char *type, uint32_t num_samples,
                             uint32_t sampling_rate, char outfile[])
{
    FILE *fp = NULL;
    uint32_t sampler_type;

    /* Stop if latency sampler is running */
    if (pktgen_tst_port_flags(info, SAMPLING_LATENCIES)) {
        pktgen_log_warning("Latency sampler is already running, stop it first!");
        return;
    }
    /* Validate sampler type*/
    if (!strcasecmp(type, "simple"))
        sampler_type = LATSAMPLER_SIMPLE;
    else if (!strcasecmp(type, "poisson"))
        sampler_type = LATSAMPLER_POISSON;
    else {
        pktgen_log_error("Unknown latsampler type %s! Valid values: simple, poisson", type);
        return;
    }

    /* Validate file path */
    fp = fopen(outfile, "w+");
    if (fp == NULL) {
        pktgen_log_error("Cannot write to file path %s!", outfile);
        return;
    }
    fclose(fp);

    if (num_samples > MAX_LATENCY_ENTRIES) {
        pktgen_log_error("Too many samples requested. Max %d!", MAX_LATENCY_ENTRIES);
        return;
    }

    info->latsamp_type        = sampler_type;
    info->latsamp_rate        = sampling_rate;
    info->latsamp_num_samples = num_samples;
    strcpy(info->latsamp_outfile, outfile);

    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * enable_mpls - Set the port to send a mpls ID
 *
 * DESCRIPTION
 * Set the given port list to send mpls ID packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_mpls(port_info_t *info, uint32_t onOff)
{
    if (onOff == ENABLE_STATE) {
        pktgen_clr_port_flags(info, EXCLUSIVE_PKT_MODES);
        pktgen_set_port_flags(info, SEND_MPLS_LABEL);
    } else
        pktgen_clr_port_flags(info, SEND_MPLS_LABEL);
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * range_set_mpls_entry - Set the port MPLS entry value
 *
 * DESCRIPTION
 * Set the given port list with the given MPLS entry.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_mpls_entry(port_info_t *info, uint32_t mpls_entry)
{
    info->mpls_entry                     = mpls_entry;
    info->seq_pkt[SINGLE_PKT].mpls_entry = info->mpls_entry;
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * enable_qinq - Set the port to send a Q-in-Q header
 *
 * DESCRIPTION
 * Set the given port list to send Q-in-Q ID packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_qinq(port_info_t *info, uint32_t onOff)
{
    if (onOff == ENABLE_STATE) {
        pktgen_clr_port_flags(info, EXCLUSIVE_MODES);
        pktgen_clr_port_flags(info, EXCLUSIVE_PKT_MODES);
        pktgen_set_port_flags(info, SEND_Q_IN_Q_IDS);
    } else
        pktgen_clr_port_flags(info, SEND_Q_IN_Q_IDS);
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_qinqids - Set the port Q-in-Q ID values
 *
 * DESCRIPTION
 * Set the given port list with the given Q-in-Q ID's.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_qinqids(port_info_t *info, uint16_t outerid, uint16_t innerid)
{
    info->seq_pkt[SINGLE_PKT].qinq_outerid = outerid;
    info->seq_pkt[SINGLE_PKT].qinq_innerid = innerid;

    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * range_set_qinqids - Set the port Q-in-Q ID values
 *
 * DESCRIPTION
 * Set the given port list with the given Q-in-Q ID's.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_qinqids(port_info_t *info, uint16_t outerid, uint16_t innerid)
{
    info->seq_pkt[RANGE_PKT].qinq_outerid = outerid;
    info->seq_pkt[RANGE_PKT].qinq_innerid = innerid;

    pktgen_packet_ctor(info, RANGE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * enable_gre - Set the port to send GRE with IPv4 payload
 *
 * DESCRIPTION
 * Set the given port list to send GRE with IPv4 payload
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_gre(port_info_t *info, uint32_t onOff)
{
    if (onOff == ENABLE_STATE) {
        pktgen_clr_port_flags(info, EXCLUSIVE_PKT_MODES);
        pktgen_set_port_flags(info, SEND_GRE_IPv4_HEADER);
    } else
        pktgen_clr_port_flags(info, SEND_GRE_IPv4_HEADER);
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * pktgen_set_gre_eth - Set the port to send GRE with Ethernet payload
 *
 * DESCRIPTION
 * Set the given port list to send GRE with Ethernet payload
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_gre_eth(port_info_t *info, uint32_t onOff)
{
    if (onOff == ENABLE_STATE) {
        pktgen_clr_port_flags(info, EXCLUSIVE_PKT_MODES);
        pktgen_set_port_flags(info, SEND_GRE_ETHER_HEADER);
    } else
        pktgen_clr_port_flags(info, SEND_GRE_ETHER_HEADER);
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * range_set_gre_key - Set the port GRE key
 *
 * DESCRIPTION
 * Set the given port list with the given GRE key.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_gre_key(port_info_t *info, uint32_t gre_key)
{
    info->gre_key                     = gre_key;
    info->seq_pkt[SINGLE_PKT].gre_key = info->gre_key;
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * pktgen_clear_stats - Clear a given port list of stats.
 *
 * DESCRIPTION
 * Clear the given port list of all statistics.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_clear_stats(port_info_t *info)
{
    eth_stats_t *base;

    /* curr_stats are reset each time the stats are read */
    memset(&info->curr_stats, 0, sizeof(eth_stats_t));
    memset(&info->queue_stats, 0, sizeof(eth_stats_t));
    memset(&info->rate_stats, 0, sizeof(eth_stats_t));
    memset(&info->prev_stats, 0, sizeof(eth_stats_t));
    memset(&info->base_stats, 0, sizeof(eth_stats_t));

    base = &info->base_stats;

    /* Normalize the stats to a zero base line */
    rte_eth_stats_get(info->pid, base);

    pktgen.max_total_ipackets = 0;
    pktgen.max_total_opackets = 0;
    info->max_ipackets        = 0;
    info->max_opackets        = 0;
    info->max_missed          = 0;

    memset(&info->pkt_stats, 0, sizeof(info->pkt_stats));
    memset(&info->pkt_sizes, 0, sizeof(info->pkt_sizes));

    latency_t *lat = &info->latency;

    memset(lat->stats, 0, (lat->end_stats - lat->stats) * sizeof(uint64_t));

    memset(&pktgen.cumm_rate_totals, 0, sizeof(eth_stats_t));
}

/**
 *
 * pktgen_port_defaults - Set all ports back to the default values.
 *
 * DESCRIPTION
 * Reset the ports back to the defaults.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_port_defaults(uint32_t pid, uint8_t seq)
{
    port_info_t *info = &pktgen.info[pid];
    pkt_seq_t *pkt    = &info->seq_pkt[seq];
    port_info_t *dst_info;

    pkt->pktSize   = MIN_PKT_SIZE;
    pkt->sport     = DEFAULT_SRC_PORT;
    pkt->dport     = DEFAULT_DST_PORT;
    pkt->ttl       = DEFAULT_TTL;
    pkt->ipProto   = PG_IPPROTO_TCP;
    pkt->ethType   = RTE_ETHER_TYPE_IPV4;
    pkt->vlanid    = DEFAULT_VLAN_ID;
    pkt->cos       = DEFAULT_COS;
    pkt->tos       = DEFAULT_TOS;
    pkt->tcp_flags = DEFAULT_TCP_FLAGS;

    rte_atomic64_set(&info->transmit_count, DEFAULT_TX_COUNT);
    rte_atomic64_init(&info->current_tx_count);
    info->tx_rate   = DEFAULT_TX_RATE;
    info->tx_burst  = DEFAULT_PKT_TX_BURST;
    info->rx_burst  = DEFAULT_PKT_RX_BURST;
    info->vlanid    = DEFAULT_VLAN_ID;
    info->cos       = DEFAULT_COS;
    info->tos       = DEFAULT_TOS;
    info->seqCnt    = 0;
    info->seqIdx    = 0;
    info->prime_cnt = DEFAULT_PRIME_COUNT;
    info->delta     = 0;

    pkt->ip_mask = DEFAULT_NETMASK;
    if ((pid & 1) == 0) {
        pkt->ip_src_addr.addr.ipv4.s_addr = DEFAULT_IP_ADDR | (pid << 8) | 1;
        pkt->ip_dst_addr.addr.ipv4.s_addr = DEFAULT_IP_ADDR | ((pid + 1) << 8) | 1;
        dst_info                          = info + 1;
    } else {
        pkt->ip_src_addr.addr.ipv4.s_addr = DEFAULT_IP_ADDR | (pid << 8) | 1;
        pkt->ip_dst_addr.addr.ipv4.s_addr = DEFAULT_IP_ADDR | ((pid - 1) << 8) | 1;
        dst_info                          = info - 1;
    }

    if (dst_info->seq_pkt != NULL) {
        rte_ether_addr_copy(&dst_info->seq_pkt[SINGLE_PKT].eth_src_addr, &pkt->eth_dst_addr);
        rte_ether_addr_copy(&dst_info->seq_pkt[RATE_PKT].eth_src_addr, &pkt->eth_dst_addr);
        rte_ether_addr_copy(&dst_info->seq_pkt[LATENCY_PKT].eth_src_addr, &pkt->eth_dst_addr);
    } else
        memset(&pkt->eth_dst_addr, 0, sizeof(pkt->eth_dst_addr));

    pktgen_packet_ctor(info, seq, -1);
    pktgen_set_tx_update(info);

    pktgen.flags |= PRINT_LABELS_FLAG;
}

/**
 *
 * pktgen_ping4 - Send a IPv4 ICMP echo request.
 *
 * DESCRIPTION
 * Send a IPv4 ICMP echo request packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_ping4(port_info_t *info)
{
    memcpy(&info->seq_pkt[PING_PKT], &info->seq_pkt[SINGLE_PKT], sizeof(pkt_seq_t));
    info->seq_pkt[PING_PKT].ipProto = PG_IPPROTO_ICMP;
    pktgen_packet_ctor(info, PING_PKT, ICMP4_ECHO);
    pktgen_set_port_flags(info, SEND_PING4_REQUEST);
    pktgen_set_tx_update(info);
}

/**
 *
 * debug_pdump - Dump hex output of first packet
 *
 * DESCRIPTION
 * Hex dump the first packets on a given port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
debug_pdump(port_info_t *info)
{
    pkt_seq_t *ppkt = &info->seq_pkt[DUMP_PKT];
    pkt_seq_t *spkt = &info->seq_pkt[SINGLE_PKT];
    struct rte_mbuf *m;

    m = rte_pktmbuf_alloc(info->special_mp);
    if (unlikely(m == NULL)) {
        pktgen_log_warning("No packet buffers found");
        return;
    }
    *ppkt = *spkt; /* Copy the sequence setup to the dump setup. */
    pktgen_packet_ctor(info, DUMP_PKT, -1);
    rte_memcpy((uint8_t *)m->buf_addr + m->data_off, (uint8_t *)&ppkt->hdr, ppkt->pktSize);

    m->pkt_len  = ppkt->pktSize;
    m->data_len = ppkt->pktSize;
    m->ol_flags = ppkt->ol_flags;

    pg_pktmbuf_dump(stdout, m, m->pkt_len);
    rte_pktmbuf_free(m);
}

#ifdef INCLUDE_PING6
/**
 *
 * pktgen_ping6 - Send a IPv6 ICMP echo request packet.
 *
 * DESCRIPTION
 * Send a IPv6 ICMP echo request packet for the given ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_ping6(port_info_t *info)
{
    memcpy(&info->pkt[PING_PKT], &info->pkt[SINGLE_PKT], sizeof(pkt_seq_t));
    info->pkt[PING_PKT].ipProto = PG_IPPROTO_ICMP;
    pktgen_packet_ctor(info, PING_PKT, ICMP6_ECHO);
    pktgen_set_port_flags(info, SEND_PING6_REQUEST);
    pktgen_set_tx_update(info);
}

#endif

/**
 *
 * pktgen_reset - Reset all ports to the default state
 *
 * DESCRIPTION
 * Reset all ports to the default state.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_reset(port_info_t *info)
{
    uint32_t s;
    char off[8];

    if (info == NULL)
        info = &pktgen.info[0];

    printf("Reset configuration to default %d\n", info->pid);

    strcpy(off, "off");
    pktgen_stop_transmitting(info);

    pktgen.flags &= ~MAC_FROM_ARP_FLAG;

    /* Make sure the port is active and enabled. */
    if (info->seq_pkt) {
        info->seq_pkt[SINGLE_PKT].pktSize = MIN_PKT_SIZE;

        for (s = 0; s < NUM_TOTAL_PKTS; s++)
            pktgen_port_defaults(info->pid, s);

        pktgen_range_setup(info);
        pktgen_rate_setup(info);
        pktgen_clear_stats(info);

        enable_range(info, estate(off));
        enable_rate(info, estate(off));
        enable_latency(info, estate(off));
        memset(info->rnd_bitfields, 0, sizeof(struct rnd_bits_s));
        pktgen_rnd_bits_init(&info->rnd_bitfields);
        pktgen_set_port_seqCnt(info, 0);
    }

    pktgen_update_display();
}

/**
 *
 * pktgen_port_restart - Attempt to cleanup port state.
 *
 * DESCRIPTION
 * Reset all ports
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_port_restart(port_info_t *info)
{
    if (info == NULL)
        info = &pktgen.info[0];

    printf("Port %d attempt to stop/start PMD\n", info->pid);

    pktgen_set_receive_state(info, 1);

    pktgen_stop_transmitting(info);

    rte_delay_us_sleep(10 * 1000);

    /* Stop and start the device to flush TX and RX buffers from the device rings. */
    if (rte_eth_dev_stop(info->pid) < 0)
        printf("Unable to stop device %d\n", info->pid);

    rte_delay_us_sleep(250);

    if (rte_eth_dev_start(info->pid) < 0)
        printf("Unable to start device %d\n", info->pid);

    pktgen_set_receive_state(info, 0);

    pktgen_update_display();
}

/**
 *
 * single_set_tx_count - Set the number of packets to transmit on a port.
 *
 * DESCRIPTION
 * Set the transmit count for all ports in the list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_tx_count(port_info_t *info, uint32_t cnt)
{
    rte_atomic64_set(&info->transmit_count, cnt);
}

/**
 *
 * rate_set_tx_count - Set the number of packets to transmit on a port.
 *
 * DESCRIPTION
 * Set the transmit count for all ports in the list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_tx_count(port_info_t *info, uint32_t cnt)
{
    rte_atomic64_set(&info->transmit_count, cnt);
}

/**
 *
 * pktgen_set_port_seqCnt - Set the sequence count for a port
 *
 * DESCRIPTION
 * Set a sequence count of packets for all ports in the list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_set_port_seqCnt(port_info_t *info, uint32_t cnt)
{
    if (cnt > NUM_SEQ_PKTS)
        cnt = NUM_SEQ_PKTS;

    info->seqCnt = cnt;
    if (cnt) {
        pktgen_clr_port_flags(info, EXCLUSIVE_MODES);
        pktgen_set_port_flags(info, SEND_SEQ_PKTS);
    } else
        pktgen_clr_port_flags(info, SEND_SEQ_PKTS);
}

/**
 *
 * pktgen_set_port_prime - Set the number of packets to send on a prime command
 *
 * DESCRIPTION
 * Set the number packets to send on the prime command for all ports in list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_set_port_prime(port_info_t *info, uint32_t cnt)
{
    if (cnt > MAX_PRIME_COUNT)
        cnt = MAX_PRIME_COUNT;
    else if (cnt == 0)
        cnt = DEFAULT_PRIME_COUNT;

    info->prime_cnt = cnt;
}

/**
 *
 * pktgen_set_port_dump - Set the number of received packets to dump to screen.
 *
 * DESCRIPTION
 * Set the number of received packets to dump to screen.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
debug_set_port_dump(port_info_t *info, uint32_t cnt)
{
    int i;

    if (cnt > MAX_DUMP_PACKETS)
        cnt = MAX_DUMP_PACKETS;

    /* Prevent concurrency issues by setting the fields in this specific order */
    info->dump_count = 0;
    info->dump_tail  = 0;
    info->dump_head  = 0;

    for (i = 0; i < MAX_DUMP_PACKETS; ++i)
        if (info->dump_list->data != NULL) {
            rte_free(info->dump_list->data);
            info->dump_list->data = NULL;
        }

    info->dump_count = cnt;
}

/**
 *
 * single_set_tx_burst - Set the transmit burst count.
 *
 * DESCRIPTION
 * Set the transmit burst count for all packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_tx_burst(port_info_t *info, uint32_t burst)
{
    if (burst == 0)
        burst = 1;
    else if (burst > MAX_PKT_TX_BURST)
        burst = MAX_PKT_TX_BURST;
    info->tx_burst  = burst;
    info->tx_cycles = 0;

    pktgen_packet_rate(info);
}

/**
 *
 * single_set_rx_burst - Set the receive burst count.
 *
 * DESCRIPTION
 * Set the receive burst count for all packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_rx_burst(port_info_t *info, uint32_t burst)
{
    if (burst == 0)
        burst = 1;
    else if (burst > MAX_PKT_RX_BURST)
        burst = MAX_PKT_RX_BURST;
    info->rx_burst = burst;

    pktgen_packet_rate(info);
}

/**
 *
 * rate_set_tx_burst - Set the transmit burst count.
 *
 * DESCRIPTION
 * Set the transmit burst count for all packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_tx_burst(port_info_t *info, uint32_t burst)
{
    if (burst == 0)
        burst = 1;
    else if (burst > MAX_PKT_TX_BURST)
        burst = MAX_PKT_TX_BURST;
    info->tx_burst  = burst;
    info->tx_cycles = 0;

    pktgen_packet_rate(info);
}

/**
 *
 * rate_set_tx_burst - Set the transmit burst count.
 *
 * DESCRIPTION
 * Set the transmit burst count for all packets.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_rx_burst(port_info_t *info, uint32_t burst)
{
    if (burst == 0)
        burst = 1;
    else if (burst > MAX_PKT_RX_BURST)
        burst = MAX_PKT_RX_BURST;
    info->rx_burst = burst;

    pktgen_packet_rate(info);
}

/**
 *
 * debug_set_tx_cycles - Set the number of Transmit cycles to use.
 *
 * DESCRIPTION
 * Set the number of transmit cycles for the given port list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
debug_set_tx_cycles(port_info_t *info, uint32_t cycles)
{
    info->tx_cycles = cycles;
}

/**
 *
 * single_set_pkt_size - Set the size of the packets to send.
 *
 * DESCRIPTION
 * Set the pkt size for the single packet transmit.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_pkt_size(port_info_t *info, uint16_t size)
{
    pkt_seq_t *pkt = &info->seq_pkt[SINGLE_PKT];

    if (size < RTE_ETHER_CRC_LEN)
        size = RTE_ETHER_CRC_LEN;

    if ((size - RTE_ETHER_CRC_LEN) < MIN_PKT_SIZE)
        size = pktgen.eth_min_pkt;
    if ((size - RTE_ETHER_CRC_LEN) > MAX_PKT_SIZE)
        size = pktgen.eth_max_pkt;

    if ((pkt->ethType == RTE_ETHER_TYPE_IPV6) && (size < (MIN_v6_PKT_SIZE + RTE_ETHER_CRC_LEN)))
        size = MIN_v6_PKT_SIZE + RTE_ETHER_CRC_LEN;

    pkt->pktSize = (size - RTE_ETHER_CRC_LEN);

    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_packet_rate(info);
    pktgen_set_tx_update(info);
}

/**
 *
 * rate_set_pkt_size - Set the size of the packets to send.
 *
 * DESCRIPTION
 * Set the pkt size for the single packet transmit.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_pkt_size(port_info_t *info, uint16_t size)
{
    pkt_seq_t *pkt = &info->seq_pkt[RATE_PKT];

    if (size < RTE_ETHER_CRC_LEN)
        size = RTE_ETHER_CRC_LEN;

    if ((size - RTE_ETHER_CRC_LEN) < MIN_PKT_SIZE)
        size = pktgen.eth_min_pkt;
    if ((size - RTE_ETHER_CRC_LEN) > MAX_PKT_SIZE)
        size = pktgen.eth_max_pkt;

    if ((pkt->ethType == RTE_ETHER_TYPE_IPV6) && (size < (MIN_v6_PKT_SIZE + RTE_ETHER_CRC_LEN)))
        size = MIN_v6_PKT_SIZE + RTE_ETHER_CRC_LEN;

    pkt->pktSize = (size - RTE_ETHER_CRC_LEN);

    pktgen_packet_ctor(info, RATE_PKT, -1);
    pktgen_packet_rate(info);
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_port_value - Set the port value for single or sequence packets.
 *
 * DESCRIPTION
 * Set the port value for single or sequence packets for the ports listed.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_port_value(port_info_t *info, char type, uint32_t portValue)
{
    if (type == 'd')
        info->seq_pkt[SINGLE_PKT].dport = (uint16_t)portValue;
    else
        info->seq_pkt[SINGLE_PKT].sport = (uint16_t)portValue;
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * rate_set_port_value - Set the port value for single or sequence packets.
 *
 * DESCRIPTION
 * Set the port value for single or sequence packets for the ports listed.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_port_value(port_info_t *info, char type, uint32_t portValue)
{
    if (type == 'd')
        info->seq_pkt[RATE_PKT].dport = (uint16_t)portValue;
    else
        info->seq_pkt[RATE_PKT].sport = (uint16_t)portValue;
    pktgen_packet_ctor(info, RATE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_tx_rate - Set the transmit rate as a percent value.
 *
 * DESCRIPTION
 * Set the transmit rate as a percent value for all ports listed.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_tx_rate(port_info_t *info, const char *r)
{
    double rate = strtod(r, NULL);

    if (rate == 0)
        rate = 0.01;
    else if (rate > 100.00)
        rate = 100.00;
    info->tx_rate   = rate;
    info->tx_cycles = 0;

    pktgen_packet_rate(info);
}

/**
 *
 * single_set_ipaddr - Set the IP address for all ports listed
 *
 * DESCRIPTION
 * Set an IP address for all ports listed in the call.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_ipaddr(port_info_t *info, char type, struct pg_ipaddr *ip, int ip_ver)
{
    pkt_seq_t *pkt = &info->seq_pkt[SINGLE_PKT];

    if (ip_ver == 4) {
        if (type == 's') {
            pkt->ip_mask                      = size_to_mask(ip->prefixlen);
            pkt->ip_src_addr.addr.ipv4.s_addr = ntohl(ip->ipv4.s_addr);
        } else if (type == 'd')
            pkt->ip_dst_addr.addr.ipv4.s_addr = ntohl(ip->ipv4.s_addr);
        else
            return;

        if (pkt->ethType != RTE_ETHER_TYPE_IPV4 && pkt->ethType != RTE_ETHER_TYPE_ARP)
            single_set_pkt_type(info, "ipv4");

    } else if (ip_ver == 6) {
        if (type == 's') {
            pkt->ip_src_addr.prefixlen = ip->prefixlen;
            rte_memcpy(pkt->ip_src_addr.addr.ipv6.s6_addr, ip->ipv6.s6_addr,
                       sizeof(struct in6_addr));
        } else if (type == 'd')
            rte_memcpy(pkt->ip_dst_addr.addr.ipv6.s6_addr, ip->ipv6.s6_addr,
                       sizeof(struct in6_addr));
        else
            return;

        if (pkt->ethType != RTE_ETHER_TYPE_IPV6)
            single_set_pkt_type(info, "ipv6");
    }
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * rate_set_ipaddr - Set the IP address for all ports listed
 *
 * DESCRIPTION
 * Set an IP address for all ports listed in the call.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_ipaddr(port_info_t *info, char type, struct pg_ipaddr *ip, int ip_ver)
{
    pkt_seq_t *pkt = &info->seq_pkt[RATE_PKT];

    if (ip_ver == 4) {
        if (type == 's') {
            pkt->ip_mask                      = size_to_mask(ip->prefixlen);
            pkt->ip_src_addr.addr.ipv4.s_addr = ntohl(ip->ipv4.s_addr);
        } else if (type == 'd')
            pkt->ip_dst_addr.addr.ipv4.s_addr = ntohl(ip->ipv4.s_addr);
        else
            return;

        if (pkt->ethType != RTE_ETHER_TYPE_IPV4 && pkt->ethType != RTE_ETHER_TYPE_ARP)
            rate_set_pkt_type(info, "ipv4");

    } else if (ip_ver == 6) {
        if (type == 's') {
            pkt->ip_src_addr.prefixlen = ip->prefixlen;
            rte_memcpy(pkt->ip_src_addr.addr.ipv6.s6_addr, ip->ipv6.s6_addr,
                       sizeof(struct in6_addr));
        } else if (type == 'd')
            rte_memcpy(pkt->ip_dst_addr.addr.ipv6.s6_addr, ip->ipv6.s6_addr,
                       sizeof(struct in6_addr));
        else
            return;

        if (pkt->ethType != RTE_ETHER_TYPE_IPV6)
            rate_set_pkt_type(info, "ipv6");
    }
    pktgen_packet_ctor(info, RATE_PKT, -1);
    pktgen_set_tx_update(info);
}

static uint8_t
tcp_flag_from_str(const char *str)
{
    if (_cp("urg"))
        return URG_FLAG;
    if (_cp("ack"))
        return ACK_FLAG;
    if (_cp("psh"))
        return PSH_FLAG;
    if (_cp("rst"))
        return RST_FLAG;
    if (_cp("syn"))
        return SYN_FLAG;
    if (_cp("fin"))
        return FIN_FLAG;
    if (_cp("all"))
        return URG_FLAG | ACK_FLAG | PSH_FLAG | RST_FLAG | SYN_FLAG | FIN_FLAG;
    return 0;
}

/**
 *
 * single_set_tcp_flag_set - Set a TCP flag
 *
 * DESCRIPTION
 * Set a TCP flag for the single packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_tcp_flag_set(port_info_t *info, const char *which)
{
    info->seq_pkt[SINGLE_PKT].tcp_flags |= tcp_flag_from_str(which);
}

/**
 *
 * single_set_tcp_flag_clr - Clear a TCP flag
 *
 * DESCRIPTION
 * Clear a TCP flag for the single packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_tcp_flag_clr(port_info_t *info, const char *which)
{
    info->seq_pkt[SINGLE_PKT].tcp_flags &= ~tcp_flag_from_str(which);
}

/**
 *
 * range_set_tcp_flag_set - Set a TCP flag
 *
 * DESCRIPTION
 * Set a TCP flag for the range packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_tcp_flag_set(port_info_t *info, const char *which)
{
    info->range.tcp_flags |= tcp_flag_from_str(which);
    info->seq_pkt[RANGE_PKT].tcp_flags = info->range.tcp_flags;
}

/**
 *
 * range_set_tcp_flag_clr - Clear a TCP flag
 *
 * DESCRIPTION
 * Clear a TCP flag for the range packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_tcp_flag_clr(port_info_t *info, const char *which)
{
    info->range.tcp_flags &= ~tcp_flag_from_str(which);
    info->seq_pkt[RANGE_PKT].tcp_flags = info->range.tcp_flags;
}

/**
 *
 * single_set_tcp_seq - Set TCP sequence number
 *
 * DESCRIPTION
 * Set TCP sequence number
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_tcp_seq(port_info_t *info, uint32_t seq)
{
    info->seq_pkt[SINGLE_PKT].tcp_seq = seq;
}

/**
 *
 * single_set_tcp_ack - Set TCP acknowledge number
 *
 * DESCRIPTION
 * Set TCP acknowledge number
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_tcp_ack(port_info_t *info, uint32_t ack)
{
    info->seq_pkt[SINGLE_PKT].tcp_ack = ack;
}

/**
 *
 * range_set_tcp_seq - Set TCP sequence numbers
 *
 * DESCRIPTION
 * Set TCP sequence numbers
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_tcp_seq(port_info_t *info, char *what, uint32_t seq)
{
    char *str = what;
    if (_cp("inc") || _cp("increment"))
        info->range.tcp_seq_inc = seq;
    else if (_cp("min") || _cp("minimum"))
        info->range.tcp_seq_min = seq;
    else if (_cp("max") || _cp("maximum"))
        info->range.tcp_seq_max = seq;
    else if (_cp("start"))
        info->range.tcp_seq = seq;
}

/**
 *
 * range_set_tcp_ack - Set TCP acknowledge numbers
 *
 * DESCRIPTION
 * Set TCP acknowledge numbers
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_tcp_ack(port_info_t *info, char *what, uint32_t ack)
{
    char *str = what;
    if (_cp("inc") || _cp("increment"))
        info->range.tcp_ack_inc = ack;
    else if (_cp("min") || _cp("minimum"))
        info->range.tcp_ack_min = ack;
    else if (_cp("max") || _cp("maximum"))
        info->range.tcp_ack_max = ack;
    else if (_cp("start"))
        info->range.tcp_ack = ack;
}

/**
 *
 * rate_set_tcp_flag_set - Set a TCP flag
 *
 * DESCRIPTION
 * Set a TCP flag for the rate packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_tcp_flag_set(port_info_t *info, const char *which)
{
    info->seq_pkt[RATE_PKT].tcp_flags |= tcp_flag_from_str(which);
}

/**
 *
 * rate_set_tcp_flag_clr - Clear a TCP flag
 *
 * DESCRIPTION
 * Clear a TCP flag for the rate packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_tcp_flag_clr(port_info_t *info, const char *which)
{
    info->seq_pkt[RATE_PKT].tcp_flags &= ~tcp_flag_from_str(which);
}

/**
 *
 * rate_set_tcp_seq - Set TCP sequence number
 *
 * DESCRIPTION
 * Set TCP sequence number for the rate packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_tcp_seq(port_info_t *info, uint32_t seq)
{
    info->seq_pkt[RATE_PKT].tcp_seq = seq;
}

/**
 *
 * rate_set_tcp_ack - Set TCP acknowledge number
 *
 * DESCRIPTION
 * Set TCP acknowledge number for the rate packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_tcp_ack(port_info_t *info, uint32_t ack)
{
    info->seq_pkt[RATE_PKT].tcp_ack = ack;
}

/**
 *
 * single_set_mac - Setup the MAC address
 *
 * DESCRIPTION
 * Set the MAC address for all ports given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_mac(port_info_t *info, const char *which, struct rte_ether_addr *mac)
{
    if (!strcmp(which, "dst")) {
        memcpy(&info->seq_pkt[SINGLE_PKT].eth_dst_addr, mac, 6);
        pktgen_packet_ctor(info, SINGLE_PKT, -1);
    } else if (!strcmp(which, "src")) {
        memcpy(&info->seq_pkt[SINGLE_PKT].eth_src_addr, mac, 6);
        pktgen_packet_ctor(info, SINGLE_PKT, -1);
    }
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_dst_mac - Setup the destination MAC address
 *
 * DESCRIPTION
 * Set the destination MAC address for all ports given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_dst_mac(port_info_t *info, struct rte_ether_addr *mac)
{
    memcpy(&info->seq_pkt[SINGLE_PKT].eth_dst_addr, mac, 6);
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_src_mac - Setup the source MAC address
 *
 * DESCRIPTION
 * Set the source MAC address for all ports given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_src_mac(port_info_t *info, struct rte_ether_addr *mac)
{
    memcpy(&info->seq_pkt[SINGLE_PKT].eth_src_addr, mac, 6);
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * rate_set_dst_mac - Setup the destination MAC address
 *
 * DESCRIPTION
 * Set the destination MAC address for all ports given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_dst_mac(port_info_t *info, struct rte_ether_addr *mac)
{
    memcpy(&info->seq_pkt[RATE_PKT].eth_dst_addr, mac, 6);
    pktgen_packet_ctor(info, RATE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * rate_set_src_mac - Setup the source MAC address
 *
 * DESCRIPTION
 * Set the source MAC address for all ports given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_src_mac(port_info_t *info, struct rte_ether_addr *mac)
{
    memcpy(&info->seq_pkt[RATE_PKT].eth_src_addr, mac, 6);
    pktgen_packet_ctor(info, RATE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * single_set_ttl_ttl - Setup the Time to Live
 *
 * DESCRIPTION
 * Set the TTL  for all ports given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_ttl_value(port_info_t *info, uint8_t ttl)
{
    info->seq_pkt[SINGLE_PKT].ttl = ttl;
    pktgen_packet_ctor(info, SINGLE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * rateset_ttl_ttl - Setup the Time to Live
 *
 * DESCRIPTION
 * Set the TTL  for all ports given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
rate_set_ttl_value(port_info_t *info, uint8_t ttl)
{
    info->seq_pkt[RATE_PKT].ttl = ttl;
    pktgen_packet_ctor(info, RATE_PKT, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * enable_range - Enable or disable range packet sending.
 *
 * DESCRIPTION
 * Enable or disable range packet sending.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_range(port_info_t *info, uint32_t state)
{
    if (state == ENABLE_STATE) {
        if (pktgen_tst_port_flags(info, SENDING_PACKETS)) {
            pktgen_log_warning("Cannot enable the range settings while sending packets!");
            return;
        }
        pktgen_clr_port_flags(info, EXCLUSIVE_MODES);
        pktgen_set_port_flags(info, SEND_RANGE_PKTS);
    } else
        pktgen_clr_port_flags(info, SEND_RANGE_PKTS);

    pktgen_packet_rate(info);
}

/**
 *
 * enable_latency - Enable or disable latency testing.
 *
 * DESCRIPTION
 * Enable or disable latency testing.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
enable_latency(port_info_t *info, uint32_t state)
{
    if (state == ENABLE_STATE) {
        pktgen_latency_setup(info);
        pktgen_packet_ctor(info, LATENCY_PKT, -2);
        pktgen_set_tx_update(info);

        pktgen_set_port_flags(info, ENABLE_LATENCY_PKTS);
    } else
        pktgen_clr_port_flags(info, ENABLE_LATENCY_PKTS);
}

/**
 *
 * single_set_jitter - Set the jitter threshold.
 *
 * DESCRIPTION
 * Set the jitter threshold.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
single_set_jitter(port_info_t *info, uint64_t threshold)
{
    latency_t *lat = &info->latency;
    uint64_t us_per_tick;

    lat->jitter_threshold_us     = threshold;
    lat->jitter_count            = 0;
    us_per_tick                  = pktgen_get_timer_hz() / 1000000;
    lat->jitter_threshold_cycles = lat->jitter_threshold_us * us_per_tick;
}

/**
 *
 * pattern_set_type - Set the pattern type per port.
 *
 * DESCRIPTION
 * Set the given pattern type.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pattern_set_type(port_info_t *info, char *str)
{
    if (strncmp(str, "abc", 3) == 0)
        info->fill_pattern_type = ABC_FILL_PATTERN;
    else if (strncmp(str, "none", 4) == 0)
        info->fill_pattern_type = NO_FILL_PATTERN;
    else if (strncmp(str, "user", 4) == 0)
        info->fill_pattern_type = USER_FILL_PATTERN;
    else if (strncmp(str, "zero", 4) == 0)
        info->fill_pattern_type = ZERO_FILL_PATTERN;
}

/**
 *
 * pattern_set_user_pattern - Set the user pattern string.
 *
 * DESCRIPTION
 * Set the given user pattern string.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pattern_set_user_pattern(port_info_t *info, char *str)
{
    char copy[USER_PATTERN_SIZE + 1], *cp;

    memset(copy, 0, sizeof(copy));
    strcpy(copy, str);
    cp = &copy[0];
    if ((cp[0] == '"') || (cp[0] == '\'')) {
        cp[strlen(cp) - 1] = 0;
        cp++;
    }
    memset(info->user_pattern, 0, USER_PATTERN_SIZE);
    snprintf(info->user_pattern, USER_PATTERN_SIZE, "%s", cp);
    info->fill_pattern_type = USER_FILL_PATTERN;
}

/**
 *
 * range_set_dest_mac - Set the destination MAC address
 *
 * DESCRIPTION
 * Set the destination MAC address for all ports given.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_dest_mac(port_info_t *info, const char *what, struct rte_ether_addr *mac)
{
    if (!strcmp(what, "min") || !strcmp(what, "minimum"))
        inet_mtoh64(mac, &info->range.dst_mac_min);
    else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
        inet_mtoh64(mac, &info->range.dst_mac_max);
    else if (!strcmp(what, "inc") || !strcmp(what, "increment"))
        inet_mtoh64(mac, &info->range.dst_mac_inc);
    else if (!strcmp(what, "start")) {
        inet_mtoh64(mac, &info->range.dst_mac);
        /* Changes add below to reflect MAC value in range */
        memcpy(&info->seq_pkt[RANGE_PKT].eth_dst_addr, mac, 6);
    }
}

/**
 *
 * range_set_src_mac - Set the source MAC address for the ports.
 *
 * DESCRIPTION
 * Set the source MAC address for the ports given in the list.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_src_mac(port_info_t *info, const char *what, struct rte_ether_addr *mac)
{
    if (!strcmp(what, "min") || !strcmp(what, "minimum"))
        inet_mtoh64(mac, &info->range.src_mac_min);
    else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
        inet_mtoh64(mac, &info->range.src_mac_max);
    else if (!strcmp(what, "inc") || !strcmp(what, "increment"))
        inet_mtoh64(mac, &info->range.src_mac_inc);
    else if (!strcmp(what, "start")) {
        inet_mtoh64(mac, &info->range.src_mac);
        /* Changes add below to reflect MAC value in range */
        memcpy(&info->seq_pkt[RANGE_PKT].eth_src_addr, mac, 6);
    }
}

/**
 *
 * range_set_src_ip - Set the source IP address value.
 *
 * DESCRIPTION
 * Set the source IP address for all of the ports listed.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_src_ip(port_info_t *info, char *what, struct pg_ipaddr *ip)
{
    if (info->seq_pkt[RANGE_PKT].ethType == RTE_ETHER_TYPE_IPV6) {
        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            rte_memcpy(info->range.src_ipv6_min, ip->ipv6.s6_addr, sizeof(struct in6_addr));
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            rte_memcpy(info->range.src_ipv6_max, ip->ipv6.s6_addr, sizeof(struct in6_addr));
        else if (!strcmp(what, "inc") || !strcmp(what, "increment"))
            rte_memcpy(info->range.src_ipv6_inc, ip->ipv6.s6_addr, sizeof(struct in6_addr));
        else if (!strcmp(what, "start"))
            rte_memcpy(info->range.src_ipv6, ip->ipv6.s6_addr, sizeof(struct in6_addr));
    } else {
        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            info->range.src_ip_min = ntohl(ip->ipv4.s_addr);
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            info->range.src_ip_max = ntohl(ip->ipv4.s_addr);
        else if (!strcmp(what, "inc") || !strcmp(what, "increment"))
            info->range.src_ip_inc = ntohl(ip->ipv4.s_addr);
        else if (!strcmp(what, "start"))
            info->range.src_ip = ntohl(ip->ipv4.s_addr);
    }
}

/**
 *
 * range_set_dst_ip - Set the destination IP address values
 *
 * DESCRIPTION
 * Set the destination IP address values.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_dst_ip(port_info_t *info, char *what, struct pg_ipaddr *ip)
{
    if (info->seq_pkt[RANGE_PKT].ethType == RTE_ETHER_TYPE_IPV6) {
        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            rte_memcpy(info->range.dst_ipv6_min, ip->ipv6.s6_addr, sizeof(struct in6_addr));
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            rte_memcpy(info->range.dst_ipv6_max, ip->ipv6.s6_addr, sizeof(struct in6_addr));
        else if (!strcmp(what, "inc") || !strcmp(what, "increment"))
            rte_memcpy(info->range.dst_ipv6_inc, ip->ipv6.s6_addr, sizeof(struct in6_addr));
        else if (!strcmp(what, "start"))
            rte_memcpy(info->range.dst_ipv6, ip->ipv6.s6_addr, sizeof(struct in6_addr));
    } else {
        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            info->range.dst_ip_min = ntohl(ip->ipv4.s_addr);
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            info->range.dst_ip_max = ntohl(ip->ipv4.s_addr);
        else if (!strcmp(what, "inc") || !strcmp(what, "increment"))
            info->range.dst_ip_inc = ntohl(ip->ipv4.s_addr);
        else if (!strcmp(what, "start"))
            info->range.dst_ip = ntohl(ip->ipv4.s_addr);
    }
}

/**
 *
 * range_set_src_port - Set the source IP port number for the ports
 *
 * DESCRIPTION
 * Set the source IP port number for the ports listed.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_src_port(port_info_t *info, char *what, uint16_t port)
{
    if (!strcmp(what, "inc") || !strcmp(what, "increment")) {
        if (port > 64)
            port = 64;
        info->range.src_port_inc = port;
    } else {
        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            info->range.src_port_min = port;
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            info->range.src_port_max = port;
        else if (!strcmp(what, "start"))
            info->range.src_port = port;
    }
}

/**
 *
 * range_set_gtpu_teid - Set the TEID for GTPU header
 *
 * DESCRIPTION
 * Set the GTP-U TEID for the ports listed.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_gtpu_teid(port_info_t *info, char *what, uint32_t teid)
{
    if (!strcmp(what, "inc") || !strcmp(what, "increment")) {
        if (teid != 0)
            info->range.gtpu_teid_inc = teid;
    } else {
        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            info->range.gtpu_teid_min = teid;
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            info->range.gtpu_teid_max = teid;
        else if (!strcmp(what, "start")) {
            info->range.gtpu_teid              = teid;
            info->seq_pkt[RANGE_PKT].gtpu_teid = teid;
        }
    }
}

/**
 *
 * range_set_dst_port - Set the destination port value
 *
 * DESCRIPTION
 * Set the destination port values.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_dst_port(port_info_t *info, char *what, uint16_t port)
{
    if (!strcmp(what, "inc") || !strcmp(what, "increment")) {
        if (port > 64)
            port = 64;
        info->range.dst_port_inc = port;
    } else {
        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            info->range.dst_port_min = port;
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            info->range.dst_port_max = port;
        else if (!strcmp(what, "start"))
            info->range.dst_port = port;
    }
}

/**
 *
 * range_set_ttl - Set the ttl value
 *
 * DESCRIPTION
 * Set the Time to Live values
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_ttl(port_info_t *info, char *what, uint8_t ttl)
{
    if (!strcmp(what, "inc") || !strcmp(what, "increment"))
        info->range.ttl_inc = ttl;
    else if (!strcmp(what, "min") || !strcmp(what, "minimum"))
        info->range.ttl_min = ttl;
    else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
        info->range.ttl_max = ttl;
    else if (!strcmp(what, "start"))
        info->range.ttl = ttl;
}

/**
 *
 * range_set_hop_limits - Set the hop_limits value
 *
 * DESCRIPTION
 * Set the Hop Limits values
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_hop_limits(port_info_t *info, char *what, uint8_t hop_limits)
{
    if (!strcmp(what, "inc") || !strcmp(what, "increment"))
        info->range.hop_limits_inc = hop_limits;
    else if (!strcmp(what, "min") || !strcmp(what, "minimum"))
        info->range.hop_limits_min = hop_limits;
    else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
        info->range.hop_limits_max = hop_limits;
    else if (!strcmp(what, "start"))
        info->range.hop_limits = hop_limits;
}

/**
 *
 * range_set_vlan_id - Set the VLAN id value
 *
 * DESCRIPTION
 * Set the VLAN id values.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_vlan_id(port_info_t *info, char *what, uint16_t id)
{
    if (!strcmp(what, "inc") || !strcmp(what, "increment")) {
        if (id > 64)
            id = 64;
        info->range.vlan_id_inc = id;
    } else {
        if ((id < MIN_VLAN_ID) || (id > MAX_VLAN_ID))
            id = MIN_VLAN_ID;

        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            info->range.vlan_id_min = id;
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            info->range.vlan_id_max = id;
        else if (!strcmp(what, "start"))
            info->range.vlan_id = id;
    }
}

/**
 *
 * range_set_tos_id - Set the tos value
 *
 * DESCRIPTION
 * Set the tos values.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_tos_id(port_info_t *info, char *what, uint8_t id)
{

    if (!strcmp(what, "inc") || !strcmp(what, "increment")) {
        info->range.tos_inc = id;
    } else {
        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            info->range.tos_min = id;
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            info->range.tos_max = id;
        else if (!strcmp(what, "start"))
            info->range.tos = id;
    }
}

/**
 *
 * range_set_traffic_class - Set the traffic class value
 *
 * DESCRIPTION
 * Set the traffic class value.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_traffic_class(port_info_t *info, char *what, uint8_t traffic_class)
{

    if (!strcmp(what, "inc") || !strcmp(what, "increment")) {
        info->range.traffic_class_inc = traffic_class;
    } else {
        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            info->range.traffic_class_min = traffic_class;
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            info->range.traffic_class_max = traffic_class;
        else if (!strcmp(what, "start"))
            info->range.traffic_class = traffic_class;
    }
}

/**
 *
 * range_set_cos_id - Set the prio (cos) value
 *
 * DESCRIPTION
 * Set the prio (cos) values.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
range_set_cos_id(port_info_t *info, char *what, uint8_t id)
{
    if (!strcmp(what, "inc") || !strcmp(what, "increment")) {
        if (id > 7)
            id = 7;
        info->range.cos_inc = id;
    } else {
        if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            info->range.cos_min = id;
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            info->range.cos_max = id;
        else if (!strcmp(what, "start"))
            info->range.cos = id;
    }
}

/**
 *
 * range_set_pkt_size - Set the Packet size value
 *
 * DESCRIPTION
 * Set the packet size values.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
range_set_pkt_size(port_info_t *info, char *what, uint16_t size)
{
    if (!strcmp(what, "inc") || !strcmp(what, "increment")) {
        if (size > pktgen.eth_max_pkt)
            size = pktgen.eth_max_pkt;
        info->range.pkt_size_inc = size;
    } else {
        if (size < pktgen.eth_min_pkt)
            size = MIN_PKT_SIZE;
        else if (size > pktgen.eth_max_pkt)
            size = MAX_PKT_SIZE;
        else
            size -= RTE_ETHER_CRC_LEN;

        if (info->seq_pkt[RANGE_PKT].ethType == RTE_ETHER_TYPE_IPV6 && size < MIN_v6_PKT_SIZE)
            size = MIN_v6_PKT_SIZE;

        if (!strcmp(what, "start"))
            info->range.pkt_size = size;
        else if (!strcmp(what, "min") || !strcmp(what, "minimum"))
            info->range.pkt_size_min = size;
        else if (!strcmp(what, "max") || !strcmp(what, "maximum"))
            info->range.pkt_size_max = size;
    }
}

/**
 *
 * pktgen_send_arp_requests - Send an ARP request for a given port.
 *
 * DESCRIPTION
 * Using the port list do an ARp send for all ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_send_arp_requests(port_info_t *info, uint32_t type)
{
    if (type == GRATUITOUS_ARP)
        pktgen_set_port_flags(info, SEND_GRATUITOUS_ARP);
    else
        pktgen_set_port_flags(info, SEND_ARP_REQUEST);
}

/**
 *
 * pktgen_set_page - Set the page type to display
 *
 * DESCRIPTION
 * Set the page type to display
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_set_page(char *str)
{
    uint16_t page = 0;

    if (str == NULL)
        return;

    if ((str[0] >= '0') && (str[0] <= '9')) {
        page = atoi(str);
        if (page > pktgen.nb_ports)
            return;
    }

    /* Switch to the correct page */
    if (_cp("next")) {
        pcap_info_t *pcap = pktgen.info[pktgen.portNum].pcap;

        if (pcap) {
            if ((pcap->pkt_idx + PCAP_PAGE_SIZE) < pcap->pkt_count)
                pcap->pkt_idx += PCAP_PAGE_SIZE;
            else
                pcap->pkt_idx = 0;
        }
        pktgen.flags |= PRINT_LABELS_FLAG;
    } else if (_cp("cpu")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= CPU_PAGE_FLAG;
    } else if (_cp("pcap")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= PCAP_PAGE_FLAG;
        if (pktgen.info[pktgen.portNum].pcap)
            pktgen.info[pktgen.portNum].pcap->pkt_idx = 0;
    } else if (_cp("range")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= RANGE_PAGE_FLAG;
    } else if (_cp("config") || _cp("cfg")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= CONFIG_PAGE_FLAG;
    } else if (_cp("stats")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= STATS_PAGE_FLAG;
    } else if (_cp("xstats")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= XSTATS_PAGE_FLAG;
    } else if (_cp("sequence") || _cp("seq")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= SEQUENCE_PAGE_FLAG;
    } else if (_cp("random") || _cp("rnd")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= RND_BITFIELD_PAGE_FLAG;
    } else if (_cp("log")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= LOG_PAGE_FLAG;
    } else if (_cp("latency") || _cp("lat")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= LATENCY_PAGE_FLAG;
    } else if (_cp("rate-pacing") || _cp("rate")) {
        pktgen.flags &= ~PAGE_MASK_BITS;
        pktgen.flags |= RATE_PAGE_FLAG;
    } else {
        uint16_t start_port;
        if (_cp("main"))
            page = 0;
        start_port = (page * pktgen.nb_ports_per_page);
        if ((pktgen.starting_port != start_port) && (start_port < pktgen.nb_ports)) {
            pktgen.starting_port = start_port;
            pktgen.ending_port   = start_port + pktgen.nb_ports_per_page;
            if (pktgen.ending_port > (pktgen.starting_port + pktgen.nb_ports))
                pktgen.ending_port = (pktgen.starting_port + pktgen.nb_ports);
        }
        if (pktgen.flags & PAGE_MASK_BITS) {
            pktgen.flags &= ~PAGE_MASK_BITS;
            pktgen.flags |= PRINT_LABELS_FLAG;
        }
    }
    pktgen_clear_display();
}

/**
 *
 * pktgen_set_seq - Set a sequence packet for given port
 *
 * DESCRIPTION
 * Set the sequence packet information for all ports listed.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_set_seq(port_info_t *info, uint32_t seqnum, struct rte_ether_addr *daddr,
               struct rte_ether_addr *saddr, struct pg_ipaddr *ip_daddr, struct pg_ipaddr *ip_saddr,
               uint32_t sport, uint32_t dport, char type, char proto, uint16_t vlanid,
               uint32_t pktsize, uint32_t gtpu_teid)
{
    pkt_seq_t *pkt;

    pkt = &info->seq_pkt[seqnum];
    memcpy(&pkt->eth_dst_addr, daddr, 6);
    memcpy(&pkt->eth_src_addr, saddr, 6);
    pkt->ip_mask = size_to_mask(ip_saddr->prefixlen);
    if (type == '4') {
        pkt->ip_src_addr.addr.ipv4.s_addr = htonl(ip_saddr->ipv4.s_addr);
        pkt->ip_dst_addr.addr.ipv4.s_addr = htonl(ip_daddr->ipv4.s_addr);
    } else {
        memcpy(&pkt->ip_src_addr.addr.ipv6.s6_addr, ip_saddr->ipv6.s6_addr,
               sizeof(struct in6_addr));
        memcpy(&pkt->ip_dst_addr.addr.ipv6.s6_addr, ip_daddr->ipv6.s6_addr,
               sizeof(struct in6_addr));
    }
    pkt->dport   = dport;
    pkt->sport   = sport;
    pkt->pktSize = pktsize - RTE_ETHER_CRC_LEN;
    pkt->ipProto = (proto == 'u')   ? PG_IPPROTO_UDP
                   : (proto == 'i') ? PG_IPPROTO_ICMP
                                    : PG_IPPROTO_TCP;
    /* Force the IP protocol to IPv4 if this is a ICMP packet. */
    if (proto == 'i')
        type = '4';
    pkt->ethType   = (type == '6') ? RTE_ETHER_TYPE_IPV6 : RTE_ETHER_TYPE_IPV4;
    pkt->vlanid    = vlanid;
    pkt->gtpu_teid = gtpu_teid;
    pktgen_packet_ctor(info, seqnum, -1);
    pktgen_set_tx_update(info);
}

void
pktgen_set_cos_tos_seq(port_info_t *info, uint32_t seqnum, uint32_t cos, uint32_t tos)
{
    pkt_seq_t *pkt;

    pkt      = &info->seq_pkt[seqnum];
    pkt->cos = cos;
    pkt->tos = tos;
    pktgen_packet_ctor(info, seqnum, -1);
    pktgen_set_tx_update(info);
}

void
pktgen_set_vxlan_seq(port_info_t *info, uint32_t seqnum, uint32_t flag, uint32_t gid, uint32_t vid)
{
    pkt_seq_t *pkt;

    pkt            = &info->seq_pkt[seqnum];
    pkt->vni_flags = flag;
    pkt->group_id  = gid;
    pkt->vxlan_id  = vid;
    pktgen_packet_ctor(info, seqnum, -1);
    pktgen_set_tx_update(info);
}

/**
 *
 * pktgen_quit - Exit pktgen.
 *
 * DESCRIPTION
 * Close and exit Pktgen.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_quit(void)
{
    cli_quit();
}
