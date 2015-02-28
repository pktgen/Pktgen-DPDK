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
pktgen_save(char * path)
{
	port_info_t	  * info;
	pkt_seq_t	  * pkt;
	range_info_t  * range;
	uint32_t		flags;
	char		buff[64];
	FILE	  * fd;
	int			i, j;
	uint64_t	lcore;
    struct ether_addr eaddr;

	fd = fopen(path, "w");
	if ( fd == NULL ) {
		return -1;
	}

	for(i=0, lcore=0; i<RTE_MAX_LCORE; i++)
		if ( rte_lcore_is_enabled(i) )
			lcore |= (1 << i);

	fprintf(fd, "#\n# Pktgen - %s\n", pktgen_version());
	fprintf(fd, "# %s, %s\n\n", wr_copyright_msg(), wr_powered_by());

	// TODO: Determine DPDK arguments for rank and memory, default for now.
	fprintf(fd, "# Command line arguments: (DPDK args are defaults)\n");
	fprintf(fd, "# %s -c %lx -n 3 -m 512 --proc-type %s -- ", pktgen.argv[0], lcore, (rte_eal_process_type() == RTE_PROC_PRIMARY)? "primary" : "secondary");
	for(i=1; i < pktgen.argc; i++)
		fprintf(fd, "%s ", pktgen.argv[i]);
	fprintf(fd, "\n\n");

	fprintf(fd, "#######################################################################\n");
	fprintf(fd, "# Pktgen Configuration script information:\n");
	fprintf(fd, "#   GUI socket is %s\n", (pktgen.flags & ENABLE_GUI_FLAG)? "Enabled" : "Not Enabled");
	fprintf(fd, "#   Flags %08x\n", pktgen.flags);
	fprintf(fd, "#   Number of ports: %d\n", pktgen.nb_ports);
	fprintf(fd, "#   Number ports per page: %d\n", pktgen.nb_ports_per_page);
	fprintf(fd, "#   Number descriptors: RX %d TX: %d\n", pktgen.nb_rxd, pktgen.nb_txd);
	fprintf(fd, "#   Promiscuous mode is %s\n\n", (pktgen.flags & PROMISCUOUS_ON_FLAG)? "Enabled" : "Disabled");

#if 0
	fprintf(fd, "# Port Descriptions (-- = blacklisted port):\n");
	for(i=0; i < RTE_MAX_ETHPORTS; i++) {
		if ( pktgen.portdesc[i] && strlen((char *)pktgen.portdesc[i]) ) {
	    	if ( (pktgen.enabled_port_mask & (1 << i)) == 0 )
	    		strcpy(buff, "--");
	    	else
	    		strcpy(buff, "++");

			fprintf(fd, "#   %s %s\n", buff, pktgen.portdesc[i]);
		}
	}
#endif
	fprintf(fd, "\n#######################################################################\n");

	fprintf(fd, "# Global configuration:\n");
	uint16_t rows, cols;
	pktgen_display_get_geometry(&rows, &cols);
	fprintf(fd, "geometry %dx%d\n", cols, rows);
	fprintf(fd, "mac_from_arp %s\n\n", (pktgen.flags & MAC_FROM_ARP_FLAG)? "enable" : "disable");

	for(i=0; i < RTE_MAX_ETHPORTS; i++) {
		info = &pktgen.info[i];
		pkt = &info->seq_pkt[SINGLE_PKT];
		range = &info->range;

		if ( info->tx_burst == 0 )
			continue;

		fprintf(fd, "######################### Port %2d ##################################\n", i);
		if ( rte_atomic64_read(&info->transmit_count) == 0 )
			strcpy(buff, "Forever");
		else
			snprintf(buff, sizeof(buff), "%ld", rte_atomic64_read(&info->transmit_count));
		fprintf(fd, "#\n");
		flags = rte_atomic32_read(&info->port_flags);
		fprintf(fd, "# Port: %2d, Burst:%3d, Rate:%3d%%, Flags:%08x, TX Count:%s\n",
				info->pid, info->tx_burst, info->tx_rate, flags, buff);
		fprintf(fd, "#           SeqCnt:%d, Prime:%d VLAN ID:%04x, ",
				info->seqCnt, info->prime_cnt, info->vlanid);
		pktgen_link_state(info->pid, buff, sizeof(buff));
		fprintf(fd, "Link: %s\n", buff);

		fprintf(fd, "#\n# Set up the primary port information:\n");
		fprintf(fd, "set %d count %ld\n", info->pid, rte_atomic64_read(&info->transmit_count));
		fprintf(fd, "set %d size %d\n", info->pid, pkt->pktSize+FCS_SIZE);
		fprintf(fd, "set %d rate %d\n", info->pid, info->tx_rate);
		fprintf(fd, "set %d burst %d\n", info->pid, info->tx_burst);
		fprintf(fd, "set %d sport %d\n", info->pid, pkt->sport);
		fprintf(fd, "set %d dport %d\n", info->pid, pkt->dport);
		fprintf(fd, "set %d prime %d\n", info->pid, info->prime_cnt);
		fprintf(fd, "type %s %d\n",
				(pkt->ethType == ETHER_TYPE_IPv4)? "ipv4" :
				(pkt->ethType == ETHER_TYPE_IPv6)? "ipv6" :
				(pkt->ethType == ETHER_TYPE_VLAN)? "vlan" :
				(pkt->ethType == ETHER_TYPE_ARP) ? "arp" :"unknown", i);
		fprintf(fd, "proto %s %d\n",
				(pkt->ipProto == PG_IPPROTO_TCP)? "tcp" :
				(pkt->ipProto == PG_IPPROTO_ICMP)? "icmp" : "udp", i);
		fprintf(fd, "set ip dst %d %s\n", i, inet_ntop4(buff, sizeof(buff), ntohl(pkt->ip_dst_addr), 0xFFFFFFFF));
		fprintf(fd, "set ip src %d %s\n", i, inet_ntop4(buff, sizeof(buff), ntohl(pkt->ip_src_addr), pkt->ip_mask));
		fprintf(fd, "set mac %d %s\n", info->pid, inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
		fprintf(fd, "vlanid %d %d\n\n", i, pkt->vlanid);

		fprintf(fd, "mpls %d %sable\n", i, (flags & SEND_MPLS_LABEL) ? "en" : "dis");
		sprintf(buff, "%x", pkt->mpls_entry);
		fprintf(fd, "mpls_entry %d %s\n", i, buff);

		fprintf(fd, "qinq %d %sable\n", i, (flags & SEND_Q_IN_Q_IDS) ? "en" : "dis");
		fprintf(fd, "qinqids %d %d %d\n", i, pkt->qinq_outerid, pkt->qinq_innerid);

		fprintf(fd, "gre %d %sable\n", i, (flags & SEND_GRE_IPv4_HEADER) ? "en" : "dis");
		fprintf(fd, "gre_eth %d %sable\n", i, (flags & SEND_GRE_ETHER_HEADER) ? "en" : "dis");
		fprintf(fd, "gre_key %d %d\n", i, pkt->gre_key);

		fprintf(fd, "#\n# Port flag values:\n");
		fprintf(fd, "icmp.echo %d %sable\n", i, (flags & ICMP_ECHO_ENABLE_FLAG)? "en" : "dis");
		fprintf(fd, "pcap %d %sable\n", i, (flags & SEND_PCAP_PKTS)? "en" : "dis");
		fprintf(fd, "range %d %sable\n", i, (flags & SEND_RANGE_PKTS)? "en" : "dis");
		fprintf(fd, "process %d %sable\n", i, (flags & PROCESS_INPUT_PKTS)? "en" : "dis");
		fprintf(fd, "capture %d %sable\n", i, (flags & CAPTURE_PKTS)? "en" : "dis");
		fprintf(fd, "rxtap %d %sable\n", i, (flags & PROCESS_RX_TAP_PKTS)? "en" : "dis");
		fprintf(fd, "txtap %d %sable\n", i, (flags & PROCESS_TX_TAP_PKTS)? "en" : "dis");
		fprintf(fd, "vlan %d %sable\n\n", i, (flags & SEND_VLAN_ID)? "en" : "dis");

		fprintf(fd, "#\n# Range packet information:\n");
		fprintf(fd, "src.mac start %d %s\n", i, inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac, &eaddr)));
		fprintf(fd, "src.mac min %d %s\n", i, inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac_min, &eaddr)));
		fprintf(fd, "src.mac max %d %s\n", i, inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac_max, &eaddr)));
        fprintf(fd, "src.mac inc %d %s\n", i, inet_mtoa(buff, sizeof(buff), inet_h64tom(range->src_mac_inc, &eaddr)));

		fprintf(fd, "dst.mac start %d %s\n", i, inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac, &eaddr)));
		fprintf(fd, "dst.mac min %d %s\n", i, inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac_min, &eaddr)));
		fprintf(fd, "dst.mac max %d %s\n", i, inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac_max, &eaddr)));
		fprintf(fd, "dst.mac inc %d %s\n", i, inet_mtoa(buff, sizeof(buff), inet_h64tom(range->dst_mac_inc, &eaddr)));

		fprintf(fd, "\n");
		fprintf(fd, "src.ip start %d %s\n", i, inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip), 0xFFFFFFFF));
		fprintf(fd, "src.ip min %d %s\n", i, inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip_min), 0xFFFFFFFF));
		fprintf(fd, "src.ip max %d %s\n", i, inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip_max), 0xFFFFFFFF));
		fprintf(fd, "src.ip inc %d %s\n", i, inet_ntop4(buff, sizeof(buff), ntohl(range->src_ip_inc), 0xFFFFFFFF));

		fprintf(fd, "\n");
		fprintf(fd, "dst.ip start %d %s\n", i, inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip), 0xFFFFFFFF));
		fprintf(fd, "dst.ip min %d %s\n", i, inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip_min), 0xFFFFFFFF));
		fprintf(fd, "dst.ip max %d %s\n", i, inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip_max), 0xFFFFFFFF));
		fprintf(fd, "dst.ip inc %d %s\n", i, inet_ntop4(buff, sizeof(buff), ntohl(range->dst_ip_inc), 0xFFFFFFFF));

		fprintf(fd, "\n");
		fprintf(fd, "src.port start %d %d\n", i, range->src_port);
		fprintf(fd, "src.port min %d %d\n", i, range->src_port_min);
		fprintf(fd, "src.port max %d %d\n", i, range->src_port_max);
		fprintf(fd, "src.port inc %d %d\n", i, range->src_port_inc);

		fprintf(fd, "\n");
		fprintf(fd, "dst.port start %d %d\n", i, range->dst_port);
		fprintf(fd, "dst.port min %d %d\n", i, range->dst_port_min);
		fprintf(fd, "dst.port max %d %d\n", i, range->dst_port_max);
		fprintf(fd, "dst.port inc %d %d\n", i, range->dst_port_inc);

		fprintf(fd, "\n");
		fprintf(fd, "vlan.id start %d %d\n", i, range->vlan_id);
		fprintf(fd, "vlan.id min %d %d\n", i, range->vlan_id_min);
		fprintf(fd, "vlan.id max %d %d\n", i, range->vlan_id_max);
		fprintf(fd, "vlan.id inc %d %d\n", i, range->vlan_id_inc);

		fprintf(fd, "\n");
		fprintf(fd, "pkt.size start %d %d\n", i, range->pkt_size + FCS_SIZE);
		fprintf(fd, "pkt.size min %d %d\n", i, range->pkt_size_min + FCS_SIZE);
		fprintf(fd, "pkt.size max %d %d\n", i, range->pkt_size_max + FCS_SIZE);
		fprintf(fd, "pkt.size inc %d %d\n\n", i, range->pkt_size_inc);

		fprintf(fd, "#\n# Set up the sequence data for the port.\n");
		fprintf(fd, "set %d seqCnt %d\n", info->pid, info->seqCnt);
		for(j=0; j<info->seqCnt; j++) {
			pkt = &info->seq_pkt[j];
			fprintf(fd, "seq %d %d %s ", j, i, inet_mtoa(buff, sizeof(buff), &pkt->eth_dst_addr));
			fprintf(fd, "%s ", inet_mtoa(buff, sizeof(buff), &pkt->eth_src_addr));
			fprintf(fd, "%s ", inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_dst_addr), 0xFFFFFFFF));
			fprintf(fd, "%s ", inet_ntop4(buff, sizeof(buff), htonl(pkt->ip_src_addr), pkt->ip_mask));
			fprintf(fd, "%d %d %s %s %d %d\n",
					pkt->sport,
					pkt->dport,
					(pkt->ethType == ETHER_TYPE_IPv4)? "ipv4" :
							(pkt->ethType == ETHER_TYPE_IPv6)? "ipv6" :
							(pkt->ethType == ETHER_TYPE_VLAN)? "vlan" : "Other",
					(pkt->ipProto == PG_IPPROTO_TCP)? "tcp" :
							(pkt->ipProto == PG_IPPROTO_ICMP)? "icmp" : "udp",
					pkt->vlanid,
					pkt->pktSize+FCS_SIZE);
		}

		if ( pktgen.info[i].pcap ) {
			fprintf(fd, "#\n# PCAP port %d\n", i);
			fprintf(fd, "#    Packet count: %d\n", pktgen.info[i].pcap->pkt_count);
			fprintf(fd, "#    Filename    : %s\n", pktgen.info[i].pcap->filename);
		}
		fprintf(fd, "\n");
	}
	fprintf(fd, "################################ Done #################################\n");

	fclose(fd);
	return 0;
}

/**************************************************************************//**
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
    return (rte_atomic32_read(&pktgen.info[port].port_flags) & SENDING_PACKETS);
}

 /**************************************************************************//**
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
pktgen_link_state(int port, char * buff, int len)
{
	port_info_t * info = &pktgen.info[port];

    if (info->link.link_status) {
        snprintf(buff, len, "<UP-%u-%s>",
           (uint32_t) info->link.link_speed,
           (info->link.link_duplex == ETH_LINK_FULL_DUPLEX) ? ("FD") : ("HD"));
    } else
    	snprintf(buff, len, "<--Down-->");

    return buff;
}

/**************************************************************************//**
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
pktgen_transmit_count_rate(int port, char * buff, int len)
{
	port_info_t * info = &pktgen.info[port];

    if ( rte_atomic64_read(&info->transmit_count) == 0 )
        snprintf(buff, len, "Forever/%d%%", info->tx_rate);
    else
        snprintf(buff, len, "%ld/%d%%", rte_atomic64_read(&info->transmit_count), info->tx_rate);

    return buff;
}

/**************************************************************************//**
*
* pktgen_port_sizes - Current stats for all port sizes
*
* DESCRIPTION
* Structure returned with all of the counts for each port size.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

int
pktgen_port_sizes(int port, port_sizes_t * psizes)
{
	port_info_t * info = &pktgen.info[port];

	*psizes = info->sizes;
    return 0;
}

/**************************************************************************//**
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
pktgen_pkt_stats(int port, pkt_stats_t * pstats)
{
	port_info_t * info = &pktgen.info[port];

	*pstats = info->stats;
    return 0;
}

/**************************************************************************//**
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
pktgen_port_stats(int port, const char * name, eth_stats_t * pstats)
{
	port_info_t * info = &pktgen.info[port];

	if ( strcmp(name, "port") == 0 )
		*pstats = info->port_stats;
	else if ( strcmp(name, "rate") == 0 )
		*pstats = info->rate_stats;

	return 0;
}


/**************************************************************************//**
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
pktgen_flags_string( port_info_t * info )
{
    static char buff[32];
    uint32_t	flags = rte_atomic32_read(&info->port_flags);

    snprintf(buff, sizeof(buff), "%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            (pktgen.flags & PROMISCUOUS_ON_FLAG)? 'P' : '-',
            (flags & ICMP_ECHO_ENABLE_FLAG)? 'E' : '-',
            (flags & SEND_ARP_REQUEST)? 'A' : '-',
            (flags & SEND_GRATUITOUS_ARP)? 'G' : '-',
            (flags & SEND_PCAP_PKTS)? 'p' : '-',
            (flags & SEND_SEQ_PKTS)? 'S' : '-',
            (flags & SEND_RANGE_PKTS)? 'R' : '-',
            (flags & PROCESS_INPUT_PKTS)? 'I' : '-',
            "-rt*"[(flags & (PROCESS_RX_TAP_PKTS | PROCESS_TX_TAP_PKTS)) >> 9],
            (flags & SEND_VLAN_ID)? 'V' :
				(flags & SEND_MPLS_LABEL)? 'M' :
				(flags & SEND_Q_IN_Q_IDS)? 'Q' : '-',
            (flags & PROCESS_GARP_PKTS)? 'g' : '-',
			(flags & SEND_GRE_IPv4_HEADER)? 'g' :
				(flags & SEND_GRE_ETHER_HEADER)? 'G' : '-',
			(flags & CAPTURE_PKTS)? 'C' : '-',
			(flags & SEND_RANDOM_PKTS) ? 'R' : '-');

    return buff;
}

/**************************************************************************//**
*
* pktgen_redisplay - Redisplay the screen or clear the screen.
*
* DESCRIPTION
* Redisplay the screen or clear the screen based on flag.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_redisplay( int cls_flag )
{
	if ( wr_scrn_is_paused() )
		return;

	wr_scrn_pause();
	if ( cls_flag ) {
		wr_scrn_cls();
		wr_scrn_pos(100, 1);
	}
	pktgen.flags |= PRINT_LABELS_FLAG;
	wr_scrn_resume();

	pktgen_page_display(NULL, NULL);
}

/**************************************************************************//**
*
* pktgen_update_display - Update the display, but do not clear screen.
*
* DESCRIPTION
* Update the display, but do not clear the screen.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_update_display(void)
{
	pktgen_redisplay(0);
}

/**************************************************************************//**
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
	if ( (page_size > 0) && (page_size <= pktgen.nb_ports) && (page_size <= 6) ) {
		pktgen.nb_ports_per_page = page_size;
		pktgen.ending_port = pktgen.starting_port + page_size;
		if ( pktgen.ending_port >= (pktgen.starting_port + pktgen.nb_ports) )
			pktgen.ending_port = (pktgen.starting_port + pktgen.nb_ports);
		pktgen_redisplay(1);
	}
}

/**************************************************************************//**
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
pktgen_screen(const char * onOff)
{
	uint16_t rows;
	pktgen_display_get_geometry(&rows, NULL);

	if ( parseState(onOff) == DISABLE_STATE ) {
		if ( !wr_scrn_is_paused() ) {
			wr_scrn_pause();
			wr_scrn_cls();
			wr_scrn_setw(1);
			wr_scrn_pos(100, 1);
		}
	} else {
		wr_scrn_cls();
		wr_scrn_pos(100,1);
		wr_scrn_setw(pktgen.last_row+1);
		wr_scrn_resume();
		pktgen_redisplay(1);
	}
}

/**************************************************************************//**
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
pktgen_set_port_number(uint32_t port_number)
{
	if ( port_number <= pktgen.nb_ports ) {
		pktgen.portNum = port_number;
		pktgen_redisplay(1);
	}
}

/**************************************************************************//**
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
pktgen_set_icmp_echo(port_info_t * info, uint32_t onOff)
{
	if ( onOff == ENABLE_STATE )
		pktgen_set_port_flags(info, ICMP_ECHO_ENABLE_FLAG);
	else
		pktgen_clr_port_flags(info, ICMP_ECHO_ENABLE_FLAG);
}

/**************************************************************************//**
*
* pktgen_set_rx_tap - Enable or disable the Rx TAP interface
*
* DESCRIPTION
* Create and setup the Rx TAP interface.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_rx_tap(port_info_t * info, uint32_t onOff)
{
	if ( onOff == ENABLE_STATE ) {
		struct ifreq	ifr;
		int sockfd, i;
		static const char * tapdevs[] = { "/dev/net/tun", "/dev/tun", NULL };

		for(i = 0; tapdevs[i]; i++) {
			if ( (info->rx_tapfd = open(tapdevs[i], O_RDWR)) >= 0 ) {
				break;
			}
		}
		if ( tapdevs[i] == NULL ) {
			pktgen_log_error("Unable to create TUN/TAP interface");
			return;
		}
		memset(&ifr, 0, sizeof(struct ifreq));

		ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

		snprintf(ifr.ifr_name, IFNAMSIZ, "%s%d", "pg_rxtap", info->pid);
		if ( ioctl(info->rx_tapfd, TUNSETIFF, (void *)&ifr) < 0 ) {
			pktgen_log_error("Unable to set TUNSETIFF for %s", ifr.ifr_name);
			close(info->rx_tapfd);
			info->rx_tapfd = 0;
			return;
		}

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);

		ifr.ifr_flags = IFF_UP | IFF_RUNNING;
		if ( ioctl(sockfd, SIOCSIFFLAGS, (void *)&ifr) < 0 ) {
			pktgen_log_error("Unable to set SIOCSIFFLAGS for %s", ifr.ifr_name);
			close(sockfd);
			close(info->rx_tapfd);
			info->rx_tapfd = 0;
			return;
		}
		close(sockfd);
		pktgen_set_port_flags(info, PROCESS_RX_TAP_PKTS);
	} else {
		if ( rte_atomic32_read(&info->port_flags) & PROCESS_RX_TAP_PKTS ) {
			close(info->rx_tapfd);
			info->rx_tapfd = 0;
		}
		pktgen_clr_port_flags(info, PROCESS_RX_TAP_PKTS);
	}
}

/**************************************************************************//**
*
* pktgen_set_tx_tap - Enable or disable the Tx TAP interface
*
* DESCRIPTION
* Create and setup the Tx TAP interface.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_tx_tap(port_info_t * info, uint32_t onOff)
{
	if ( onOff == ENABLE_STATE ) {
		struct ifreq	ifr;
		int sockfd, i;
		static const char * tapdevs[] = { "/dev/net/tun", "/dev/tun", NULL };

		for(i = 0; tapdevs[i]; i++) {
			if ( (info->tx_tapfd = open(tapdevs[i], O_RDWR)) >= 0 ) {
				break;
			}
		}
		if ( tapdevs[i] == NULL ) {
			pktgen_log_error("Unable to create TUN/TAP interface.");
			return;
		}
		memset(&ifr, 0, sizeof(struct ifreq));

		ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

		snprintf(ifr.ifr_name, IFNAMSIZ, "%s%d", "pg_txtap", info->pid);
		if ( ioctl(info->tx_tapfd, TUNSETIFF, (void *)&ifr) < 0 ) {
			pktgen_log_error("Unable to set TUNSETIFF for %s", ifr.ifr_name);
			close(info->tx_tapfd);
			info->tx_tapfd = 0;
			return;
		}

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);

		ifr.ifr_flags = IFF_UP | IFF_RUNNING;
		if ( ioctl(sockfd, SIOCSIFFLAGS, (void *)&ifr) < 0 ) {
			pktgen_log_error("Unable to set SIOCSIFFLAGS for %s", ifr.ifr_name);
			close(sockfd);
			close(info->tx_tapfd);
			info->tx_tapfd = 0;
			return;
		}
		close(sockfd);
		pktgen_set_port_flags(info, PROCESS_TX_TAP_PKTS);
	} else {
		if ( rte_atomic32_read(&info->port_flags) & PROCESS_TX_TAP_PKTS ) {
			close(info->tx_tapfd);
			info->tx_tapfd = 0;
		}
		pktgen_clr_port_flags(info, PROCESS_TX_TAP_PKTS);
	}
}

/**************************************************************************//**
*
* pktgen_mac_from_arp - Enable or disable getting MAC from ARP requests.
*
* DESCRIPTION
* Enable or disable getting the MAC address from the ARP request packets.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_mac_from_arp(uint32_t onOff)
{
	if ( onOff == ENABLE_STATE )
		pktgen.flags |= MAC_FROM_ARP_FLAG;
	else
		pktgen.flags &= ~MAC_FROM_ARP_FLAG;
}

/**************************************************************************//**
*
* pktgen_set_random - Enable/disable random bitfield mode
*
* DESCRIPTION
* Enable/disable random bitfield mode
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_random(port_info_t *info, uint32_t onOff)
{
	if ( onOff == ENABLE_STATE )
		pktgen_set_port_flags(info, SEND_RANDOM_PKTS);
	else
		pktgen_clr_port_flags(info, SEND_RANDOM_PKTS);
}

/*
 * Local wrapper function to test mp is NULL and return or continue
 * to call rte_mempool_dump() routine.
 */
static void
__mempool_dump(FILE * f, struct rte_mempool * mp) {

	if ( mp == NULL )
		return;
	rte_mempool_dump(f, mp);
}

/**************************************************************************//**
*
* pktgen_mempool_dump - Display the mempool information
*
* DESCRIPTION
* Dump out the mempool information.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_mempool_dump(port_info_t * info, char * name)
{
	int				all;
	uint16_t		q;

	all = !strcmp(name, "all");

	if ( info->q[0].tx_mp == NULL )
		return;

	for(q = 0; q < wr_get_port_rxcnt(pktgen.l2p, info->pid); q++) {
		if ( all || !strcmp(name, "rx") )
			rte_mempool_dump(stdout, info->q[q].rx_mp);
	}
	for(q = 0; q < wr_get_port_txcnt(pktgen.l2p, info->pid); q++) {
		if ( all || (!strcmp(name, "tx") && (q < wr_get_port_txcnt(pktgen.l2p, info->pid))) )
			__mempool_dump(stdout, info->q[q].tx_mp);
		if ( all || !strcmp(name, "range") )
			__mempool_dump(stdout, info->q[q].range_mp);
		if ( all || !strcmp(name, "seq") )
			__mempool_dump(stdout, info->q[q].seq_mp);
		if ( all || !strcmp(name, "arp") )
			__mempool_dump(stdout, info->q[q].special_mp);
		if ( all || !strcmp(name, "pcap") )
			__mempool_dump(stdout, info->q[q].pcap_mp);
	}
}

/**************************************************************************//**
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
pktgen_start_transmitting(port_info_t * info)
{
	uint8_t		q;

	if ( !(rte_atomic32_read(&info->port_flags) & SENDING_PACKETS) ) {
		for(q = 0; q < wr_get_port_txcnt(pktgen.l2p, info->pid); q++ )
			pktgen_set_q_flags(info, q, CLEAR_FAST_ALLOC_FLAG);
		rte_atomic64_set(&info->current_tx_count, rte_atomic64_read(&info->transmit_count));
		pktgen_set_port_flags(info, SENDING_PACKETS);
		if ( rte_atomic64_read(&info->current_tx_count) == 0 )
			pktgen_set_port_flags(info, SEND_FOREVER);
	}
}

/**************************************************************************//**
*
* pktgen_stop_transmitting - Stop port transmitting packets.
*
* DESCRIPTION
* Stop the given ports from send traffic.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_stop_transmitting(port_info_t * info)
{
	uint8_t		q;

	if ( rte_atomic32_read(&info->port_flags) & SENDING_PACKETS ) {
		pktgen_clr_port_flags(info, (SENDING_PACKETS | SEND_FOREVER));
		for(q = 0; q < wr_get_port_txcnt(pktgen.l2p, info->pid); q++ )
			pktgen_set_q_flags(info, q, DO_TX_CLEANUP);
	}
}

/**************************************************************************//**
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
pktgen_prime_ports(port_info_t * info)
{
	uint8_t		q;

	rte_atomic64_set(&info->current_tx_count, info->prime_cnt);
	pktgen_set_port_flags(info, SENDING_PACKETS);
	for(q = 0; q < wr_get_port_txcnt(pktgen.l2p, info->pid); q++ )
		pktgen_set_q_flags(info, q, DO_TX_FLUSH);
}

/**************************************************************************//**
*
* pktgen_set_proto - Set up the protocol type for a port/packet.
*
* DESCRIPTION
* Setup all single packets with a protocol types with the port list.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_proto(port_info_t * info, char type)
{
	info->seq_pkt[SINGLE_PKT].ipProto = (type == 'u')? PG_IPPROTO_UDP :
									(type == 'i') ? PG_IPPROTO_ICMP :
									(type == 't') ? PG_IPPROTO_TCP :
									/* TODO print error: unknown type */ PG_IPPROTO_TCP;

	// ICMP only works on IPv4 packets.
	if ( type == 'i' )
		info->seq_pkt[SINGLE_PKT].ethType = ETHER_TYPE_IPv4;

	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_pcap_enable_disable - Enable or disable PCAP sending of packets.
*
* DESCRIPTION
* Enable or disable PCAP packet sending.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_pcap_enable_disable(port_info_t * info, char * str)
{
	if ( (info->pcap != NULL) && (info->pcap->pkt_count != 0) ) {
		if ( parseState(str) == ENABLE_STATE ) {
			pktgen_clr_port_flags(info, SEND_RANGE_PKTS);
			pktgen_clr_port_flags(info, SEND_SEQ_PKTS);
			pktgen_set_port_flags(info, SEND_PCAP_PKTS);
		} else
			pktgen_clr_port_flags(info, SEND_PCAP_PKTS);
		pktgen_packet_rate(info);
	}
}

/**************************************************************************//**
*
* pktgen_pcap_filter - Compile a PCAP filter for a portlist
*
* DESCRIPTION
* Compile a pcap filter for a portlist
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_pcap_filter(port_info_t * info, char * str)
{
	pcap_t * pc = pcap_open_dead(DLT_EN10MB, 65535);

	info->pcap_result = pcap_compile(pc, &info->pcap_program, str, 1, PCAP_NETMASK_UNKNOWN);

	pcap_close(pc);
}

/**************************************************************************//**
*
* pktgen_blink_enable_disable - Enable or disable a port from blinking.
*
* DESCRIPTION
* Enable or disable the given ports from blinking.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_blink_enable_disable(port_info_t * info, char * str)
{
	if ( parseState(str) == ENABLE_STATE ) {
		pktgen.blinklist |= (1 << info->pid);
	} else {
		pktgen.blinklist &= ~(1 << info->pid);
		rte_eth_led_on(info->pid);
	}
}

/**************************************************************************//**
*
* pktgen_process_enable_disable - Enable or disable input packet processing.
*
* DESCRIPTION
* Enable or disable input packet processing of ICMP, ARP, ...
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_process_enable_disable(port_info_t * info, char * str)
{
	if ( parseState(str) == ENABLE_STATE )
		pktgen_set_port_flags(info, PROCESS_INPUT_PKTS);
	else
		pktgen_clr_port_flags(info, PROCESS_INPUT_PKTS);
}

/**************************************************************************//**
*
* pktgen_garp_enable_disable - Enable or disable GARP packet processing.
*
* DESCRIPTION
* Enable or disable GARP packet processing of ICMP, ARP, ...
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_garp_enable_disable(port_info_t * info, char * str)
{
	if ( parseState(str) == ENABLE_STATE )
		pktgen_set_port_flags(info, PROCESS_GARP_PKTS | PROCESS_INPUT_PKTS);
	else
		pktgen_clr_port_flags(info, PROCESS_GARP_PKTS | PROCESS_INPUT_PKTS);
}

/**************************************************************************//**
*
* pktgen_set_pkt_type - Set the packet type value.
*
* DESCRIPTION
* Set the packet type value for the given port list.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_pkt_type(port_info_t * info, const char * type)
{
	info->seq_pkt[SINGLE_PKT].ethType = (type[0] == 'a') ? ETHER_TYPE_ARP  :
									    (type[3] == '4') ? ETHER_TYPE_IPv4 :
										(type[3] == '6') ? ETHER_TYPE_IPv6 :
										/* TODO print error: unknown type */ ETHER_TYPE_IPv4;

	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_set_vlan - Set the port to send a VLAN ID
*
* DESCRIPTION
* Set the given port list to send VLAN ID packets.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_vlan(port_info_t * info, uint32_t onOff)
{
	if ( onOff == ENABLE_STATE ) {
		pktgen_clr_port_flags(info, SEND_MPLS_LABEL);
		pktgen_clr_port_flags(info, SEND_Q_IN_Q_IDS);
		pktgen_set_port_flags(info, SEND_VLAN_ID);
	}
	else
		pktgen_clr_port_flags(info, SEND_VLAN_ID);
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_set_vlanid - Set the port VLAN ID value
*
* DESCRIPTION
* Set the given port list with the given VLAN ID.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_vlanid(port_info_t * info, uint16_t vlanid)
{
	info->vlanid = vlanid;
	info->seq_pkt[SINGLE_PKT].vlanid = info->vlanid;
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_set_mpls - Set the port to send a mpls ID
*
* DESCRIPTION
* Set the given port list to send mpls ID packets.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_mpls(port_info_t * info, uint32_t onOff)
{
	if ( onOff == ENABLE_STATE ) {
		pktgen_clr_port_flags(info, SEND_VLAN_ID);
		pktgen_clr_port_flags(info, SEND_Q_IN_Q_IDS);
		pktgen_set_port_flags(info, SEND_MPLS_LABEL);
	}
	else
		pktgen_clr_port_flags(info, SEND_MPLS_LABEL);
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_set_mpls_entry - Set the port MPLS entry value
*
* DESCRIPTION
* Set the given port list with the given MPLS entry.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_mpls_entry(port_info_t * info, uint32_t mpls_entry)
{
	info->mpls_entry = mpls_entry;
	info->seq_pkt[SINGLE_PKT].mpls_entry = info->mpls_entry;
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_set_qinq - Set the port to send a Q-in-Q header
*
* DESCRIPTION
* Set the given port list to send Q-in-Q ID packets.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_qinq(port_info_t * info, uint32_t onOff)
{
	if ( onOff == ENABLE_STATE ) {
		pktgen_clr_port_flags(info, SEND_VLAN_ID);
		pktgen_clr_port_flags(info, SEND_MPLS_LABEL);
		pktgen_set_port_flags(info, SEND_Q_IN_Q_IDS);
	}
	else
		pktgen_clr_port_flags(info, SEND_Q_IN_Q_IDS);
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_set_qinqids - Set the port Q-in-Q ID values
*
* DESCRIPTION
* Set the given port list with the given Q-in-Q ID's.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_qinqids(port_info_t * info, uint16_t outerid, uint16_t innerid)
{
	info->qinq_outerid = outerid;
	info->seq_pkt[SINGLE_PKT].qinq_outerid = info->qinq_outerid;
	info->qinq_innerid = innerid;
	info->seq_pkt[SINGLE_PKT].qinq_innerid = info->qinq_innerid;
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_set_gre - Set the port to send GRE with IPv4 payload
*
* DESCRIPTION
* Set the given port list to send GRE with IPv4 payload
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_gre(port_info_t * info, uint32_t onOff)
{
	if ( onOff == ENABLE_STATE ) {
		pktgen_clr_port_flags(info, SEND_GRE_ETHER_HEADER);
		pktgen_set_port_flags(info, SEND_GRE_IPv4_HEADER);
	}
	else
		pktgen_clr_port_flags(info, SEND_GRE_IPv4_HEADER);
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
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
pktgen_set_gre_eth(port_info_t * info, uint32_t onOff)
{
	if ( onOff == ENABLE_STATE ) {
		pktgen_clr_port_flags(info, SEND_GRE_IPv4_HEADER);
		pktgen_set_port_flags(info, SEND_GRE_ETHER_HEADER);
	}
	else
		pktgen_clr_port_flags(info, SEND_GRE_ETHER_HEADER);
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_set_gre_key - Set the port GRE key
*
* DESCRIPTION
* Set the given port list with the given GRE key.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_gre_key(port_info_t * info, uint32_t gre_key)
{
	info->gre_key = gre_key;
	info->seq_pkt[SINGLE_PKT].gre_key = info->gre_key;
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
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

void pktgen_clear_stats(port_info_t * info)
{
	memset(&info->sizes, 0, sizeof(port_sizes_t));
	memset(&info->port_stats, 0, sizeof(eth_stats_t));
	memset(&info->rate_stats, 0, sizeof(eth_stats_t));

	rte_eth_stats_get(info->pid, &info->init_stats);
	info->stats.dropped_pkts	= 0;
	info->stats.arp_pkts		= 0;
	info->stats.echo_pkts		= 0;
	info->stats.ip_pkts			= 0;
	info->stats.ipv6_pkts		= 0;
	info->stats.vlan_pkts		= 0;
	info->stats.unknown_pkts	= 0;
	info->stats.tx_failed		= 0;

	memset(&pktgen.cumm_rate_totals, 0, sizeof(eth_stats_t));

	pktgen_update_display();
}

/**************************************************************************//**
*
* pktgen_cls - Clear the screen.
*
* DESCRIPTION
* Clear the screen and redisplay the data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_cls(void)
{
	if ( wr_scrn_is_paused() ) {
		wr_scrn_cls();
		wr_scrn_pos(100, 1);
	} else	// Update the display quickly.
		pktgen_redisplay(1);
}

/**************************************************************************//**
*
* pktgen_update - Update the screen information
*
* DESCRIPTION
* Update the screen information
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_update(void)
{
	pktgen_page_display(NULL, NULL);
}

/**************************************************************************//**
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

void pktgen_port_defaults(uint32_t pid, uint8_t seq)
{
	port_info_t * info = &pktgen.info[pid];
	pkt_seq_t * pkt = &info->seq_pkt[seq];
	port_info_t * dst_info;

	pkt->pktSize			= MIN_PKT_SIZE;
	pkt->sport				= DEFAULT_SRC_PORT;
	pkt->dport				= DEFAULT_DST_PORT;
	pkt->ipProto			= PG_IPPROTO_TCP;
	pkt->ethType			= ETHER_TYPE_IPv4;
	pkt->vlanid				= DEFAULT_VLAN_ID;

	rte_atomic64_set(&info->transmit_count, DEFAULT_TX_COUNT);
	rte_atomic64_init(&info->current_tx_count);
	info->tx_rate			= DEFAULT_TX_RATE;
	info->tx_burst			= DEFAULT_PKT_BURST;
	info->vlanid			= DEFAULT_VLAN_ID;
	info->seqCnt			= 0;
	info->seqIdx			= 0;
	info->prime_cnt			= DEFAULT_PRIME_COUNT;
	info->delta				= 0;

	pktgen_packet_rate(info);

	pkt->ip_mask = DEFAULT_NETMASK;
	if ( (pid & 1) == 0 ) {
		pkt->ip_src_addr = DEFAULT_IP_ADDR | (pid << 8) | 1;
		pkt->ip_dst_addr = DEFAULT_IP_ADDR | ((pid+1) << 8) | 1;
		dst_info = info + 1;
	} else {
		pkt->ip_src_addr = DEFAULT_IP_ADDR | (pid << 8) | 1;
		pkt->ip_dst_addr = DEFAULT_IP_ADDR | ((pid-1) << 8) | 1;
		dst_info = info - 1;
	}

	if (dst_info->seq_pkt != NULL)
		ether_addr_copy(&dst_info->seq_pkt[SINGLE_PKT].eth_src_addr,
		    &pkt->eth_dst_addr);
	else
		memset(&pkt->eth_dst_addr, 0, sizeof (pkt->eth_dst_addr));


	pktgen_packet_ctor(info, seq, -1);

    pktgen.flags	|= PRINT_LABELS_FLAG;
}

/**************************************************************************//**
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

void pktgen_ping4(port_info_t * info)
{
	memcpy(&info->seq_pkt[PING_PKT],
		   &info->seq_pkt[SINGLE_PKT], sizeof(pkt_seq_t));
	info->seq_pkt[PING_PKT].ipProto = PG_IPPROTO_ICMP;
	pktgen_packet_ctor(info, PING_PKT, ICMP4_ECHO);
	pktgen_set_port_flags(info, SEND_PING4_REQUEST);
}

#ifdef INCLUDE_PING6
/**************************************************************************//**
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

void pktgen_ping6(port_info_t * info)
{
	memcpy(&info->pkt[PING_PKT],
		   &info->pkt[SINGLE_PKT], sizeof(pkt_seq_t));
	info->pkt[PING_PKT].ipProto = PG_IPPROTO_ICMP;
	pktgen_packet_ctor(info, PING_PKT, ICMP6_ECHO);
	pktgen_set_port_flags(info, SEND_PING6_REQUEST);
}
#endif

/**************************************************************************//**
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

void pktgen_reset(port_info_t * info)
{
	uint32_t	s;

	if ( info == NULL )
		info = &pktgen.info[0];

    pktgen_stop_transmitting(info);

	pktgen.flags &= ~MAC_FROM_ARP_FLAG;

	// Make sure the port is active and enabled.
	if ( info->seq_pkt ) {
		info->seq_pkt[SINGLE_PKT].pktSize = MIN_PKT_SIZE;

		for (s = 0; s < NUM_TOTAL_PKTS; s++)
			pktgen_port_defaults(info->pid, s);

		pktgen_range_setup(info);
		pktgen_clear_stats(info);
	}

    pktgen.flags &= ~PRINT_LABELS_FLAG;
	pktgen_update_display();
}

/**************************************************************************//**
*
* pktgen_set_tx_count - Set the number of packets to transmit on a port.
*
* DESCRIPTION
* Set the transmit count for all ports in the list.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_tx_count(port_info_t * info, uint32_t cnt)
{
	rte_atomic64_set(&info->transmit_count, cnt);
}

/**************************************************************************//**
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

void pktgen_set_port_seqCnt(port_info_t * info, uint32_t cnt)
{
	if ( cnt > NUM_SEQ_PKTS )
		cnt = NUM_SEQ_PKTS;

	info->seqCnt = cnt;
	if ( cnt ) {
		pktgen_clr_port_flags(info, SEND_RANGE_PKTS);
		pktgen_clr_port_flags(info, SEND_PCAP_PKTS);
		pktgen_set_port_flags(info, SEND_SEQ_PKTS);
	} else
		pktgen_clr_port_flags(info, SEND_SEQ_PKTS);
	pktgen_packet_rate(info);
}

/**************************************************************************//**
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

void pktgen_set_port_prime(port_info_t * info, uint32_t cnt)
{
	if ( cnt > MAX_PRIME_COUNT )
		cnt = MAX_PRIME_COUNT;
	else if ( cnt == 0 )
		cnt = DEFAULT_PRIME_COUNT;

	info->prime_cnt = cnt;
}

/**************************************************************************//**
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

void pktgen_set_port_dump(port_info_t * info, uint32_t cnt)
{
	int i;

	if ( cnt > MAX_DUMP_PACKETS )
		cnt = MAX_DUMP_PACKETS;

	// Prevent concurrency issues by setting the fields in this specific order
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

/**************************************************************************//**
*
* pktgen_set_tx_burst - Set the transmit burst count.
*
* DESCRIPTION
* Set the transmit burst count for all packets.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_tx_burst(port_info_t * info, uint32_t burst)
{
	if ( burst == 0 )
		burst = 1;
	else if ( burst > DEFAULT_PKT_BURST )
		burst = DEFAULT_PKT_BURST;
	info->tx_burst = burst;
	pktgen_packet_rate(info);
}

/**************************************************************************//**
*
* pktgen_set_tx_cycles - Set the number of Transmit cycles to use.
*
* DESCRIPTION
* Set the number of transmit cycles for the given port list.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_tx_cycles(port_info_t * info, uint32_t cycles)
{
	info->tx_cycles		= cycles;
}

/**************************************************************************//**
*
* pktgen_set_pkt_size - Set the size of the packets to send.
*
* DESCRIPTION
* Set the pkt size for the single packet transmit.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_pkt_size(port_info_t * info, uint32_t size)
{
	if ( (size - FCS_SIZE) < MIN_PKT_SIZE)
		size = (MIN_PKT_SIZE + FCS_SIZE);
	else if ( (size - FCS_SIZE) > MAX_PKT_SIZE)
		size = MAX_PKT_SIZE + FCS_SIZE;
	info->seq_pkt[SINGLE_PKT].pktSize = (size - FCS_SIZE);
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
	pktgen_packet_rate(info);
}

/**************************************************************************//**
*
* pktgen_set_port_value - Set the port value for single or sequence packets.
*
* DESCRIPTION
* Set the port value for single or sequence packets for the ports listed.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_port_value(port_info_t * info, char type, uint32_t portValue)
{
	if ( type == 'd' )
		info->seq_pkt[SINGLE_PKT].dport = (uint16_t)portValue;
	else
		info->seq_pkt[SINGLE_PKT].sport = (uint16_t)portValue;
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_set_tx_rate - Set the transmit rate as a percent value.
*
* DESCRIPTION
* Set the transmit rate as a percent value for all ports listed.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_tx_rate(port_info_t * info, uint32_t rate)
{
	if ( rate == 0 )
		rate = 1;
	else if ( rate > 100 )
		rate = 100;
	info->tx_rate = rate;
	pktgen_packet_rate(info);
}

/**************************************************************************//**
*
* pktgen_set_ipaddr - Set the IP address for all ports listed
*
* DESCRIPTION
* Set an IP address for all ports listed in the call.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_ipaddr(port_info_t * info, char type, cmdline_ipaddr_t * ip)
{
	if ( type == 's' ) {
		info->seq_pkt[SINGLE_PKT].ip_mask = size_to_mask(ip->prefixlen);
		info->seq_pkt[SINGLE_PKT].ip_src_addr = ntohl(ip->addr.ipv4.s_addr);
	} else
		info->seq_pkt[SINGLE_PKT].ip_dst_addr = ntohl(ip->addr.ipv4.s_addr);
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_set_dst_mac - Setup the destination MAC address
*
* DESCRIPTION
* Set the destination MAC address for all ports given.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_dst_mac(port_info_t * info, cmdline_etheraddr_t * mac)
{
	memcpy(&info->seq_pkt[SINGLE_PKT].eth_dst_addr, mac->mac, 6);
	pktgen_packet_ctor(info, SINGLE_PKT, -1);
}

/**************************************************************************//**
*
* pktgen_range_enable_disable - Enable or disable range packet sending.
*
* DESCRIPTION
* Enable or disable range packet sending.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_range_enable_disable(port_info_t * info, char * str)
{
	if ( parseState(str) == ENABLE_STATE ) {
		pktgen_clr_port_flags(info, SEND_SEQ_PKTS);
		pktgen_clr_port_flags(info, SEND_PCAP_PKTS);
		pktgen_set_port_flags(info, SEND_RANGE_PKTS);
	} else
		pktgen_clr_port_flags(info, SEND_RANGE_PKTS);
	pktgen_packet_rate(info);
}

/**************************************************************************//**
*
* pktgen_set_dest_mac - Set the destination MAC address
*
* DESCRIPTION
* Set the destination MAC address for all ports given.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_dest_mac(port_info_t * info, const char * what, cmdline_etheraddr_t * mac)
{
	if ( !strcmp(what, "min") )
		inet_mtoh64((struct ether_addr *)mac, &info->range.dst_mac_min);
	else if ( !strcmp(what, "max") )
		inet_mtoh64((struct ether_addr *)mac, &info->range.dst_mac_max);
	else if ( !strcmp(what, "inc") )
		inet_mtoh64((struct ether_addr *)mac, &info->range.dst_mac_inc);
	else if ( !strcmp(what, "start") )
		inet_mtoh64((struct ether_addr *)mac, &info->range.dst_mac);
}

/**************************************************************************//**
*
* pktgen_set_src_mac - Set the source MAC address for the ports.
*
* DESCRIPTION
* Set the source MAC address for the ports given in the list.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_set_src_mac(port_info_t * info, const char * what, cmdline_etheraddr_t * mac)
{
	if ( !strcmp(what, "min") )
		inet_mtoh64((struct ether_addr *)mac, &info->range.src_mac_min);
	else if ( !strcmp(what, "max") )
		inet_mtoh64((struct ether_addr *)mac, &info->range.src_mac_max);
	else if ( !strcmp(what, "inc") )
		inet_mtoh64((struct ether_addr *)mac, &info->range.src_mac_inc);
	else if ( !strcmp(what, "start") )
		inet_mtoh64((struct ether_addr *)mac, &info->range.src_mac);
}

/**************************************************************************//**
*
* pktgen_set_src_ip - Set the source IP address value.
*
* DESCRIPTION
* Set the source IP address for all of the ports listed.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_src_ip(port_info_t * info, char * what, cmdline_ipaddr_t * ip)
{
	if ( !strcmp(what, "min") )
		info->range.src_ip_min = ntohl(ip->addr.ipv4.s_addr);
	else if ( !strcmp(what, "max") )
		info->range.src_ip_max = ntohl(ip->addr.ipv4.s_addr);
	else if ( !strcmp(what, "inc") )
		info->range.src_ip_inc = ntohl(ip->addr.ipv4.s_addr);
	else if ( !strcmp(what, "start") )
		info->range.src_ip = ntohl(ip->addr.ipv4.s_addr);
}

/**************************************************************************//**
*
* pktgen_set_dst_ip - Set the destination IP address values
*
* DESCRIPTION
* Set the destination IP address values.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_dst_ip(port_info_t * info, char * what, cmdline_ipaddr_t * ip)
{
	if ( !strcmp(what, "min") )
		info->range.dst_ip_min = ntohl(ip->addr.ipv4.s_addr);
	else if ( !strcmp(what, "max") )
		info->range.dst_ip_max = ntohl(ip->addr.ipv4.s_addr);
	else if ( !strcmp(what, "inc") )
		info->range.dst_ip_inc = ntohl(ip->addr.ipv4.s_addr);
	else if ( !strcmp(what, "start") )
		info->range.dst_ip = ntohl(ip->addr.ipv4.s_addr);
}

/**************************************************************************//**
*
* pktgen_set_src_port - Set the source IP port number for the ports
*
* DESCRIPTION
* Set the source IP port number for the ports listed.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_src_port(port_info_t * info, char * what, uint16_t port)
{
	if ( !strcmp(what, "inc") ) {
		if ( port > 64 )
			port = 64;
		info->range.src_port_inc = port;
	} else {
		if ( !strcmp(what, "min") )
			info->range.src_port_min = port;
		else if ( !strcmp(what, "max") )
			info->range.src_port_max = port;
		else if ( !strcmp(what, "start") )
			info->range.src_port = port;
	}
}

/**************************************************************************//**
*
* pktgen_set_dst_port - Set the destination port value
*
* DESCRIPTION
* Set the destination port values.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_dst_port(port_info_t * info, char * what, uint16_t port)
{
	if ( !strcmp(what, "inc") ) {
		if ( port > 64 )
			port = 64;
		info->range.dst_port_inc = port;
	} else {
		if ( !strcmp(what, "min") )
			info->range.dst_port_min = port;
		else if ( !strcmp(what, "max") )
			info->range.dst_port_max = port;
		else if ( !strcmp(what, "start") )
			info->range.dst_port = port;
	}
}

/**************************************************************************//**
*
* pktgen_set_vlan_id - Set the VLAN id value
*
* DESCRIPTION
* Set the VLAN id values.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_vlan_id(port_info_t * info, char * what, uint16_t id)
{
	if ( !strcmp(what, "inc") ) {
		if ( id > 64 )
			id = 64;
		info->range.vlan_id_inc = id;
	} else {
		if ( id < MIN_VLAN_ID )
			id = MIN_VLAN_ID;
		else if ( id > MAX_VLAN_ID )
			id = MAX_VLAN_ID;

		if ( !strcmp(what, "min") )
			info->range.vlan_id_min = id;
		else if ( !strcmp(what, "max") )
			info->range.vlan_id_max = id;
		else if ( !strcmp(what, "start") )
			info->range.vlan_id = id;
	}
}

/**************************************************************************//**
*
* pktgen_set_range_pkt_size - Set the Packet size value
*
* DESCRIPTION
* Set the packet size values.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_range_pkt_size(port_info_t * info, char * what, uint16_t size)
{
	if ( !strcmp(what, "inc") ) {
		if ( size > ETHER_MIN_LEN )
			size = ETHER_MIN_LEN;

		info->range.pkt_size_inc = size;
	} else {
		if ( size < ETHER_MIN_LEN )
			size = MIN_PKT_SIZE;
		else if ( size > ETHER_MAX_LEN )
			size = MAX_PKT_SIZE;
		else
			size -= FCS_SIZE;

		if ( !strcmp(what, "start") )
			info->range.pkt_size = size;
		else if ( !strcmp(what, "min") )
			info->range.pkt_size_min = size;
		else if ( !strcmp(what, "max") )
			info->range.pkt_size_max = size;
	}
}

/**************************************************************************//**
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

void pktgen_send_arp_requests(port_info_t * info, uint32_t type)
{
	if ( type == GRATUITOUS_ARP )
		pktgen_set_port_flags(info, SEND_GRATUITOUS_ARP);
	else
		pktgen_set_port_flags(info, SEND_ARP_REQUEST);
}

/**************************************************************************//**
*
* pktgen_set_page - Set the page type to display
*
* DESCRIPTION
* Set the page type ot display
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_set_page( char * str )
{
	uint16_t	page;

	if ( str == NULL )
		return;

	page = atoi(str);
	if ( page > pktgen.nb_ports )
		return;

	// Switch to the correct page
	if ( str[0] == 'n' ) {
		pcap_info_t	* pcap = pktgen.info[pktgen.portNum].pcap;

		if ( pcap ) {
			if ( (pcap->pkt_idx + PCAP_PAGE_SIZE) < pcap->pkt_count )
				pcap->pkt_idx += PCAP_PAGE_SIZE;
			else
				pcap->pkt_idx = 0;
		}
		pktgen.flags |= PRINT_LABELS_FLAG;
	} else if ( (str[0] == 'c') && (str[1] == 'p') ) {
		pktgen.flags &= ~PAGE_MASK_BITS;
		pktgen.flags |= CPU_PAGE_FLAG;
	} else if ( str[0] == 'p' ) {
		pktgen.flags &= ~PAGE_MASK_BITS;
		pktgen.flags |= PCAP_PAGE_FLAG;
		if ( pktgen.info[pktgen.portNum].pcap )
			pktgen.info[pktgen.portNum].pcap->pkt_idx = 0;
	} else if ( ( str[0] == 'r' ) && (str[1] == 'a') ) {
		pktgen.flags &= ~PAGE_MASK_BITS;
		pktgen.flags |= RANGE_PAGE_FLAG;
	} else if ( str[0] == 'c' ) {
		pktgen.flags &= ~PAGE_MASK_BITS;
		pktgen.flags |= CONFIG_PAGE_FLAG;
	} else if ( str[0] == 's' ) {
		pktgen.flags &= ~PAGE_MASK_BITS;
		pktgen.flags |= SEQUENCE_PAGE_FLAG;
	} else if ( str[0] == 'r') {
		pktgen.flags &= ~PAGE_MASK_BITS;
		pktgen.flags |= RND_BITFIELD_PAGE_FLAG;
	} else if ( str[0] == 'l') {
		pktgen.flags &= ~PAGE_MASK_BITS;
		pktgen.flags |= LOG_PAGE_FLAG;
	} else {
		uint16_t start_port;
		if ( str[0] == 'm' )
			page = 0;
		start_port = (page * pktgen.nb_ports_per_page);
		if ( (pktgen.starting_port != start_port) && (start_port < pktgen.nb_ports) ) {
			pktgen.starting_port	= start_port;
			pktgen.ending_port		= start_port + pktgen.nb_ports_per_page;
			if ( pktgen.ending_port > (pktgen.starting_port + pktgen.nb_ports) )
				pktgen.ending_port = (pktgen.starting_port + pktgen.nb_ports);
		}
		if ( pktgen.flags & PAGE_MASK_BITS ) {
			pktgen.flags &= ~PAGE_MASK_BITS;
			pktgen.flags |= PRINT_LABELS_FLAG;
		}
	}
	pktgen_redisplay(1);
}

/**************************************************************************//**
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

void pktgen_set_seq(port_info_t * info, uint32_t seqnum,
		cmdline_etheraddr_t *daddr, cmdline_etheraddr_t * saddr,
		cmdline_ipaddr_t * ip_daddr, cmdline_ipaddr_t * ip_saddr,
		uint32_t sport, uint32_t dport, char type, char proto,
		uint16_t vlanid, uint32_t pktsize)
{
	pkt_seq_t	  * pkt;

	pkt = &info->seq_pkt[seqnum];
	memcpy(&pkt->eth_dst_addr, daddr->mac, 6);
	memcpy(&pkt->eth_src_addr, saddr->mac, 6);
	pkt->ip_mask		= size_to_mask(ip_saddr->prefixlen);
	pkt->ip_src_addr	= htonl(ip_saddr->addr.ipv4.s_addr);
	pkt->ip_dst_addr 	= htonl(ip_daddr->addr.ipv4.s_addr);
	pkt->dport			= dport;
	pkt->sport			= sport;
	pkt->pktSize		= pktsize-FCS_SIZE;
	pkt->ipProto		= (proto == 'u')? PG_IPPROTO_UDP :
						  (proto == 'i')? PG_IPPROTO_ICMP : PG_IPPROTO_TCP;
	// Force the IP protocol to IPv4 if this is a ICMP packet.
	if ( proto == 'i' )
		type = '4';
	pkt->ethType		= (type == '6')? ETHER_TYPE_IPv6 : ETHER_TYPE_IPv4;
	pkt->vlanid			= vlanid;
	pktgen_packet_ctor(info, seqnum, -1);
}

/**************************************************************************//**
*
* pktgen_compile_pkt - Compile a packet for a given port.
*
* DESCRIPTION
* Compile a packet for a given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_compile_pkt(port_info_t * info, uint32_t seqnum,
		cmdline_etheraddr_t *daddr, cmdline_etheraddr_t * saddr,
		cmdline_ipaddr_t * ip_daddr, cmdline_ipaddr_t * ip_saddr,
		uint32_t sport, uint32_t dport, char type, char proto,
		uint16_t vlanid, uint32_t pktsize)
{
	pkt_seq_t	  * pkt;

	if ( seqnum >= NUM_EXTRA_TX_PKTS )
		return;

	pkt = &info->seq_pkt[seqnum + EXTRA_TX_PKT];

	memcpy(&pkt->eth_dst_addr, daddr->mac, 6);
	memcpy(&pkt->eth_src_addr, saddr->mac, 6);
	pkt->ip_mask		= size_to_mask(ip_saddr->prefixlen);
	pkt->ip_src_addr	= htonl(ip_saddr->addr.ipv4.s_addr);
	pkt->ip_dst_addr 	= htonl(ip_daddr->addr.ipv4.s_addr);
	pkt->dport			= dport;
	pkt->sport			= sport;
	pkt->pktSize		= pktsize-FCS_SIZE;
	pkt->ipProto		= (proto == 'u')? PG_IPPROTO_UDP :
						  (proto == 'i')? PG_IPPROTO_ICMP : PG_IPPROTO_TCP;
	// Force the IP protocol to IPv4 if this is a ICMP packet.
	if ( proto == 'i' )
		type = '4';
	pkt->ethType		= (type == '4')? ETHER_TYPE_IPv4 :
						  (type == '6')? ETHER_TYPE_IPv6 :
						  (type == 'n')? ETHER_TYPE_VLAN : ETHER_TYPE_IPv4;
	pkt->vlanid			= vlanid;
	pktgen_packet_ctor(info, seqnum, -1);
}

/**************************************************************************//**
*
* pktgen_send_pkt - Send a packet from the sequence array.
*
* DESCRIPTION
* Send a packet from the special pkt_seq_t structures. Seqnum is the 0-N
* index value into the info.seq_pkt[] array for EXTRA_TX_PKTS.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_send_pkt(port_info_t * info, uint32_t seqnum)
{
	pktgen_send_seq_pkt(info, seqnum + EXTRA_TX_PKT);
}

/**************************************************************************//**
*
* pktgen_recv_pkt - Receive a packet from the sequence array.
*
* DESCRIPTION
* Receive a packet from the special pkt_seq_t structures.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void pktgen_recv_pkt(__attribute__ ((unused)) port_info_t * info)
{
	pktgen_log_warning("Not working yet!");
}

/**************************************************************************//**
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

void pktgen_quit(void)
{
	cmdline_quit(pktgen.cl);
}
