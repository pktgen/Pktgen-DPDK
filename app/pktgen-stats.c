/*-
 * Copyright(c) <2010-2024>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <stdio.h>

#include <pg_delay.h>
#include <lua_config.h>

#include "pktgen-cmds.h"
#include "pktgen-display.h"

#include "pktgen.h"

#include <rte_bus_pci.h>
#include <rte_bus.h>

/**
 *
 * pktgen_print_static_data - Display the static data on the screen.
 *
 * DESCRIPTION
 * Display a set of port static data on the screen.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
static void
pktgen_print_static_data(void)
{
    port_info_t *pinfo;
    struct rte_eth_dev_info dev = {0};
    uint32_t pid, col, row, sp, ip_row;
    pkt_seq_t *pkt;
    char buff[INET6_ADDRSTRLEN * 2];
    int display_cnt;

    display_topline("<Main Page>", pktgen.starting_port, (pktgen.ending_port - 1), pktgen.nb_ports);

    row = PORT_FLAGS_ROW;
    pktgen_display_set_color("stats.port.label");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Port:Flags");

    row = LINK_STATE_ROW;
    /* Labels for dynamic fields (update every second) */
    pktgen_display_set_color("stats.port.linklbl");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Link State");

    pktgen_display_set_color("stats.port.sizes");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkts/s Rx");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "       Tx");

    pktgen_display_set_color("stats.mac");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "MBits/s Rx/Tx");

    pktgen_display_set_color("stats.port.totlbl");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Total Rx Pkts");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      Tx Pkts");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      Rx/Tx MBs");

    pktgen_display_set_color("stats.port.totals");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkts/s Rx Max");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "       Tx Max");

    pktgen_display_set_color("stats.port.errlbl");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Errors Rx/Tx");

    row = PKT_SIZE_ROW;
    pktgen_display_set_color("stats.port.sizelbl");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Broadcast");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Multicast");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Sizes 64");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      65-127");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      128-255");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      256-511");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      512-1023");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      1024-1518");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Runts/Jumbos");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "ARP/ICMP Pkts");

    if (pktgen.flags & TX_DEBUG_FLAG) {
        pktgen_display_set_color("stats.port.errlbl");
        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Tx Overrun");
        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Cycles per Tx");

        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Missed Rx");
        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "mcasts Rx");
        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "No Mbuf Rx");
    }

    /* Labels for static fields */
    pktgen_display_set_color("stats.stat.label");
    ip_row = row;
#if 0
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "TCP Flags");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "TCP Seq/Ack");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pattern Type");
#endif
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Tx Count/% Rate");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkt Size/Rx:Tx Burst");
#if 0
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "TTL/Port Src/Dest");
#endif
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Port Src/Dest");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkt Type:VLAN ID");
#if 0
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "802.1p CoS/DSCP/IPP");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "VxLAN Flg/Grp/vid");
#endif
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "IP  Destination");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "    Source");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "MAC Destination");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "    Source");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "NUMA/Vend:ID/PCI");
    row++;

    /* Get the last location to use for the window starting row. */
    pktgen.last_row = row;
    display_dashline(pktgen.last_row);

    /* Display the colon after the row label. */
    pktgen_print_div(PORT_FLAGS_ROW, pktgen.last_row - 1, COLUMN_WIDTH_0 - 1);

    sp          = pktgen.starting_port;
    display_cnt = 0;
    for (pid = 0; pid < pktgen.nb_ports_per_page; pid++) {
        pinfo = l2p_get_port_pinfo(pid + sp);
        if (pinfo == NULL)
            continue;
        pktgen_display_set_color("stats.stat.values");

        pkt = &pinfo->seq_pkt[SINGLE_PKT];

        /* Display Port information Src/Dest IP addr, Netmask, Src/Dst MAC addr */
        col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;
        row = ip_row;

#if 0
        snprintf(buff, sizeof(buff), "%s%s%s%s%s%s", pkt->tcp_flags & URG_FLAG ? "U" : ".",
                 pkt->tcp_flags & ACK_FLAG ? "A" : ".", pkt->tcp_flags & PSH_FLAG ? "P" : ".",
                 pkt->tcp_flags & RST_FLAG ? "R" : ".", pkt->tcp_flags & SYN_FLAG ? "S" : ".",
                 pkt->tcp_flags & FIN_FLAG ? "F" : ".");
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%u/%u", pkt->tcp_seq, pkt->tcp_ack);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                    (pinfo->fill_pattern_type == ABC_FILL_PATTERN)    ? "abcd..."
                    : (pinfo->fill_pattern_type == NO_FILL_PATTERN)   ? "None"
                    : (pinfo->fill_pattern_type == ZERO_FILL_PATTERN) ? "Zero"
                                                                     : pinfo->user_pattern);
#endif
        pktgen_display_set_color("stats.rate.count");
        pktgen_transmit_count_rate(pid, buff, sizeof(buff));
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        pktgen_display_set_color("stats.stat.values");
        snprintf(buff, sizeof(buff), "%d /%3d:%3d", pkt->pkt_size + RTE_ETHER_CRC_LEN,
                 pinfo->rx_burst, pinfo->tx_burst);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
#if 0
        snprintf(buff, sizeof(buff), "%d/%5d/%5d", pkt->ttl, pkt->sport, pkt->dport);
#endif
        snprintf(buff, sizeof(buff), "%5d/%5d", pkt->sport, pkt->dport);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        snprintf(buff, sizeof(buff), "%s / %s:%04x",
                 (pkt->ethType == RTE_ETHER_TYPE_IPV4)   ? "IPv4"
                 : (pkt->ethType == RTE_ETHER_TYPE_IPV6) ? "IPv6"
                 : (pkt->ethType == RTE_ETHER_TYPE_ARP)  ? "ARP"
                                                         : "Other",
                 (pkt->ipProto == PG_IPPROTO_TCP)                               ? "TCP"
                 : (pkt->ipProto == PG_IPPROTO_ICMP)                            ? "ICMP"
                 : (rte_atomic32_read(&pinfo->port_flags) & SEND_VXLAN_PACKETS) ? "VXLAN"
                                                                                : "UDP",
                 pkt->vlanid);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

#if 0
        snprintf(buff, sizeof(buff), "%3d/%3d/%3d", pkt->cos, pkt->tos >> 2, pkt->tos >> 5);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%04x/%5d/%5d", pkt->vni_flags, pkt->group_id, pkt->vxlan_id);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
#endif
        pktgen_display_set_color("stats.ip");
        if (pkt->ethType == RTE_ETHER_TYPE_IPV6) {
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                        inet_ntop6(buff, sizeof(buff), pkt->ip_dst_addr.addr.ipv6.s6_addr,
                                   PG_PREFIXMAX | ((COLUMN_WIDTH_1 - 1) << 8)));
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                        inet_ntop6(buff, sizeof(buff), pkt->ip_src_addr.addr.ipv6.s6_addr,
                                   pkt->ip_src_addr.prefixlen | ((COLUMN_WIDTH_1 - 1) << 8)));
        } else {
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                        inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_dst_addr.addr.ipv4.s_addr),
                                   0xFFFFFFFF));
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                        inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_src_addr.addr.ipv4.s_addr),
                                   pkt->ip_mask));
        }
        pktgen_display_set_color("stats.mac");
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                    inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                    inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));

        rte_eth_dev_info_get(pid, &dev);

        const struct rte_bus *bus = NULL;
        if (dev.device)
            bus = rte_bus_find_by_device(dev.device);
        if (bus && !strcmp(rte_bus_name(bus), "pci")) {
            char name[RTE_ETH_NAME_MAX_LEN];
            char vend[8], device[8];

            vend[0] = device[0] = '\0';
            sscanf(rte_dev_bus_info(dev.device), "vendor_id=%4s, device_id=%4s", vend, device);

            rte_eth_dev_get_name_by_port(pid, name);
            snprintf(buff, sizeof(buff), "%d/%s:%s/%s", rte_dev_numa_node(dev.device), vend, device,
                     rte_dev_name(dev.device));
        } else
            snprintf(buff, sizeof(buff), "-1/0000:0000/00:00.0");
        pktgen_display_set_color("stats.bdf");
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        display_cnt++;
    }

    /* Display the string for total pkts/s rate of all ports */
    col = (COLUMN_WIDTH_1 * display_cnt) + COLUMN_WIDTH_0;
    pktgen_display_set_color("stats.total.label");
    scrn_printf(LINK_STATE_ROW, col, "%*s", COLUMN_WIDTH_3, "---Total Rate---");
    scrn_eol();
    pktgen_display_set_color(NULL);

    pktgen.flags &= ~PRINT_LABELS_FLAG;
}

/**
 *
 * pktgen_get_link_status - Get the port link status.
 *
 * DESCRIPTION
 * Try to get the link status of a port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_get_link_status(port_info_t *pinfo)
{
    struct rte_eth_link link = {0};
    uint32_t link_speed      = pinfo->link.link_speed;

    if (rte_eth_link_get_nowait(pinfo->pid, &link) == 0) {
        if (link.link_speed == RTE_ETH_SPEED_NUM_UNKNOWN) {
            /* Setup a few default values to prevent problems later. */
            link.link_speed   = RTE_ETH_SPEED_NUM_10G;
            link.link_duplex  = RTE_ETH_LINK_FULL_DUPLEX;
            link.link_autoneg = RTE_ETH_LINK_SPEED_AUTONEG;
            link.link_status  = RTE_ETH_LINK_UP;
        }
        if (link.link_status == RTE_ETH_LINK_UP && link_speed != link.link_speed) {
            pinfo->link.link_status  = link.link_status;
            pinfo->link.link_speed   = link.link_speed;
            pinfo->link.link_autoneg = link.link_autoneg;
            pinfo->link.link_duplex  = link.link_duplex;

            pktgen_packet_rate(pinfo);
        }
    }
}

/**
 *
 * pktgen_page_stats - Display the statistics on the screen for all ports.
 *
 * DESCRIPTION
 * Display the port statistics on the screen for all ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_page_stats(void)
{
    port_info_t *pinfo;
    unsigned int pid, col, row;
    struct rte_eth_stats *rate, *cumm, *prev;
    pkt_sizes_t sizes = {0};
    pkt_stats_t stats = {0};

    unsigned sp;
    char buff[32];
    int display_cnt;

    if (pktgen.flags & PRINT_LABELS_FLAG)
        pktgen_print_static_data();

    cumm = &pktgen.cumm_rate_totals;
    memset(cumm, 0, sizeof(struct rte_eth_stats));

    /* Calculate the total values */
    RTE_ETH_FOREACH_DEV(pid)
    {
        pinfo = l2p_get_port_pinfo(pid);
        if (pinfo == NULL)
            continue;

        rate = &pinfo->rate_stats;

        cumm->ipackets += rate->ipackets;
        cumm->opackets += rate->opackets;
        cumm->ibytes += rate->ibytes;
        cumm->obytes += rate->obytes;
        cumm->ierrors += rate->ierrors;
        cumm->oerrors += rate->oerrors;

        if (cumm->ipackets > pktgen.max_total_ipackets)
            pktgen.max_total_ipackets = cumm->ipackets;
        if (cumm->opackets > pktgen.max_total_opackets)
            pktgen.max_total_opackets = cumm->opackets;

        cumm->imissed += rate->imissed;
        cumm->rx_nombuf += rate->rx_nombuf;
    }

    sp          = pktgen.starting_port;
    display_cnt = 0;
    for (pid = 0; pid < pktgen.nb_ports_per_page; pid++) {
        pinfo = l2p_get_port_pinfo(pid + sp);
        if (pinfo == NULL)
            continue;

        memset(&sizes, 0, sizeof(pkt_sizes_t));
        memset(&stats, 0, sizeof(pkt_stats_t));

        /* Display the disable string when port is not enabled. */
        col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;

        /* Display the port number for the column */
        row = PORT_FLAGS_ROW;
        snprintf(buff, sizeof(buff), "%d:%s", pid + sp, pktgen_flags_string(pinfo));
        pktgen_display_set_color("stats.port.flags");
        scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, buff);

        /* Grab the link state of the port and display Duplex/Speed and UP/Down */
        row = LINK_STATE_ROW;
        pktgen_get_link_status(pinfo);

        pktgen_link_state(pid + sp, buff, sizeof(buff));
        pktgen_display_set_color("stats.port.status");
        scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, buff);

        rate = &pinfo->rate_stats;
        prev = &pinfo->prev_stats;

        /* Rx/Tx pkts/s rate */
        row = PKT_RATE_ROW;
        pktgen_display_set_color("stats.port.sizes");
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, rate->ipackets);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, rate->opackets);

        /* Rx/Tx Mbits per second */
        pktgen_display_set_color("stats.mac");
        snprintf(buff, sizeof(buff), "%'" PRIu64 "/%'" PRIu64,
                 iBitsTotal(pinfo->rate_stats) / Million, oBitsTotal(pinfo->rate_stats) / Million);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        /* Total Rx/Tx packets and bytes */
        pktgen_display_set_color("stats.port.totals");
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, pinfo->curr_stats.ipackets);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, pinfo->curr_stats.opackets);
        snprintf(buff, sizeof(buff), "%'lu/%'lu", iBitsTotal(pinfo->curr_stats) / Million,
                 oBitsTotal(pinfo->curr_stats) / Million);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        /* Max Rx/Tx packets */
        pktgen_display_set_color("stats.port.totals");
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, pinfo->max_ipackets);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, pinfo->max_opackets);

        /* Rx/Tx Errors */
        pktgen_display_set_color("stats.port.errors");
        snprintf(buff, sizeof(buff), "%'" PRIu64 "/%'" PRIu64, prev->ierrors, prev->oerrors);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        for (int qid = 0; qid < RTE_ETHDEV_QUEUE_STAT_CNTRS; qid++) {
            sizes.broadcast += pinfo->pkt_sizes.broadcast;
            sizes.multicast += pinfo->pkt_sizes.multicast;
            sizes._64 += pinfo->pkt_sizes._64;
            sizes._65_127 += pinfo->pkt_sizes._65_127;
            sizes._128_255 += pinfo->pkt_sizes._128_255;
            sizes._256_511 += pinfo->pkt_sizes._256_511;
            sizes._512_1023 += pinfo->pkt_sizes._512_1023;
            sizes._1024_1518 += pinfo->pkt_sizes._1024_1518;
            sizes.runt += pinfo->pkt_sizes.runt;
            sizes.jumbo += pinfo->pkt_sizes.jumbo;

            stats.arp_pkts += pinfo->pkt_stats.arp_pkts;
            stats.dropped_pkts += pinfo->pkt_stats.dropped_pkts;
            stats.echo_pkts += pinfo->pkt_stats.echo_pkts;
            stats.ibadcrc += pinfo->pkt_stats.ibadcrc;
            stats.ibadlen += pinfo->pkt_stats.ibadlen;
            stats.imissed += pinfo->pkt_stats.imissed;
            stats.ip_pkts += pinfo->pkt_stats.ip_pkts;
            stats.ipv6_pkts += pinfo->pkt_stats.ipv6_pkts;
            stats.rx_nombuf += pinfo->pkt_stats.rx_nombuf;
            stats.tx_failed += pinfo->pkt_stats.tx_failed;
            stats.unknown_pkts += pinfo->pkt_stats.unknown_pkts;
            stats.vlan_pkts += pinfo->pkt_stats.vlan_pkts;
        }

        /* Packets Sizes */
        row = PKT_SIZE_ROW;
        pktgen_display_set_color("stats.port.sizes");
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes.broadcast);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes.multicast);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._64);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._65_127);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._128_255);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._256_511);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._512_1023);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._1024_1518);
        snprintf(buff, sizeof(buff), "%'" PRIu64 "/%'" PRIu64, sizes.runt, sizes.jumbo);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%'" PRIu64 "/%'" PRIu64, stats.arp_pkts, stats.echo_pkts);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        if (pktgen.flags & TX_DEBUG_FLAG) {
            snprintf(buff, sizeof(buff), "%'" PRIu64, stats.tx_failed);
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
            snprintf(buff, sizeof(buff), "%'" PRIu64 "/%'" PRIu64, pinfo->tx_pps, pinfo->tx_cycles);
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

            snprintf(buff, sizeof(buff), "%'" PRIu64, stats.imissed);
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, "None");
            snprintf(buff, sizeof(buff), "%'" PRIu64, stats.rx_nombuf);
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        }
        pktgen_display_set_color(NULL);
        display_cnt++;
    }

    /* Display the total pkts/s for all ports */
    col = (COLUMN_WIDTH_1 * display_cnt) + COLUMN_WIDTH_0;
    row = PKT_RATE_ROW;
    pktgen_display_set_color("stats.port.sizes");
    scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_3, cumm->ipackets);
    scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_3, cumm->opackets);
    scrn_eol();

    pktgen_display_set_color("stats.mac");
    snprintf(buff, sizeof(buff), "%'" PRIu64 "/%'" PRIu64,
             iBitsTotal(pktgen.cumm_rate_totals) / Million,
             oBitsTotal(pktgen.cumm_rate_totals) / Million);
    scrn_printf(row++, col, "%*s", COLUMN_WIDTH_3, buff);
    scrn_eol();

    pktgen_display_set_color("stats.port.totals");
    scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_3, pktgen.max_total_ipackets);
    scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_3, pktgen.max_total_opackets);
    scrn_eol();
    pktgen_display_set_color(NULL);
}

/**
 *
 * pktgen_process_stats - Process statistics for all ports on timer1
 *
 * DESCRIPTION
 * When timer1 callback happens then process all of the port statistics.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_process_stats(void)
{
    unsigned int pid;
    struct rte_eth_stats *curr, *rate, *prev, *base, *queue;
    port_info_t *pinfo;
    static unsigned int counter = 0;

    counter++;
    if (pktgen.flags & BLINK_PORTS_FLAG) {
        RTE_ETH_FOREACH_DEV(pid)
        {
            if ((pktgen.blinklist & (1UL << pid)) == 0)
                continue;

            if (counter & 1)
                rte_eth_led_on(pid);
            else
                rte_eth_led_off(pid);
        }
    }

    RTE_ETH_FOREACH_DEV(pid)
    {
        pinfo = l2p_get_port_pinfo(pid);
        if (pinfo == NULL)
            continue;

        pktgen_get_link_status(pinfo);

        curr  = &pinfo->curr_stats;
        queue = &pinfo->queue_stats;
        rate  = &pinfo->rate_stats;
        prev  = &pinfo->prev_stats;
        base  = &pinfo->base_stats;

        memset(curr, 0, sizeof(struct rte_eth_stats));
        rte_eth_stats_get(pid, curr);

        memcpy(&curr->q_ipackets, &queue->q_ipackets,
               (sizeof(uint64_t) * RTE_ETHDEV_QUEUE_STAT_CNTRS));
        memcpy(&curr->q_opackets, &queue->q_opackets,
               (sizeof(uint64_t) * RTE_ETHDEV_QUEUE_STAT_CNTRS));
        memcpy(&curr->q_ibytes, &queue->q_ibytes, (sizeof(uint64_t) * RTE_ETHDEV_QUEUE_STAT_CNTRS));
        memcpy(&curr->q_obytes, &queue->q_obytes, (sizeof(uint64_t) * RTE_ETHDEV_QUEUE_STAT_CNTRS));
        memcpy(&curr->q_errors, &queue->q_errors, (sizeof(uint64_t) * RTE_ETHDEV_QUEUE_STAT_CNTRS));

        /* Normalize the counters */
        curr->ipackets  = curr->ipackets - base->ipackets;
        curr->opackets  = curr->opackets - base->opackets;
        curr->ibytes    = curr->ibytes - base->ibytes;
        curr->obytes    = curr->obytes - base->obytes;
        curr->ierrors   = curr->ierrors - base->ierrors;
        curr->oerrors   = curr->oerrors - base->oerrors;
        curr->imissed   = curr->imissed - base->imissed;
        curr->rx_nombuf = curr->rx_nombuf - base->rx_nombuf;

        /* Figure out the rate values */
        rate->ipackets  = (curr->ipackets - prev->ipackets);
        rate->opackets  = (curr->opackets - prev->opackets);
        rate->ibytes    = (curr->ibytes - prev->ibytes);
        rate->obytes    = (curr->obytes - prev->obytes);
        rate->ierrors   = (curr->ierrors - prev->ierrors);
        rate->oerrors   = (curr->oerrors - prev->oerrors);
        rate->imissed   = (curr->imissed - prev->imissed);
        rate->rx_nombuf = (curr->rx_nombuf - prev->rx_nombuf);

        for (int i = 0; i < RTE_ETHDEV_QUEUE_STAT_CNTRS; i++) {
            rate->q_ipackets[i] = curr->q_ipackets[i] - prev->q_ipackets[i];
            rate->q_opackets[i] = curr->q_opackets[i] - prev->q_opackets[i];
            rate->q_ibytes[i]   = curr->q_ibytes[i] - prev->q_ibytes[i];
            rate->q_obytes[i]   = curr->q_obytes[i] - prev->q_obytes[i];
            rate->q_errors[i]   = curr->q_errors[i] - prev->q_errors[i];
        }

        /* Find the new max rate values */
        if (rate->ipackets > pinfo->max_ipackets)
            pinfo->max_ipackets = rate->ipackets;
        if (rate->opackets > pinfo->max_opackets)
            pinfo->max_opackets = rate->opackets;

        /* Save the current values in previous */
        memcpy(prev, curr, sizeof(struct rte_eth_stats));
    }
}

void
pktgen_page_queue_stats(uint16_t pid)
{
    unsigned int col, row, q, hdr, width;
    struct rte_eth_stats *s, *r;
    struct rte_ether_addr ethaddr;
    char buff[128], mac_buf[32], dev_name[64];
    port_info_t *pinfo;

    pinfo = l2p_get_port_pinfo(pid);
    s     = &pinfo->curr_stats;

    display_topline("<Real Port Stats Page>", 0, 0, 0);

    pktgen_display_set_color("stats.port.status");
    scrn_puts(" Port %2u", pid);

    row = 3;
    col = 1;

    pktgen_display_set_color("stats.stat.label");
    scrn_printf(row + 0, col, "%-*s", COLUMN_WIDTH_0, "PCI Address     :");
    scrn_printf(row + 1, col, "%-*s", COLUMN_WIDTH_0, "Pkts Rx/Tx      :");
    scrn_printf(row + 2, col, "%-*s", COLUMN_WIDTH_0, "Rx Errors/Missed:");
    scrn_printf(row + 3, col, "%-*s", COLUMN_WIDTH_0, "Rate Rx/Tx      :");
    scrn_printf(row + 4, col, "%-*s", COLUMN_WIDTH_0, "MAC Address     :");
    scrn_printf(row + 5, col, "%-*s", COLUMN_WIDTH_0, "Link Status     :");

    col   = COLUMN_WIDTH_0;
    width = COLUMN_WIDTH_1 + 8;

    pktgen_display_set_color("stats.stat.values");
    rte_eth_dev_get_name_by_port(pid, dev_name);
    snprintf(buff, sizeof(buff), "%s", dev_name);
    scrn_printf(row + 0, col, "%*s", width, buff);

    snprintf(buff, sizeof(buff), "%'lu/%'lu", s->ipackets, s->opackets);
    scrn_printf(row + 1, col, "%*s", width, buff);

    snprintf(buff, sizeof(buff), "%'lu/%'lu", s->ierrors, s->imissed);
    scrn_printf(row + 2, col, "%*s", width, buff);

    r = &pinfo->rate_stats;
    snprintf(buff, sizeof(buff), "%'lu/%'lu", r->ipackets, r->opackets);
    scrn_printf(row + 3, col, "%*s", width, buff);

    rte_eth_macaddr_get(pid, &ethaddr);
    rte_ether_format_addr(mac_buf, sizeof(mac_buf), &ethaddr);
    snprintf(buff, sizeof(buff), "%s", mac_buf);
    scrn_printf(row + 4, col, "%*s", width, buff);

    pktgen_link_state(pid, buff, sizeof(buff));
    pktgen_display_set_color("stats.port.status");
    scrn_printf(row + 5, col, "%*s", width, buff);

    row += 6;

    hdr = 0;
    for (q = 0; q < RTE_ETHDEV_QUEUE_STAT_CNTRS; q++) {
        uint64_t rxpkts, txpkts, txbytes, rxbytes, errors, qcnt;

        if (!hdr) {
            hdr = 1;
            row++;
            pktgen_display_set_color("stats.port.status");
            scrn_printf(row++, 1, " Rate/s %12s %12s %12s %12s %12s %12s", "ipackets", "opackets",
                        "ibytes MB", "obytes MB", "errors", "bursts");
            pktgen_display_set_color("stats.stat.values");
        }

        rxpkts              = r->q_ipackets[q];
        txpkts              = r->q_opackets[q];
        rxbytes             = r->q_ibytes[q] / Million;
        txbytes             = r->q_obytes[q] / Million;
        errors              = r->q_errors[q];
        qcnt                = pinfo->qcnt[q] - pinfo->prev_qcnt[q];
        pinfo->prev_qcnt[q] = pinfo->qcnt[q];

        scrn_printf(row++, 1, "  Q %2d: %'12lu %'12lu %'12lu %'12lu %'12lu %'12lu", q, rxpkts,
                    txpkts, rxbytes, txbytes, errors, qcnt);
    }
    pktgen_display_set_color(NULL);
    display_dashline(++row);
    scrn_eol();
}

static struct xstats_info {
    struct rte_eth_xstat_name *names;
    struct rte_eth_xstat *xstats;
    struct rte_eth_xstat *prev;
    int cnt;
} xstats_info[RTE_MAX_ETHPORTS];

static void
_xstats_display(uint16_t port_id)
{
    struct xstats_info *xinfo;
    int idx_xstat, idx;

    if (!rte_eth_dev_is_valid_port(port_id)) {
        printf("Error: Invalid port number %i\n", port_id);
        return;
    }
    xinfo = &xstats_info[port_id];

    /* Get count */
    xinfo->cnt = rte_eth_xstats_get_names(port_id, NULL, 0);
    if (xinfo->cnt < 0) {
        printf("Error: Cannot get count of xstats\n");
        return;
    }
    if (xinfo->cnt == 0)
        return;

    if (xinfo->names == NULL) {
        /* Get id-name lookup table */
        xinfo->names = calloc(1, sizeof(struct rte_eth_xstat_name) * xinfo->cnt);
        if (xinfo->names == NULL) {
            printf("Cannot allocate memory for xstats lookup\n");
            return;
        }
        if (xinfo->cnt != rte_eth_xstats_get_names(port_id, xinfo->names, xinfo->cnt)) {
            printf("Error: Cannot get xstats lookup\n");
            return;
        }
    }

    /* Get stats themselves */
    if (xinfo->xstats == NULL) {
        xinfo->xstats = calloc(1, sizeof(struct rte_eth_xstat) * xinfo->cnt);
        if (xinfo->xstats == NULL) {
            printf("Cannot allocate memory for xstats\n");
            return;
        }
        xinfo->prev = calloc(1, sizeof(struct rte_eth_xstat) * xinfo->cnt);
        if (xinfo->prev == NULL) {
            printf("Cannot allocate memory for previous xstats\n");
            return;
        }
        if (xinfo->cnt != rte_eth_xstats_get(port_id, xinfo->prev, xinfo->cnt)) {
            printf("Error: Unable to get prev_xstats\n");
            return;
        }
    }
    if (xinfo->cnt != rte_eth_xstats_get(port_id, xinfo->xstats, xinfo->cnt)) {
        printf("Error: Unable to get xstats\n");
        return;
    }

    /* Display xstats */
    idx = 0;
    for (idx_xstat = 0; idx_xstat < xinfo->cnt; idx_xstat++) {
        uint64_t value;

        value = xinfo->xstats[idx_xstat].value - xinfo->prev[idx_xstat].value;
        if (xinfo->xstats[idx_xstat].value || value) {
            if (idx == 0) {
                pktgen_display_set_color("stats.port.data");
                printf("%3d: ", port_id);
                pktgen_display_set_color("stats.port.label");
                scrn_eol();
            } else if ((idx & 1) == 0) {
                printf("\n     ");
                scrn_eol();
            }
            idx++;

            pktgen_display_set_color("stats.port.label");
            printf("%-32s| ", xinfo->names[idx_xstat].name);
            pktgen_display_set_color("stats.port.data");
            printf("%'15" PRIu64, value);
            pktgen_display_set_color("stats.port.label");
            printf(" | ");
            scrn_eol();
        }
    }
    rte_memcpy(xinfo->prev, xinfo->xstats, sizeof(struct rte_eth_xstat) * xinfo->cnt);
    printf("\n");
    scrn_eol();
    printf("\n");
}

void
pktgen_page_xstats(uint16_t pid)
{
    display_topline("<Port XStats Page>", 0, 0, 0);

    pktgen_display_set_color("stats.port.status");
    scrn_printf(3, 1, "     %-32s| %15s | %-32s| %15s |\n", "XStat Name", "Per/Second",
                "XStat Name", "Per/Second");
    pktgen_display_set_color("top.page");
    printf("Port %3d ", pid);
    pktgen_display_set_color("stats.port.status");
    for (int k = 0; k < 100; k++)
        printf("=");
    printf("\n");
    pktgen_display_set_color("stats.stat.label");

    _xstats_display(pid);

    pktgen_display_set_color(NULL);
    scrn_eol();
}
