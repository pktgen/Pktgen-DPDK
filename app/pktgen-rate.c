/*-
 * Copyright(c) <2016-2023>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2020 by Keith Wiles @ intel.com */

#include <stdio.h>

#include "lua_config.h"

#include "pktgen-cmds.h"
#include "pktgen-display.h"

#include "pktgen.h"

#include <rte_bus_pci.h>
#include <rte_bus.h>

static void
calculate_rate(port_info_t *info)
{
    rate_info_t *rate = &info->rate;

    rate->bytes_per_vframe = (rate->vlines * rate->pixels * rate->color_bits) / 8;
    rate->bits_per_sec     = (rate->bytes_per_vframe * rate->fps) * 8;

    rate->pkts_per_vframe    = rate->bytes_per_vframe / rate->payload;
    rate->total_pkts_per_sec = rate->pkts_per_vframe * rate->fps;
    rate->pps_rate           = 1.0 / (double)rate->total_pkts_per_sec;

    rate->cycles_per_pkt = (pktgen.hz / rate->total_pkts_per_sec);

    info->tx_cycles = info->rate.cycles_per_pkt;
}

void
pktgen_rate_init(port_info_t *info)
{
    rate_info_t *rate = &info->rate;

    rate->fps        = 60;
    rate->vlines     = 720;
    rate->pixels     = 1280;
    rate->color_bits = 20;

    rate->payload  = 800;
    rate->overhead = 62;

    calculate_rate(info);
}

void
update_rate_values(port_info_t *info)
{
    rate_info_t *rate = &info->rate;

    info->seq_pkt[RATE_PKT].ipProto = PG_IPPROTO_UDP;
    info->seq_pkt[RATE_PKT].pktSize = (rate->payload + rate->overhead) - RTE_ETHER_CRC_LEN;

    info->tx_burst = 1;

    calculate_rate(info);
}

void
rate_set_value(port_info_t *info, const char *what, uint32_t value)
{
    rate_info_t *rate = &info->rate;

    if (!strcmp(what, "fps")) {
        if (value < 1)
            value = 1;
        if (value > 120)
            value = 120;
        rate->fps = value;
    } else if (!strcmp(what, "lines")) {
        rate->vlines = value;
    } else if (!strcmp(what, "pixels")) {
        rate->pixels = value;
    } else if (!strcmp(what, "color")) {
        if (value > 32)
            value = 32;
        if (value < 1)
            value = 1;
        rate->color_bits = value;
    } else if (!strcmp(what, "payload")) {
        if ((value + rate->payload) <= (uint32_t)(pktgen.eth_max_pkt - RTE_ETHER_CRC_LEN))
            rate->payload = value;
    } else if (!strcmp(what, "overhead")) {
        if ((value + rate->overhead) <= (uint32_t)(pktgen.eth_max_pkt - RTE_ETHER_CRC_LEN))
            rate->overhead = value;
    }
}

/**
 *
 * pktgen_rate_setup - Setup the default values for a rate port.
 *
 * DESCRIPTION
 * Setup the default rate data for a given port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_rate_setup(port_info_t *info)
{
    rte_memcpy(&info->seq_pkt[RATE_PKT], &info->seq_pkt[SINGLE_PKT], sizeof(pkt_seq_t));

    pktgen_rate_init(info);
}

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
rate_print_static_data(void)
{
    port_info_t *info;
    struct rte_eth_dev_info dev = {0};
    uint32_t pid, col, row, sp, ip_row;
    pkt_seq_t *pkt;
    char buff[INET6_ADDRSTRLEN * 2];
    int display_cnt;

    pktgen_display_set_color("top.page");
    display_topline("<Rate-Pacing Page>");

    pktgen_display_set_color("top.ports");
    scrn_printf(1, 3, "Ports %d-%d of %d", pktgen.starting_port, (pktgen.ending_port - 1),
                pktgen.nb_ports);

    row = PORT_STATE_ROW;
    pktgen_display_set_color("stats.port.label");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "  Port:Flags");

    /* Labels for dynamic fields (update every second) */
    pktgen_display_set_color("stats.port.linklbl");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Link State");

    pktgen_display_set_color("stats.port.ratelbl");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkts/s Max/Rx");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "       Max/Tx");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "MBits/s Rx/Tx");

    row++;
    pktgen_display_set_color("stats.port.sizelbl");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Lat avg/max ");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Jitter Threshold");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Jitter count");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Total Rx pkts");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Jitter percent");

    row++;
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "FPS/Line/Pixel/Color");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Payload/Overhead");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Bpf/bps");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkts per Frame/Total");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Rate/Cycles per Pkt");

    /* Labels for static fields */
    pktgen_display_set_color("stats.stat.label");
    ip_row = ++row;
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Tx Count/% Rate");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "PktSize/Rx:Tx Burst");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "TTL/Port Src/Dst");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkt / Type");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "TCP Flags");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "TCP Seq/Ack");
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

        pkt = &info->seq_pkt[RATE_PKT];

        pktgen_display_set_color("stats.stat.values");
        /* Display Port information Src/Dest IP addr, Netmask, Src/Dst MAC addr */
        col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;
        row = ip_row;

        pktgen_display_set_color("stats.rate.count");
        rate_transmit_count_rate(pid, buff, sizeof(buff));
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        pktgen_display_set_color("stats.stat.values");
        snprintf(buff, sizeof(buff), "%d /%5d:%5d", pkt->pktSize + RTE_ETHER_CRC_LEN,
                 info->rx_burst, info->tx_burst);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        snprintf(buff, sizeof(buff), "%d/%5d/%5d", pkt->ttl, pkt->sport, pkt->dport);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        snprintf(buff, sizeof(buff), "%s / %s",
                 (pkt->ethType == RTE_ETHER_TYPE_IPV4)   ? "IPv4"
                 : (pkt->ethType == RTE_ETHER_TYPE_IPV6) ? "IPv6"
                 : (pkt->ethType == RTE_ETHER_TYPE_ARP)  ? "ARP"
                                                         : "Other",
                 (pkt->ipProto == PG_IPPROTO_TCP)    ? "TCP"
                 : (pkt->ipProto == PG_IPPROTO_ICMP) ? "ICMP"
                                                     : "UDP");
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%s%s%s%s%s%s", pkt->tcp_flags & URG_FLAG ? "U" : ".",
                 pkt->tcp_flags & ACK_FLAG ? "A" : ".", pkt->tcp_flags & PSH_FLAG ? "P" : ".",
                 pkt->tcp_flags & RST_FLAG ? "R" : ".", pkt->tcp_flags & SYN_FLAG ? "S" : ".",
                 pkt->tcp_flags & FIN_FLAG ? "F" : ".");
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        snprintf(buff, sizeof(buff), "%5u/%5u", pkt->tcp_seq, pkt->tcp_ack);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

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
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                    inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                    inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));
        rte_eth_dev_info_get(pid, &dev);
        pktgen_display_set_color("stats.mac");
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
    scrn_printf(LINK_STATE_ROW, col, "%*s", COLUMN_WIDTH_3, "----TotalRate----");
    scrn_eol();
    pktgen_display_set_color(NULL);

    pktgen.flags &= ~PRINT_LABELS_FLAG;
}

/**
 *
 * pktgen_page_rate - Display the rate pacing on the screen for all ports.
 *
 * DESCRIPTION
 * Display the port latency on the screen for all ports.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_page_rate(void)
{
    port_info_t *info;
    rate_info_t *rate;
    latency_t *lat;
    unsigned int pid, col, row;
    unsigned sp;
    char buff[32];
    int display_cnt;

    if (pktgen.flags & PRINT_LABELS_FLAG)
        rate_print_static_data();

    memset(&pktgen.cumm_rate_totals, 0, sizeof(eth_stats_t));

    sp          = pktgen.starting_port;
    display_cnt = 0;
    for (pid = 0; pid < pktgen.nb_ports_per_page; pid++) {
        if (get_map(pktgen.l2p, pid + sp, RTE_MAX_LCORE) == 0)
            continue;

        info = &pktgen.info[pid + sp];
        rate = &info->rate;
        lat  = &info->latency;

        /* Display the disable string when port is not enabled. */
        col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;
        row = PORT_STATE_ROW;

        /* Display the port number for the column */
        snprintf(buff, sizeof(buff), "%d:%s", pid + sp, pktgen_flags_string(info));
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

        pktgen_display_set_color("stats.port.rate");
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

        pktgen_display_set_color("stats.port.sizes");
        snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64, lat->min_cycles, lat->max_cycles);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%" PRIu64, lat->jitter_threshold_us);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%" PRIu64, lat->jitter_count);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%" PRIu64, info->prev_stats.ipackets);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        if (info->prev_stats.ipackets)
            snprintf(buff, sizeof(buff), "%" PRIu64,
                     (lat->jitter_count * 100) / info->prev_stats.ipackets);
        else
            snprintf(buff, sizeof(buff), "%" PRIu64, lat->avg_cycles);

        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        row++;
        snprintf(buff, sizeof(buff), "%d/%d/%d/%d", rate->fps, rate->vlines, rate->pixels,
                 rate->color_bits);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%d/%d", rate->payload, rate->overhead);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%d/%ld Mbps", rate->bytes_per_vframe,
                 ((uint64_t)rate->bits_per_sec / Million));
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%d/%d", rate->pkts_per_vframe, rate->total_pkts_per_sec);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%.2fus/%ld", rate->pps_rate * 1000000.0,
                 rate->cycles_per_pkt);
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
