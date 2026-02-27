/*-
 * Copyright(c) <2010-2026>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <stdio.h>
#include <string.h>

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
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkts/s Rx:Tx");

    pktgen_display_set_color("stats.mac");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "MBits/s Rx:Tx");

    pktgen_display_set_color("stats.port.totals");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Max Pkts/s Rx:Tx");

    pktgen_display_set_color("stats.port.totlbl");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Total Rx Pkts");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      Tx Pkts");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      Rx:Tx MBs");

    pktgen_display_set_color("stats.port.errlbl");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Errors Rx/Tx/missed");

    row = PKT_SIZE_ROW;
    pktgen_display_set_color("stats.port.sizelbl");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Broadcast");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Multicast");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Rx Sizes 64");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "         65-127");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "         128-255");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "         256-511");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "         512-1023");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "         1024-1522");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Runts/Jumbos");

    if (pktgen.flags & TX_DEBUG_FLAG) {
        pktgen_display_set_color("stats.port.errlbl");
        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Tx Overrun");
        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "   Pkts per Queue");
        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "   Cycles per Queue");

        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Rx Missed");
        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "   mcasts");
        scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "   No Mbuf");
    }

    /* Labels for static fields */
    pktgen_display_set_color("stats.stat.label");
    ip_row = row;
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Rx/Tx queue count");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Tx Count/% Rate");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkt Size/Rx:Tx Burst");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Port Src/Dest");
    scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Type:VLAN ID:Flags");
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
            break;
        pktgen_display_set_color("stats.stat.values");

        pkt = &pinfo->seq_pkt[SINGLE_PKT];

        /* Display Port information Src/Dest IP addr, Netmask, Src/Dst MAC addr */
        col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;
        row = ip_row;

        pktgen_display_set_color("stats.rate.count");
        snprintf(buff, sizeof(buff), "%d/%d", l2p_get_rxcnt(pid), l2p_get_txcnt(pid));
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        pktgen_transmit_count_rate(pid, buff, sizeof(buff));
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        pktgen_display_set_color("stats.stat.values");
        snprintf(buff, sizeof(buff), "%d /%3d:%3d", pkt->pkt_size + RTE_ETHER_CRC_LEN,
                 pinfo->rx_burst, pinfo->tx_burst);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        snprintf(buff, sizeof(buff), "%5d/%5d", pkt->sport, pkt->dport);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        snprintf(buff, sizeof(buff), "%s / %s:%04x:%04x",
                 (pkt->ethType == RTE_ETHER_TYPE_IPV4)   ? "IPv4"
                 : (pkt->ethType == RTE_ETHER_TYPE_IPV6) ? "IPv6"
                 : (pkt->ethType == RTE_ETHER_TYPE_ARP)  ? "ARP"
                                                         : "Other",
                 (pkt->ipProto == PG_IPPROTO_TCP)                               ? "TCP"
                 : (pkt->ipProto == PG_IPPROTO_ICMP)                            ? "ICMP"
                 : (rte_atomic64_read(&pinfo->port_flags) & SEND_VXLAN_PACKETS) ? "VXLAN"
                                                                                : "UDP",
                 pkt->vlanid, pkt->tcp_flags);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        pktgen_display_set_color("stats.ip");
        if (pkt->ethType == RTE_ETHER_TYPE_IPV6) {
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                        inet_ntop6(buff, sizeof(buff), pkt->ip_dst_addr.addr.ipv6.a,
                                   PG_PREFIXMAX | ((COLUMN_WIDTH_1 - 1) << 8)));
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
                        inet_ntop6(buff, sizeof(buff), pkt->ip_src_addr.addr.ipv6.a,
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

        if (rte_eth_dev_info_get(pid, &dev) < 0)
            rte_exit(EXIT_FAILURE, "Cannot get device info for port %u\n", pid);

        const struct rte_bus *bus = NULL;
        if (dev.device)
            bus = rte_bus_find_by_device(dev.device);
        if (bus && !strcmp(rte_bus_name(bus), "pci")) {
            char name[RTE_ETH_NAME_MAX_LEN];
            char vend[8], device[8], pci[32];

            vend[0] = device[0] = '\0';
            sscanf(rte_dev_bus_info(dev.device), "vendor_id=%4s, device_id=%4s", vend, device);

            rte_eth_dev_get_name_by_port(pid, name);
            strcpy(pci, rte_dev_name(dev.device));
            snprintf(buff, sizeof(buff), "%d/%s:%s/%s", rte_dev_numa_node(dev.device), vend, device,
                     &pci[5]);
        } else
            snprintf(buff, sizeof(buff), "-1/0000:00:00.0");
        pktgen_display_set_color("stats.stat.values");
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
    struct rte_eth_link link = (struct rte_eth_link){0};

    if (rte_eth_link_get_nowait(pinfo->pid, &link) != 0)
        return;

    /* Provide safe defaults only when link is UP but speed unknown */
    if (link.link_status == RTE_ETH_LINK_UP && link.link_speed == RTE_ETH_SPEED_NUM_UNKNOWN) {
        link.link_speed   = RTE_ETH_SPEED_NUM_10G;
        link.link_duplex  = RTE_ETH_LINK_FULL_DUPLEX;
        link.link_autoneg = RTE_ETH_LINK_SPEED_AUTONEG;
    }

    /* Change if status toggled or (while up) speed/duplex/autoneg differ */
    if (link.link_status != pinfo->link.link_status ||
        (link.link_status == RTE_ETH_LINK_UP && (link.link_speed != pinfo->link.link_speed ||
                                                 link.link_duplex != pinfo->link.link_duplex ||
                                                 link.link_autoneg != pinfo->link.link_autoneg))) {

        pinfo->link.link_status  = link.link_status;
        pinfo->link.link_speed   = link.link_speed;
        pinfo->link.link_autoneg = link.link_autoneg;
        pinfo->link.link_duplex  = link.link_duplex;

        if (link.link_status == RTE_ETH_LINK_UP)
            pktgen_packet_rate(pinfo);
        else {
            /* Link down: zero pacing so UI reflects inactive state */
            pinfo->tx_cycles = 0;
            pinfo->tx_pps    = 0;
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
            break;

        rate = &pinfo->stats.rate;

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
            break;

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

        rate = &pinfo->stats.rate;
        prev = &pinfo->stats.prev;

        /* Rx/Tx pkts/s rate */
        row = PKT_RATE_ROW;
        pktgen_display_set_color("stats.port.sizes");
        snprintf(buff, sizeof(buff), "%'" PRIu64 ":%'" PRIu64, rate->ipackets, rate->opackets);
        scrn_printf(row++, col, "%'*s", COLUMN_WIDTH_1, buff);

        /* Rx/Tx Mbits per second */
        pktgen_display_set_color("stats.port.sizes");
        snprintf(buff, sizeof(buff), "%'" PRIu64 ":%'" PRIu64,
                 iBitsTotal(pinfo->stats.rate) / Million, oBitsTotal(pinfo->stats.rate) / Million);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        /* Max Rx/Tx packets */
        pktgen_display_set_color("stats.port.sizes");
        snprintf(buff, sizeof(buff), "%'" PRIu64 ":%'" PRIu64, pktgen.max_total_ipackets,
                 pktgen.max_total_opackets);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        /* Total Rx/Tx packets and bytes */
        pktgen_display_set_color("stats.port.sizes");
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, pinfo->stats.curr.ipackets);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, pinfo->stats.curr.opackets);
        snprintf(buff, sizeof(buff), "%'lu:%'lu", iBitsTotal(pinfo->stats.curr) / Million,
                 oBitsTotal(pinfo->stats.curr) / Million);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        /* Rx/Tx Errors */
        pktgen_display_set_color("stats.port.errors");
        snprintf(buff, sizeof(buff), "%'" PRIu64 "/%'" PRIu64 "/%'" PRIu64, prev->ierrors,
                 prev->oerrors, prev->imissed);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        /* Packets Sizes */
        row = PKT_SIZE_ROW;
        pktgen_display_set_color("stats.port.sizes");

        size_stats_t sizes = pinfo->stats.sizes;
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes.broadcast);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes.multicast);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._64);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._65_127);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._128_255);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._256_511);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._512_1023);
        scrn_printf(row++, col, "%'*llu", COLUMN_WIDTH_1, sizes._1024_1522);
        snprintf(buff, sizeof(buff), "%'" PRIu64 "/%'" PRIu64, sizes.runt, sizes.jumbo);
        scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        if (pktgen.flags & TX_DEBUG_FLAG) {
            snprintf(buff, sizeof(buff), "%'" PRIu64, pinfo->tx_pps);
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
            snprintf(buff, sizeof(buff), "%'" PRIu64, pinfo->tx_cycles);
            scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        }
        pktgen_display_set_color(NULL);
        display_cnt++;
    }

    /* Display the total pkts/s for all ports */
    col = (COLUMN_WIDTH_1 * display_cnt) + COLUMN_WIDTH_0;
    row = PKT_RATE_ROW;
    pktgen_display_set_color("stats.port.sizes");
    snprintf(buff, sizeof(buff), "%'" PRIu64 ":%'" PRIu64, cumm->ipackets, cumm->opackets);
    scrn_printf(row++, col, "%'*s", COLUMN_WIDTH_3, buff);
    scrn_eol();

    pktgen_display_set_color("stats.port.sizes");
    snprintf(buff, sizeof(buff), "%'" PRIu64 ":%'" PRIu64,
             iBitsTotal(pktgen.cumm_rate_totals) / Million,
             oBitsTotal(pktgen.cumm_rate_totals) / Million);
    scrn_printf(row++, col, "%*s", COLUMN_WIDTH_3, buff);
    scrn_eol();

    pktgen_display_set_color("stats.port.sizes");
    snprintf(buff, sizeof(buff), "%'" PRIu64 ":%'" PRIu64, pktgen.max_total_ipackets,
             pktgen.max_total_opackets);
    scrn_printf(row++, col, "%*s", COLUMN_WIDTH_3, buff);
    scrn_eol();
    pktgen_display_set_color(NULL);
}

static void
process_xstats(port_info_t *pinfo)
{
    xstats_t *xs = &pinfo->stats.xstats;
    int cnt;

    if (xs->cnt == 0) {
        /* Get count */
        cnt = rte_eth_xstats_get_names(pinfo->pid, NULL, 0);
        if (cnt < 0) {
            printf("Error: Cannot get count of xstats\n");
            return;
        }
        if (cnt == 0)
            return;

        xs->cnt    = cnt;
        xs->names  = rte_calloc(NULL, cnt, sizeof(struct rte_eth_xstat_name), 0);
        xs->xstats = rte_calloc(NULL, cnt, sizeof(struct rte_eth_xstat), 0);
        xs->prev   = rte_calloc(NULL, cnt, sizeof(struct rte_eth_xstat), 0);
        if (xs->names == NULL || xs->xstats == NULL || xs->prev == NULL) {
            printf("Cannot allocate memory for xstats\n");
            return;
        }
        if (rte_eth_xstats_get_names(pinfo->pid, xs->names, xs->cnt) < 0) {
            printf("Error: Cannot get xstats lookup\n");
            return;
        }
    }

    if (rte_eth_xstats_get(pinfo->pid, xs->xstats, xs->cnt) < 0) {
        printf("Error: Unable to get xstats\n");
        return;
    }

    for (int i = 0; i < xs->cnt; i++) {
        struct rte_eth_xstat *cur = &xs->xstats[i];
        char *name                = xs->names[i].name;
        uint64_t val              = cur->value;

        if (val == 0)
            continue;

        /* Update size stats */
        if (strncmp(name, "rx_size_64", strlen("rx_size_64")) == 0)
            pinfo->stats.sizes._64 = val;
        else if (strncmp(name, "rx_size_65_127", strlen("rx_size_65_127")) == 0)
            pinfo->stats.sizes._65_127 = val;
        else if (strncmp(name, "rx_size_128_255", strlen("rx_size_128_255")) == 0)
            pinfo->stats.sizes._128_255 = val;
        else if (strncmp(name, "rx_size_256_511", strlen("rx_size_256_511")) == 0)
            pinfo->stats.sizes._256_511 = val;
        else if (strncmp(name, "rx_size_512_1023", strlen("rx_size_512_1023")) == 0)
            pinfo->stats.sizes._512_1023 = val;
        else if (strncmp(name, "rx_size_1024_1522", strlen("rx_size_1024_1522")) == 0)
            pinfo->stats.sizes._1024_1522 = val;
        else if (strncmp(name, "rx_oversize_errors", strlen("rx_oversize_errors")) == 0)
            pinfo->stats.sizes.jumbo = val;
        else if (strncmp(name, "rx_undersized_errors", strlen("rx_undersized_errors")) == 0)
            pinfo->stats.sizes.runt = val;
        else if (strncmp(name, "rx_broadcast_packets", strlen("rx_broadcast_packets")) == 0)
            pinfo->stats.sizes.broadcast = val;
        else if (strncmp(name, "rx_multicast_packets", strlen("rx_multicast_packets")) == 0)
            pinfo->stats.sizes.multicast = val;
    }
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
    struct rte_eth_stats *curr, *rate, *prev, *base;
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
            break;

        pktgen_get_link_status(pinfo);

        curr = &pinfo->stats.curr;
        rate = &pinfo->stats.rate;
        prev = &pinfo->stats.prev;
        base = &pinfo->stats.base;

        memset(curr, 0, sizeof(struct rte_eth_stats));
        rte_eth_stats_get(pid, curr);

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

        /* Save the current values in previous */
        memcpy(prev, curr, sizeof(struct rte_eth_stats));

        process_xstats(pinfo);

        /* Snapshot per-queue counters written by worker lcores.
         * rte_smp_rmb() ensures all worker stores are visible before the copy.
         * snap_qstats is owned exclusively by this timer thread; the display
         * reads from snap_qstats so it never races with the worker hot path. */
        rte_smp_rmb();
        int nq = RTE_MAX(l2p_get_rxcnt(pid), l2p_get_txcnt(pid));
        for (int q = 0; q < nq && q < MAX_QUEUES_PER_PORT; q++)
            pinfo->stats.snap_qstats[q] = pinfo->stats.qstats[q];
    }
}

void
pktgen_page_qstats(uint16_t pid)
{
    unsigned int col, row, q, hdr, width;
    struct rte_eth_stats *s, *r;
    struct rte_ether_addr ethaddr;
    char buff[128], mac_buf[32], dev_name[64];
    uint64_t ipackets, opackets, errs;
    port_info_t *pinfo;

    pinfo = l2p_get_port_pinfo(pid);
    s     = &pinfo->stats.curr;

    display_topline("<Port Queue Stats>", 0, 0, 0);

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

    r = &pinfo->stats.rate;
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
    ipackets = opackets = errs = 0;
    hdr                        = 0;
    int nq                     = RTE_MAX(l2p_get_rxcnt(pid), l2p_get_txcnt(pid));
    for (q = 0; q < (unsigned int)nq; q++) {
        uint64_t rxpkts, txpkts, errors;
        qstats_t *qs      = &pinfo->stats.snap_qstats[q];
        qstats_t *prev_qs = &pinfo->stats.prev_qstats[q];

        if (!hdr) {
            hdr = 1;
            row++;
            pktgen_display_set_color("stats.port.status");
            scrn_printf(row++, 1, "%-8s: %14s %14s %14s", "Rate/sec", "ipackets", "opackets",
                        "errors");
            pktgen_display_set_color("stats.stat.values");
        }

        rxpkts   = qs->q_ipackets - prev_qs->q_ipackets;
        txpkts   = qs->q_opackets - prev_qs->q_opackets;
        errors   = qs->q_errors - prev_qs->q_errors;
        *prev_qs = *qs;

        scrn_printf(row++, 1, "  Q %2d  : %'14lu %'14lu %'14lu", q, rxpkts, txpkts, errors);
        ipackets += rxpkts;
        opackets += txpkts;
        errs += errors;
    }
    scrn_printf(row++, 1, " %-7s: %'14lu %'14lu %'14lu", "Totals", ipackets, opackets, errs);
    pktgen_display_set_color(NULL);
    display_dashline(row + 2);
    scrn_eol();
}

static void
_xstats_display(uint16_t port_id)
{
    port_info_t *pinfo;
    xstats_t *xs;

    if (!rte_eth_dev_is_valid_port(port_id)) {
        printf("Error: Invalid port number %i\n", port_id);
        return;
    }
    pinfo = l2p_get_port_pinfo(port_id);
    if (pinfo == NULL)
        return;

    xs = &pinfo->stats.xstats;

    /* Display xstats */
    int idx = 0;
    for (int idx_xstat = 0; idx_xstat < xs->cnt; idx_xstat++) {
        uint64_t value;

        value = xs->xstats[idx_xstat].value - xs->prev[idx_xstat].value;
        if (xs->xstats[idx_xstat].value) {
            if (idx == 0) {
                printf("     ");
                scrn_eol();
            } else if ((idx & 1) == 0) {
                printf("\n     ");
                scrn_eol();
            }
            idx++;

            pktgen_display_set_color("stats.port.status");
            printf("%-32s", xs->names[idx_xstat].name);
            pktgen_display_set_color("stats.port.label");
            printf("| ");
            pktgen_display_set_color("stats.port.rate");
            printf("%'15" PRIu64, value);
            pktgen_display_set_color("stats.port.label");
            printf(" | ");
            scrn_eol();
        }
    }
    rte_memcpy(xs->prev, xs->xstats, sizeof(struct rte_eth_xstat) * xs->cnt);
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
