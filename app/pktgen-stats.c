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

#include "pktgen-cmds.h"
#include "pktgen-display.h"

#include "pktgen.h"

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
    port_info_t * info;
    uint32_t pid, col, row, sp, ip_row;
    pkt_seq_t * pkt;
    char buff[32];
	int display_cnt;

	pktgen_display_set_color("top.page");
    display_topline("<Main Page>");

	pktgen_display_set_color("top.ports");
    wr_scrn_printf(1, 3, "Ports %d-%d of %d", pktgen.starting_port, (pktgen.ending_port - 1), pktgen.nb_ports);

    row = PORT_STATE_ROW;
	pktgen_display_set_color("stats.port.label");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "  Flags:Port");

	/* Labels for dynamic fields (update every second) */
	pktgen_display_set_color("stats.dyn.label");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Link State");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkts/s  Rx");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "        Tx");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "MBits/s Rx/Tx");

    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Broadcast");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Multicast");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "  64 Bytes");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "  65-127");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "  128-255");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "  256-511");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "  512-1023");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "  1024-1518");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Runts/Jumbos");

    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Errors Rx/Tx");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Total Rx Pkts");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      Tx Pkts");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      Rx MBs ");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "      Tx MBs ");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "ARP/ICMP Pkts");
	if ( pktgen.flags & TX_DEBUG_FLAG ) {
		wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Tx Overrun");
		wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Cycles per Tx");

		wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Missed Rx");
		wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Bad CRC Rx");
		wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Bad Len Rx");
		wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "mcasts Rx");
		wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "No Mbuf Rx");
	}

	/* Labels for static fields */
	pktgen_display_set_color("stats.stat.label");
    ip_row = ++row;
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Tx Count/% Rate");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "PktSize/Tx Burst");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Src/Dest Port");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Pkt Type:VLAN ID");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Dst  IP Address");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Src  IP Address");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Dst MAC Address");
    wr_scrn_printf(row++, 1, "%-*s", COLUMN_WIDTH_0, "Src MAC Address");

    // Get the last location to use for the window starting row.
    pktgen.last_row = ++row;
    display_dashline(pktgen.last_row);

    // Display the colon after the row label.
	pktgen_display_set_color("stats.colon");
    for(row = PORT_STATE_ROW; row < ((ip_row + IP_ADDR_ROWS) - 2); row++)
        wr_scrn_printf(row, COLUMN_WIDTH_0-1, ":");


	pktgen_display_set_color("stats.stat.values");
    sp = pktgen.starting_port;
	display_cnt = 0;
    for (pid = 0; pid < pktgen.nb_ports_per_page; pid++) {
        if ( wr_get_map(pktgen.l2p, pid+sp, RTE_MAX_LCORE) == 0 )
            continue;

        info	= &pktgen.info[pid+sp];

        pkt		= &info->seq_pkt[SINGLE_PKT];

        // Display Port information Src/Dest IP addr, Netmask, Src/Dst MAC addr
        col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;
        row = ip_row;

        pktgen_transmit_count_rate(pid, buff, sizeof(buff));
        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        snprintf(buff, sizeof(buff), "%d/%d", pkt->pktSize + FCS_SIZE, info->tx_burst);
        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        snprintf(buff, sizeof(buff), "%d/%d", pkt->sport, pkt->dport);
        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
        snprintf(buff, sizeof(buff), "%s/%s:%04x", (pkt->ethType == ETHER_TYPE_IPv4)? "IPv4" :
                                              (pkt->ethType == ETHER_TYPE_IPv6)? "IPv6" :
											  (pkt->ethType == ETHER_TYPE_ARP)? "ARP" : "Other",
                                              (pkt->ipProto == PG_IPPROTO_TCP)? "TCP" :
                                              (pkt->ipProto == PG_IPPROTO_ICMP)? "ICMP" : "UDP",
                                               pkt->vlanid);
        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_dst_addr), 0xFFFFFFFF));
        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_src_addr), pkt->ip_mask));
        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));
		display_cnt++;
    }

    // Display the string for total pkts/s rate of all ports
    col = (COLUMN_WIDTH_1 * display_cnt) + COLUMN_WIDTH_0;
	pktgen_display_set_color("stats.total.label");
    wr_scrn_printf(LINK_STATE_ROW, col, "%*s", COLUMN_WIDTH_1, "---TotalRate---"); wr_scrn_eol();
	pktgen_display_set_color(NULL);

    pktgen.flags &= ~PRINT_LABELS_FLAG;
}
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
pktgen_get_link_status(port_info_t * info, int pid, int wait) {

	int		i;

    /* get link status */
    for(i = 0; i < RTE_MAX_ETHPORTS; i++) {
		memset(&info->link, 0, sizeof(info->link));
		rte_eth_link_get_nowait(pid, &info->link);
		if ( info->link.link_status )
			return;
		if ( wait )
			rte_delay_ms(250);
	}
	// Setup a few default values to prevent problems later.
	info->link.link_speed	= 10000;
	info->link.link_duplex	= ETH_LINK_FULL_DUPLEX;
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
    port_info_t * info;
    unsigned int pid, col, row;
    unsigned sp;
    char buff[32];
	int display_cnt;

    if ( pktgen.flags & PRINT_LABELS_FLAG )
        pktgen_print_static_data();

    memset(&pktgen.cumm_rate_totals, 0, sizeof(eth_stats_t));

    sp = pktgen.starting_port;
	display_cnt = 0;
    for (pid = 0; pid < pktgen.nb_ports_per_page; pid++) {
        if ( wr_get_map(pktgen.l2p, pid+sp, RTE_MAX_LCORE) == 0 )
            continue;

        info = &pktgen.info[pid + sp];

        // Display the disable string when port is not enabled.
        col = (COLUMN_WIDTH_1 * pid) + COLUMN_WIDTH_0;
        row = PORT_STATE_ROW;

        // Display the port number for the column
        snprintf(buff, sizeof(buff), "%s:%d", pktgen_flags_string(info), pid+sp);
		pktgen_display_set_color("stats.port.flags");
        wr_scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, buff);
		pktgen_display_set_color(NULL);

        row = LINK_STATE_ROW;

        // Grab the link state of the port and display Duplex/Speed and UP/Down
        pktgen_get_link_status(info, pid, 0);

        pktgen_link_state(pid, buff, sizeof(buff));
		pktgen_display_set_color("stats.port.status");
        wr_scrn_printf(row, col, "%*s", COLUMN_WIDTH_1, buff);
		pktgen_display_set_color(NULL);

        // Rx/Tx pkts/s rate
        row = LINK_STATE_ROW + 1;
        wr_scrn_printf(row++,  col, "%*llu", COLUMN_WIDTH_1, info->rate_stats.ipackets);
        wr_scrn_printf(row++,  col, "%*llu", COLUMN_WIDTH_1, info->rate_stats.opackets);

        snprintf(buff, sizeof(buff), "%lu/%lu",
        		iBitsTotal(info->rate_stats)/Million, oBitsTotal(info->rate_stats)/Million);
        wr_scrn_printf(row++,  col, "%*s", COLUMN_WIDTH_1, buff);

        pktgen.cumm_rate_totals.ipackets += info->rate_stats.ipackets;
        pktgen.cumm_rate_totals.opackets += info->rate_stats.opackets;
        pktgen.cumm_rate_totals.ibytes += info->rate_stats.ibytes;
        pktgen.cumm_rate_totals.obytes += info->rate_stats.obytes;
        pktgen.cumm_rate_totals.ierrors += info->rate_stats.ierrors;
        pktgen.cumm_rate_totals.oerrors += info->rate_stats.oerrors;

        pktgen.cumm_rate_totals.imissed += info->rate_stats.imissed;
        pktgen.cumm_rate_totals.ibadcrc += info->rate_stats.ibadcrc;
        pktgen.cumm_rate_totals.ibadlen += info->rate_stats.ibadlen;
        pktgen.cumm_rate_totals.imcasts += info->rate_stats.imcasts;
        pktgen.cumm_rate_totals.rx_nombuf += info->rate_stats.rx_nombuf;

        // Packets Sizes
        row = PKT_SIZE_ROW;
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes.broadcast);
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes.multicast);
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._64);
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._65_127);
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._128_255);
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._256_511);
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._512_1023);
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->sizes._1024_1518);
        snprintf(buff, sizeof(buff), "%lu/%lu", info->sizes.runt, info->sizes.jumbo);
        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        // Rx/Tx Errors
        row = PKT_TOTALS_ROW;
        snprintf(buff, sizeof(buff), "%lu/%lu", info->port_stats.ierrors, info->port_stats.oerrors);
        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

        // Total Rx/Tx
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->port_stats.ipackets);
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, info->port_stats.opackets);

        // Total Rx/Tx mbits
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, iBitsTotal(info->port_stats)/Million);
        wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, oBitsTotal(info->port_stats)/Million);

        snprintf(buff, sizeof(buff), "%lu/%lu", info->stats.arp_pkts, info->stats.echo_pkts);
        wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

		if ( pktgen.flags & TX_DEBUG_FLAG ) {
			snprintf(buff, sizeof(buff), "%lu", info->stats.tx_failed);
			wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
			snprintf(buff, sizeof(buff), "%lu/%lu", info->tx_pps, info->tx_cycles);
			wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);

			snprintf(buff, sizeof(buff), "%lu", info->stats.imissed);
			wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
			snprintf(buff, sizeof(buff), "%lu", info->stats.ibadcrc);
			wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
			snprintf(buff, sizeof(buff), "%lu", info->stats.ibadlen);
			wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
			snprintf(buff, sizeof(buff), "%lu", info->stats.imcasts);
			wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
			snprintf(buff, sizeof(buff), "%lu", info->stats.rx_nombuf);
			wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff);
		}
		display_cnt++;
    }

    // Display the total pkts/s for all ports
    col = (COLUMN_WIDTH_1 * display_cnt) + COLUMN_WIDTH_0;
    row = LINK_STATE_ROW + 1;
    wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, pktgen.cumm_rate_totals.ipackets); wr_scrn_eol();
    wr_scrn_printf(row++, col, "%*llu", COLUMN_WIDTH_1, pktgen.cumm_rate_totals.opackets); wr_scrn_eol();
    snprintf(buff, sizeof(buff), "%lu/%lu",
    	    iBitsTotal(pktgen.cumm_rate_totals)/Million, oBitsTotal(pktgen.cumm_rate_totals)/Million);
    wr_scrn_printf(row++, col, "%*s", COLUMN_WIDTH_1, buff); wr_scrn_eol();
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
pktgen_process_stats(__attribute__((unused)) struct rte_timer *tim, __attribute__((unused)) void *arg)
{
    unsigned int pid;
    struct rte_eth_stats    stats;
    port_info_t * info;
    static unsigned int counter = 0;

    counter++;
    if ( pktgen.flags & BLINK_PORTS_FLAG ) {
    	for (pid = 0; pid < pktgen.nb_ports; pid++) {
    		if ( (pktgen.blinklist & (1ULL << pid)) == 0 )
    			continue;

			if ( counter & 1 )
				rte_eth_led_on(pid);
			else
				rte_eth_led_off(pid);
    	}
    }

    for (pid = 0; pid < pktgen.nb_ports; pid++) {
        if ( wr_get_map(pktgen.l2p, pid, RTE_MAX_LCORE) == 0 )
            continue;

        info = &pktgen.info[pid];

        rte_eth_stats_get(pid, &stats);

        // Normalize counts to the initial state, used for clearing statistics
        stats.ipackets  -= info->init_stats.ipackets;
        stats.opackets  -= info->init_stats.opackets;
        stats.ibytes    -= info->init_stats.ibytes;
        stats.obytes    -= info->init_stats.obytes;
        stats.ierrors   -= info->init_stats.ierrors;
        stats.oerrors   -= info->init_stats.oerrors;

        stats.imissed += info->init_stats.imissed;
        stats.ibadcrc += info->init_stats.ibadcrc;
        stats.ibadlen += info->init_stats.ibadlen;
        stats.imcasts += info->init_stats.imcasts;
        stats.rx_nombuf += info->init_stats.rx_nombuf;

        info->rate_stats.ipackets   = stats.ipackets - info->port_stats.ipackets;
        info->rate_stats.opackets   = stats.opackets - info->port_stats.opackets;
        info->rate_stats.ibytes     = stats.ibytes - info->port_stats.ibytes;
        info->rate_stats.obytes     = stats.obytes - info->port_stats.obytes;
        info->rate_stats.ierrors    = stats.ierrors - info->port_stats.ierrors;
        info->rate_stats.oerrors    = stats.oerrors - info->port_stats.oerrors;

        info->rate_stats.imissed += stats.imissed - info->init_stats.imissed;
        info->rate_stats.ibadcrc += stats.ibadcrc - info->init_stats.ibadcrc;
        info->rate_stats.ibadlen += stats.ibadlen - info->init_stats.ibadlen;
        info->rate_stats.imcasts += stats.imcasts - info->init_stats.imcasts;
        info->rate_stats.rx_nombuf += stats.rx_nombuf - info->init_stats.rx_nombuf;

        // Use structure move to copy the data.
        *(struct rte_eth_stats *)&info->port_stats = *(struct rte_eth_stats *)&stats;
    }
}
