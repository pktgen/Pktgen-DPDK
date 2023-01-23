/*-
 * Copyright(c) <2016-2021>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2016 by Keith Wiles @ intel.com */

#include <stdio.h>

#include "lua_config.h"

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
    port_info_t *info;
    struct rte_eth_dev_info dev = {0};
    uint32_t pid, col, row, sp, ip_row;
    pkt_seq_t *pkt;
    char buff[32];
    int display_cnt;

    pktgen_display_set_color("top.page");
    display_topline("<Latency Page>");

    pktgen_display_set_color("top.ports");
    scrn_printf(1, 3, "Ports %d-%d of %d", pktgen.starting_port, (pktgen.ending_port - 1),
                pktgen.nb_ports);

    row = PORT_STATE_ROW;
    pktgen_display_set_color("stats.port.label");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "  Flags:Port");

    /* Labels for dynamic fields (update every second) */
    pktgen_display_set_color("stats.dyn.label");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Link State");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkts/s Max/Rx");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "       Max/Tx");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "MBits/s Rx/Tx");

    row++;
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Latency avg/max");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Jitter Threshold");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Jitter count");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Jitter percent");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Latency rate (ms)");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Num Latency Pkts");

    /* Labels for static fields */
    pktgen_display_set_color("stats.stat.label");
    ip_row = ++row;
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pattern Type");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Tx Count/% Rate");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "PktSize/Rx:Tx Burst");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Src/Dest Port");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkt Type:VLAN ID");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Dst  IP Address");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Src  IP Address");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Dst MAC Address");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Src MAC Address");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "NUMA/Vend:ID/PCI");

    /* Get the last location to use for the window starting row. */
    pktgen.last_row = ++row;
    display_dashline(pktgen.last_row);

    /* Display the colon after the row label. */
    pktgen_print_div(PORT_STATE_ROW, pktgen.last_row - 1, COLUMN_WIDTH_0 - 1);

    pktgen_display_set_color("stats.stat.values");
    sp          = pktgen.starting_port;
    display_cnt = 0;
    for (pid = 0; pid < pktgen.nb_ports_per_page; pid++) {
        if (get_map(pktgen.l2p, pid + sp, RTE_MAX_LCORE) == 0)
            continue;

        info = &pktgen.info[pid + sp];

        pkt = &info->seq_pkt[SINGLE_PKT];

        pktgen_display_set_color("stats.stat.values");
        /* Display Port information Src/Dest IP addr, Netmask, Src/Dst MAC addr */
        col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;
        row = ip_row;

        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                    (info->fill_pattern_type == ABC_FILL_PATTERN)    ? "abcd..."
                    : (info->fill_pattern_type == NO_FILL_PATTERN)   ? "None"
                    : (info->fill_pattern_type == ZERO_FILL_PATTERN) ? "Zero"
                                                                     : info->user_pattern);
        pktgen_transmit_count_rate(pid, buff, sizeof(buff));
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%d /%3d:%3d", pkt->pktSize + RTE_ETHER_CRC_LEN, info->rx_burst, info->tx_burst);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        snprintf(buff, sizeof(buff), "%d/%5d/%5d", pkt->ttl, pkt->sport, pkt->dport);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        snprintf(buff, sizeof(buff), "%s / %s:%04x",
                 (pkt->ethType == RTE_ETHER_TYPE_IPV4)   ? "IPv4"
                 : (pkt->ethType == RTE_ETHER_TYPE_IPV6) ? "IPv6"
                 : (pkt->ethType == RTE_ETHER_TYPE_ARP)  ? "ARP"
                                                         : "Other",
                 (pkt->ipProto == PG_IPPROTO_TCP)    ? "TCP"
                 : (pkt->ipProto == PG_IPPROTO_ICMP) ? "ICMP"
                                                     : "UDP",
                 pkt->vlanid);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        scrn_printf(
            row++, col, "%*s", COLUMN_WIDTH_1,
            inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_dst_addr.addr.ipv4.s_addr), 0xFFFFFFFF));
        scrn_printf(
            row++, col, "%*s", COLUMN_WIDTH_1,
            inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_src_addr.addr.ipv4.s_addr), pkt->ip_mask));
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
            snprintf(buff, sizeof(buff), "%d/%s:%s/%s", rte_dev_numa_node(dev.device), vend, device, rte_dev_name(dev.device));
        } else
            snprintf(buff, sizeof(buff), "-1/0000:0000/00:00.0");
        pktgen_display_set_color("stats.bdf");
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        display_cnt++;
    }

    /* Display the string for total pkts/s rate of all ports */
    col = (COLUMN_WIDTH_1 * display_cnt) + COLUMN_WIDTH_0;
    pktgen_display_set_color("stats.total.label");
    scrn_printf(LINK_STATE_ROW, col, "%*s", COLUMN_WIDTH_3, "----TotalRate----");
    scrn_eol();
    pktgen_display_set_color(NULL);

    pktgen.flags &= ~PRINT_LABELS_FLAG;
}

/**
 *
 * pktgen_page_latency - Display the latency on the screen for all ports.
 *
 * DESCRIPTION
 * Display the port latency on the screen for all ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_page_latency(void)
{
    port_info_t *info;
    unsigned int pid, col, row;
    unsigned sp;
    char buff[32];
    int display_cnt;
    uint64_t avg_lat, ticks, max_lat;

    if (pktgen.flags & PRINT_LABELS_FLAG)
        pktgen_print_static_data();

    memset(&pktgen.cumm_rate_totals, 0, sizeof(eth_stats_t));

    sp          = pktgen.starting_port;
    display_cnt = 0;
    for (pid = 0; pid < pktgen.nb_ports_per_page; pid++) {
        if (get_map(pktgen.l2p, pid + sp, RTE_MAX_LCORE) == 0)
            continue;

        info = &pktgen.info[pid + sp];

        /* Display the disable string when port is not enabled. */
        col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;
        row = PORT_STATE_ROW;

        /* Display the port number for the column */
        snprintf(buff, sizeof(buff), "%s:%d", pktgen_flags_string(info), pid + sp);
        pktgen_display_set_color("stats.port.flags");
        scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, buff);
        pktgen_display_set_color(NULL);

        row = LINK_STATE_ROW;

        /* Grab the link state of the port and display Duplex/Speed and UP/Down */
        pktgen_get_link_status(info, pid, 0);

        pktgen_link_state(pid, buff, sizeof(buff));
        pktgen_display_set_color("stats.port.status");
        scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, buff);
        pktgen_display_set_color(NULL);

        pktgen_display_set_color("stats.stat.values");
        /* Rx/Tx pkts/s rate */
        row = LINK_STATE_ROW + 1;
        snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64, info->max_ipackets,
                 info->rate_stats.ipackets);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64, info->max_opackets,
                 info->rate_stats.opackets);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64, iBitsTotal(info->rate_stats) / Million,
                 oBitsTotal(info->rate_stats) / Million);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        pktgen.cumm_rate_totals.ipackets += info->rate_stats.ipackets;
        pktgen.cumm_rate_totals.opackets += info->rate_stats.opackets;
        pktgen.cumm_rate_totals.ibytes += info->rate_stats.ibytes;
        pktgen.cumm_rate_totals.obytes += info->rate_stats.obytes;
        pktgen.cumm_rate_totals.ierrors += info->rate_stats.ierrors;
        pktgen.cumm_rate_totals.oerrors += info->rate_stats.oerrors;

        if (pktgen.cumm_rate_totals.ipackets > pktgen.max_total_ipackets)
            pktgen.max_total_ipackets = pktgen.cumm_rate_totals.ipackets;
        if (pktgen.cumm_rate_totals.opackets > pktgen.max_total_opackets)
            pktgen.max_total_opackets = pktgen.cumm_rate_totals.opackets;

        pktgen.cumm_rate_totals.imissed += info->rate_stats.imissed;
        pktgen.cumm_rate_totals.rx_nombuf += info->rate_stats.rx_nombuf;

        row++;
        ticks   = rte_get_timer_hz() / 1000000;
        avg_lat = 0;
        max_lat = 0;
        if (info->latency_nb_pkts) {
            avg_lat = (info->avg_latency / info->latency_nb_pkts) / ticks;
            if (avg_lat > info->max_avg_latency)
                info->max_avg_latency = avg_lat;
            if (info->min_avg_latency == 0)
                info->min_avg_latency = avg_lat;
            else if (avg_lat < info->min_avg_latency)
                info->min_avg_latency = avg_lat;
            max_lat = info->max_latency / ticks;
            info->latency_nb_pkts = 0;
            info->avg_latency     = 0;
        }
        snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64, avg_lat, max_lat);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%" PRIu64, info->jitter_threshold);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%" PRIu64, info->jitter_count);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        avg_lat = 0;
        if (info->prev_stats.ipackets)
            snprintf(buff, sizeof(buff), "%" PRIu64,
                     (info->jitter_count * 100) / info->prev_stats.ipackets);
        else
            snprintf(buff, sizeof(buff), "%" PRIu64, avg_lat);

        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        display_cnt++;
    }

    /* Display the total pkts/s for all ports */
    col = (COLUMN_WIDTH_1 * display_cnt) + COLUMN_WIDTH_0;
    row = LINK_STATE_ROW + 1;
    snprintf(buff, sizeof(buff), "%lu/%lu", pktgen.max_total_ipackets,
             pktgen.cumm_rate_totals.ipackets);
    scrn_printf(row++, col, "%*s", COLUMN_WIDTH_3, buff);
    scrn_eol();
    snprintf(buff, sizeof(buff), "%lu/%lu", pktgen.max_total_opackets,
             pktgen.cumm_rate_totals.opackets);
    scrn_printf(row++, col, "%*s", COLUMN_WIDTH_3, buff);
    scrn_eol();
    snprintf(buff, sizeof(buff), "%lu/%lu", iBitsTotal(pktgen.cumm_rate_totals) / Million,
             oBitsTotal(pktgen.cumm_rate_totals) / Million);
    scrn_printf(row++, col, "%*s", COLUMN_WIDTH_3, buff);
    scrn_eol();
}
