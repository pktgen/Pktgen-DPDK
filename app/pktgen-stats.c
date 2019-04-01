/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <stdio.h>

#include <rte_lua.h>

#include "pktgen-cmds.h"
#include "pktgen-display.h"

#include "pktgen.h"

#if RTE_VERSION >= RTE_VERSION_NUM(17, 11, 0, 0)
#include <rte_bus_pci.h>
#endif

/**************************************************************************//**
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
	struct rte_eth_dev_info dev = { 0 };
	uint32_t pid, col, row, sp, ip_row;
	pkt_seq_t *pkt;
	char buff[32];
	int display_cnt;

	pktgen_display_set_color("top.page");
	display_topline("<Main Page>");

	pktgen_display_set_color("top.ports");
	scrn_printf(1, 3, "Ports %d-%d of %d", pktgen.starting_port,
	            (pktgen.ending_port - 1), pktgen.nb_ports);

	row = PORT_STATE_ROW;
	pktgen_display_set_color("stats.port.label");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "  Flags:Port");

	/* Labels for dynamic fields (update every second) */
	pktgen_display_set_color("stats.dyn.label");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Link State");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkts/s Max/Rx");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "       Max/Tx");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "MBits/s Rx/Tx");

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

	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Errors Rx/Tx");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Total Rx Pkts");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      Tx Pkts");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      Rx MBs");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      Tx MBs");

	if (pktgen.flags & TX_DEBUG_FLAG) {
		scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Tx Overrun");
		scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Cycles per Tx");

		scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Missed Rx");
#if RTE_VERSION < RTE_VERSION_NUM(2, 2, 0, 0)
		scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Bad CRC Rx");
		scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Bad Len Rx");
#endif
		scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "mcasts Rx");
		scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "No Mbuf Rx");
	}

	/* Labels for static fields */
	pktgen_display_set_color("stats.stat.label");
	ip_row = ++row;
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pattern Type");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Tx Count/% Rate");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkt Size/Tx Burst");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Port Src/Dest");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkt Type:VLAN ID");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "802.1p CoS/DSCP/IPP");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "VxLAN Flg/Grp/vid");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "IP  Destination");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "    Source");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "MAC Destination");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "    Source");
	scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "PCI Vendor/Addr");
	row++;

	/* Get the last location to use for the window starting row. */
	pktgen.last_row = ++row;
	display_dashline(pktgen.last_row);

	/* Display the colon after the row label. */
	pktgen_display_set_color("stats.colon");
	for (row = PORT_STATE_ROW; row < (uint32_t)(pktgen.last_row - 2); row++)
		scrn_printf(row, COLUMN_WIDTH_0 - 1, ":");

	sp = pktgen.starting_port;
	display_cnt = 0;
	for (pid = 0; pid < pktgen.nb_ports_per_page; pid++) {
		if (get_map(pktgen.l2p, pid + sp, RTE_MAX_LCORE) == 0)
			continue;

		pktgen_display_set_color("stats.stat.values");
		info    = &pktgen.info[pid + sp];

		pkt     = &info->seq_pkt[SINGLE_PKT];

		/* Display Port information Src/Dest IP addr, Netmask, Src/Dst MAC addr */
		col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;
		row = ip_row;

		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
		            (info->fill_pattern_type == ABC_FILL_PATTERN) ? "abcd..." :
		            (info->fill_pattern_type == NO_FILL_PATTERN) ? "None" :
		            (info->fill_pattern_type == ZERO_FILL_PATTERN) ? "Zero" :
		            info->user_pattern);

		pktgen_display_set_color("stats.rate.count");
		pktgen_transmit_count_rate(pid, buff, sizeof(buff));
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

		pktgen_display_set_color("stats.stat.values");
		snprintf(buff, sizeof(buff), "%d /%5d", pkt->pktSize + ETHER_CRC_LEN, info->tx_burst);
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
		snprintf(buff, sizeof(buff), "%d /%5d", pkt->sport, pkt->dport);
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
		snprintf(buff, sizeof(buff), "%s / %s:%04x",
		         (pkt->ethType == ETHER_TYPE_IPv4) ? "IPv4" :
		         (pkt->ethType == ETHER_TYPE_IPv6) ? "IPv6" :
		         (pkt->ethType == ETHER_TYPE_ARP) ? "ARP" : "Other",
		         (pkt->ipProto == PG_IPPROTO_TCP) ? "TCP" :
		         (pkt->ipProto == PG_IPPROTO_ICMP) ? "ICMP" :
			 (rte_atomic32_read(&info->port_flags) & SEND_VXLAN_PACKETS) ? "VXLAN" : "UDP",
		         pkt->vlanid);
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

		snprintf(buff, sizeof(buff), "%3d/%3d/%3d",  pkt->cos, pkt->tos >> 2, pkt->tos >> 5);
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

		snprintf(buff, sizeof(buff), "%04x/%5d/%5d",  pkt->vni_flags, pkt->group_id, pkt->vxlan_id);
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

		pktgen_display_set_color("stats.ip");
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
		            inet_ntop4(buff, sizeof(buff),
		                       htonl(pkt->ip_dst_addr.addr.ipv4.s_addr), 0xFFFFFFFF));
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
		            inet_ntop4(buff, sizeof(buff),
		                       htonl(pkt->ip_src_addr.addr.ipv4.s_addr), pkt->ip_mask));
		pktgen_display_set_color("stats.mac");
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
		            inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1,
		            inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));

		rte_eth_dev_info_get(pid, &dev);
#if RTE_VERSION < RTE_VERSION_NUM(18, 4, 0, 0)
		if (dev.pci_dev)
			snprintf(buff, sizeof(buff), "%04x:%04x/%02x:%02d.%d",
			         dev.pci_dev->id.vendor_id,
			         dev.pci_dev->id.device_id,
			         dev.pci_dev->addr.bus,
			         dev.pci_dev->addr.devid,
			         dev.pci_dev->addr.function);
		else
#else
		struct rte_bus *bus;
		if (dev.device)
			bus = rte_bus_find_by_device(dev.device);
		else
			bus = NULL;
		if (bus && !strcmp(bus->name, "pci")) {
			struct rte_pci_device *pci_dev = RTE_DEV_TO_PCI(dev.device);
			snprintf(buff, sizeof(buff), "%04x:%04x/%02x:%02d.%d",
			         pci_dev->id.vendor_id,
			         pci_dev->id.device_id,
			         pci_dev->addr.bus,
			         pci_dev->addr.devid,
			         pci_dev->addr.function);
		} else
#endif
			snprintf(buff, sizeof(buff), "%04x:%04x/%02x:%02d.%d",
			         0, 0, 0, 0, 0);
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

#define LINK_RETRY  16
/**************************************************************************//**
 *
 * pktgen_get_link_status - Get the port link status.
 *
 * DESCRIPTION
 * Try to get the link status of a port. The <wait> flag if set tells the
 * routine to try and wait for the link status for 3 seconds. If the <wait> flag
 * is zero the try three times to get a link status if the link is not up.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */
void
pktgen_get_link_status(port_info_t *info, int pid, int wait)
{
	int i;

	/* get link status */
	for (i = 0; i < LINK_RETRY; i++) {
		memset(&info->link, 0, sizeof(info->link));
		rte_eth_link_get_nowait(pid, &info->link);
		if (info->link.link_status && info->link.link_speed)
			return;
		if (!wait)
			break;
		rte_delay_us_sleep(100 * 1000);
	}

	if (wait)
		printf("**** Failed to get link UP and speed, use defaults!\n");
	/* Setup a few default values to prevent problems later. */
#if RTE_VERSION >= RTE_VERSION_NUM(17,2,0,0)
	info->link.link_speed   = ETH_SPEED_NUM_10G;
#else
	info->link.link_speed   = 10000;
#endif
	info->link.link_duplex  = ETH_LINK_FULL_DUPLEX;
}

/**************************************************************************//**
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
	port_info_t *info;
	unsigned int pid, col, row;
	struct rte_eth_stats *rate, *cumm, *prev;
	unsigned sp;
	char buff[32];
	int display_cnt;

	if (pktgen.flags & PRINT_LABELS_FLAG)
		pktgen_print_static_data();

	cumm = &pktgen.cumm_rate_totals;
	memset(cumm, 0, sizeof(eth_stats_t));

	/* Calculate the total values */
	RTE_ETH_FOREACH_DEV(pid) {
		if (get_map(pktgen.l2p, pid, RTE_MAX_LCORE) == 0)
			continue;

		info = &pktgen.info[pid];

		rate = &info->rate_stats;
		prev = &info->prev_stats;

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
#if RTE_VERSION < RTE_VERSION_NUM(2, 2, 0, 0)
		cumm->ibadcrc += rate->ibadcrc;
		cumm->ibadlen += rate->ibadlen;
#endif
#if RTE_VERSION < RTE_VERSION_NUM(16, 4, 0, 0)
		cumm->imcasts += rate->imcasts;
#endif
		cumm->rx_nombuf += rate->rx_nombuf;
	}

	sp = pktgen.starting_port;
	display_cnt = 0;
	for (pid = 0; pid < pktgen.nb_ports_per_page; pid++) {
		if (get_map(pktgen.l2p, pid + sp, RTE_MAX_LCORE) == 0)
			continue;

		info = &pktgen.info[pid + sp];

		/* Display the disable string when port is not enabled. */
		col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;
		row = PORT_STATE_ROW;

		/* Display the port number for the column */
		snprintf(buff, sizeof(buff), "%s:%d", pktgen_flags_string(
		                 info), pid + sp);
		pktgen_display_set_color("stats.port.flags");
		scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, buff);
		pktgen_display_set_color(NULL);

		row = LINK_STATE_ROW;

		/* Grab the link state of the port and display Duplex/Speed and UP/Down */
		pktgen_get_link_status(info, pid + sp, 0);

		pktgen_link_state(pid + sp, buff, sizeof(buff));
		pktgen_display_set_color("stats.port.status");
		scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, buff);
		pktgen_display_set_color(NULL);

		rate = &info->rate_stats;
		prev = &info->prev_stats;

		pktgen_display_set_color("stats.port.data");
		/* Rx/Tx pkts/s rate */
		row = LINK_STATE_ROW + 1;
		snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64,
		         info->max_ipackets, rate->ipackets);
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

		snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64,
		         info->max_opackets, rate->opackets);
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

		snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64,
		         iBitsTotal(info->rate_stats) / Million,
		         oBitsTotal(info->rate_stats) / Million);
		scrn_printf(row++,  col, "%*s", COLUMN_WIDTH_1, buff);

		/* Packets Sizes */
		row = PKT_SIZE_ROW;
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes.broadcast);
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes.multicast);
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._64);
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._65_127);
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._128_255);
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._256_511);
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._512_1023);
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._1024_1518);
		snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64,
		         info->sizes.runt, info->sizes.jumbo);
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

		snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64,
		         info->stats.arp_pkts, info->stats.echo_pkts);
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

		/* Rx/Tx Errors */
		row = PKT_TOTALS_ROW;
		snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64,
		         prev->ierrors, prev->oerrors);
		scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

		/* Total Rx/Tx */
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->curr_stats.ipackets);
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->curr_stats.opackets);

		/* Total Rx/Tx mbits */
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, iBitsTotal(info->curr_stats) / Million);
		scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, oBitsTotal(info->curr_stats) / Million);

		if (pktgen.flags & TX_DEBUG_FLAG) {
			snprintf(buff, sizeof(buff), "%" PRIu64, info->stats.tx_failed);
			scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
			snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64,
			         info->tx_pps, info->tx_cycles);
			scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

			snprintf(buff, sizeof(buff), "%" PRIu64, info->stats.imissed);
			scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
#if RTE_VERSION < RTE_VERSION_NUM(2, 2, 0, 0)
			snprintf(buff, sizeof(buff), "%" PRIu64, info->stats.ibadcrc);
			scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
			snprintf(buff, sizeof(buff), "%lu", info->stats.ibadlen);
			scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
#endif
#if RTE_VERSION < RTE_VERSION_NUM(16, 4, 0, 0)
			snprintf(buff, sizeof(buff), "%lu", info->stats.imcasts);
			scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
#else
			scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, "None");
#endif
			snprintf(buff, sizeof(buff), "%" PRIu64, info->stats.rx_nombuf);
			scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
		}
		pktgen_display_set_color(NULL);
		display_cnt++;
	}

	pktgen_display_set_color("stats.total.data");

	/* Display the total pkts/s for all ports */
	col = (COLUMN_WIDTH_1 * display_cnt) + COLUMN_WIDTH_0;
	row = LINK_STATE_ROW + 1;
	snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64,
	         pktgen.max_total_ipackets, cumm->ipackets);
	scrn_printf(row++, col, "%*s", COLUMN_WIDTH_3, buff);
	scrn_eol();
	snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64,
	         pktgen.max_total_opackets, cumm->opackets);
	scrn_printf(row++, col, "%*s", COLUMN_WIDTH_3, buff);
	scrn_eol();
	snprintf(buff, sizeof(buff), "%" PRIu64 "/%" PRIu64,
	         iBitsTotal(pktgen.cumm_rate_totals) / Million,
	         oBitsTotal(pktgen.cumm_rate_totals) / Million);
	scrn_printf(row++, col, "%*s", COLUMN_WIDTH_3, buff);
	scrn_eol();
	pktgen_display_set_color(NULL);
}

/**************************************************************************//**
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
pktgen_process_stats(struct rte_timer *tim __rte_unused, void *arg __rte_unused)
{
	unsigned int pid;
	struct rte_eth_stats *curr, *rate, *prev;
	port_info_t *info;
	static unsigned int counter = 0;
#ifdef DEBUG_TIMERS
	static uint64_t tsc = 0;

	if (tsc == 0)
		tsc = rte_rdtsc();
	else {
		uint64_t curr = rte_rdtsc();

		if ((curr - tsc) != pktgen.hz)
			printf("delta %18lu hz %lu %8ld\n", curr-tsc, pktgen.hz, pktgen.hz - (curr-tsc));
		tsc = curr;
	}
#endif
	counter++;
	if (pktgen.flags & BLINK_PORTS_FLAG) {
		RTE_ETH_FOREACH_DEV(pid) {
			if ( (pktgen.blinklist & (1ULL << pid)) == 0)
				continue;

			if (counter & 1)
				rte_eth_led_on(pid);
			else
				rte_eth_led_off(pid);
		}
	}

	RTE_ETH_FOREACH_DEV(pid) {
		info = &pktgen.info[pid];

		curr = &info->curr_stats;
		rte_eth_stats_get(pid, curr);

		rate = &info->rate_stats;
		prev = &info->prev_stats;

		rate->ipackets   = curr->ipackets - prev->ipackets;
		rate->opackets   = curr->opackets - prev->opackets;
		rate->ibytes     = curr->ibytes - prev->ibytes;
		rate->obytes     = curr->obytes - prev->obytes;
		rate->ierrors    = curr->ierrors - prev->ierrors;
		rate->oerrors    = curr->oerrors - prev->oerrors;
		rate->imissed    = curr->imissed - prev->imissed;
		rate->rx_nombuf  = curr->rx_nombuf - prev->rx_nombuf;

#if RTE_VERSION < RTE_VERSION_NUM(2, 2, 0, 0)
		rate->ibadcrc    = curr->ibadcrc - prev->ibadcrc;
		rate->ibadlen    = curr->ibadlen - prev->ibadlen;
#endif
#if RTE_VERSION < RTE_VERSION_NUM(16, 4, 0, 0)
		rate->imcasts    = curr->imcasts - prev->imcasts;
#endif

		/* Find the new max rate values */
		if (rate->ipackets > info->max_ipackets)
			info->max_ipackets = rate->ipackets;
		if (rate->opackets > info->max_opackets)
			info->max_opackets = rate->opackets;

		/* Use structure move to copy the data. */
		*prev = *curr;
	}
}

/***************************************************************************/

void
pktgen_page_phys_stats(uint16_t pid)
{
	unsigned int col, row, q, hdr;
	struct rte_eth_stats stats, *s, *r;
	struct ether_addr ethaddr;
	char buff[32], mac_buf[32];

	s = &stats;
	memset(s, 0, sizeof(struct rte_eth_stats));

	pktgen_display_set_color("top.page");
	display_topline("<Real Port Stats Page>");

        row = 3;
        col = 1;
        pktgen_display_set_color("stats.port.status");
        scrn_printf(row, col, "Port %u", pid);

        col = COLUMN_WIDTH_0 - 3;
        scrn_printf(row, col, "%*s", COLUMN_WIDTH_3, "Pkts Rx/Tx");

        col = (COLUMN_WIDTH_0 + (COLUMN_WIDTH_3 * 1)) - 3;
        scrn_printf(row, col, "%*s", COLUMN_WIDTH_3, "Rx Errors/Missed");

        col = (COLUMN_WIDTH_0 + (COLUMN_WIDTH_3 * 2)) - 3;
        scrn_printf(row, col, "%*s", COLUMN_WIDTH_3, "Rate Rx/Tx");

        col = (COLUMN_WIDTH_0 + (COLUMN_WIDTH_3 * 3)) - 3;
        scrn_printf(row, col, "%*s", COLUMN_WIDTH_3, "MAC Address");

	rte_eth_stats_get(pid, &stats);

	row = 4;
	col = 1;
	pktgen_display_set_color("stats.stat.label");
	snprintf(buff, sizeof(buff), "%2d-%s", pid, rte_eth_devices[pid].data->name);
	scrn_printf(row, col, "%-*s:", COLUMN_WIDTH_0 - 4, buff);

	pktgen_display_set_color("stats.stat.values");
	col = COLUMN_WIDTH_0 - 3;
	snprintf(buff, sizeof(buff), "%lu/%lu", s->ipackets, s->opackets);
	scrn_printf(row, col, "%*s", COLUMN_WIDTH_3, buff);

	col = (COLUMN_WIDTH_0 + (COLUMN_WIDTH_3 * 1)) - 3;
	snprintf(buff, sizeof(buff), "%lu/%lu", s->ierrors, s->imissed);
	scrn_printf(row, col, "%*s", COLUMN_WIDTH_3, buff);

	col = (COLUMN_WIDTH_0 + (COLUMN_WIDTH_3 * 2)) - 3;
	r = &pktgen.info[pid].rate_stats;
	snprintf(buff, sizeof(buff), "%lu/%lu", r->ipackets, r->opackets);
	scrn_printf(row, col, "%*s", COLUMN_WIDTH_3, buff);

	col = (COLUMN_WIDTH_0 + (COLUMN_WIDTH_3 * 3)) - 3;
	rte_eth_macaddr_get(pid, &ethaddr);
	ether_format_addr(mac_buf, sizeof(mac_buf), &ethaddr);
	snprintf(buff, sizeof(buff), "%s", mac_buf);
	scrn_printf(row, col, "%*s", COLUMN_WIDTH_3, buff);
	row++;

	hdr = 0;
	for(q = 0; q < RTE_ETHDEV_QUEUE_STAT_CNTRS; q++) {
		if (!hdr) {
			hdr = 1;
			row++;
        		pktgen_display_set_color("stats.port.status");
			scrn_printf(row++, 1, "           %14s %14s %14s %14s %14s",
					"ipackets", "opackets", "ibytes", "obytes", "errors");
			pktgen_display_set_color("stats.stat.values");
		}
		scrn_printf(row++, 1, "     Q %2d: %14lu %14lu %14lu %14lu %14lu", q,
				stats.q_ipackets[q],
				stats.q_opackets[q],
				stats.q_ibytes[q],
				stats.q_obytes[q],
				stats.q_errors[q]);
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
	struct xstats_info *info;
	int idx_xstat, idx;

	if (!rte_eth_dev_is_valid_port(port_id)) {
		printf("Error: Invalid port number %i\n", port_id);
		return;
	}
	info = &xstats_info[port_id];

	/* Get count */
	info->cnt = rte_eth_xstats_get_names(port_id, NULL, 0);
	if (info->cnt  < 0) {
		printf("Error: Cannot get count of xstats\n");
		return;
	}
	if (info->cnt == 0)
		return;

	if (info->names == NULL) {
		/* Get id-name lookup table */
		info->names = malloc(sizeof(struct rte_eth_xstat_name) * info->cnt);
		if (info->names == NULL) {
			printf("Cannot allocate memory for xstats lookup\n");
			return;
		}
		if (info->cnt != rte_eth_xstats_get_names(port_id, info->names, info->cnt)) {
			printf("Error: Cannot get xstats lookup\n");
			return;
		}
	}

	/* Get stats themselves */
	if (info->xstats == NULL) {
		info->xstats = malloc(sizeof(struct rte_eth_xstat) * info->cnt);
		if (info->xstats == NULL) {
			printf("Cannot allocate memory for xstats\n");
			return;
		}
		info->prev = malloc(sizeof(struct rte_eth_xstat) * info->cnt);
		if (info->prev== NULL) {
			printf("Cannot allocate memory for previous xstats\n");
			return;
		}
		if (info->cnt != rte_eth_xstats_get(port_id, info->prev, info->cnt)) {
			printf("Error: Unable to get prev_xstats\n");
			return;
		}
	}
	if (info->cnt != rte_eth_xstats_get(port_id, info->xstats, info->cnt)) {
		printf("Error: Unable to get xstats\n");
		return;
	}

	/* Display xstats */
	idx = 0;
	for (idx_xstat = 0; idx_xstat < info->cnt; idx_xstat++) {
		uint64_t value;

		value = info->xstats[idx_xstat].value - info->prev[idx_xstat].value;
		if (info->xstats[idx_xstat].value || value) {
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
			printf("%-32s| ", info->names[idx_xstat].name);
			pktgen_display_set_color("stats.port.data");
			printf("%12"PRIu64, value);
			pktgen_display_set_color("stats.port.label");
			printf(" | ");
			scrn_eol();
		}
	}
	rte_memcpy(info->prev, info->xstats, sizeof(struct rte_eth_xstat) * info->cnt);
	printf("\n");
	scrn_eol();
	printf("\n");
}

void
pktgen_page_xstats(uint16_t pid)
{
	uint64_t p;
	int k;

	pktgen_display_set_color("top.page");
	display_topline("<Port XStats Page>");

	pktgen_display_set_color("stats.port.status");
	scrn_printf(3, 1, "     %-32s| %12s | %-32s| %12s |\n",
		"XStat Name", "Per/Second", "XStat Name", "Per/Second");
	pktgen_display_set_color("top.page");
	printf("Port ");
	pktgen_display_set_color("stats.port.status");
	for(k = 0; k < 97; k++)
		printf("=");
	printf("\n");
	pktgen_display_set_color("stats.stat.label");

	k = 0;
	for (p = rte_eth_find_next_owned_by(pid, RTE_ETH_DEV_NO_OWNER);
	     (unsigned int)p < (unsigned int)RTE_MAX_ETHPORTS;
	     p = rte_eth_find_next_owned_by(p + 1, RTE_ETH_DEV_NO_OWNER)) {

		_xstats_display(p);
		if (k++ >= pktgen.nb_ports_per_page)
			break;
	}

	pktgen_display_set_color(NULL);
	scrn_eol();
}
