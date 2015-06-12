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


#include <stdint.h>

#include "pktgen.h"
#include "pktgen-gre.h"
#include "pktgen-tcp.h"
#include "pktgen-ipv4.h"
#include "pktgen-ipv6.h"
#include "pktgen-udp.h"
#include "pktgen-arp.h"
#include "pktgen-vlan.h"
#include "pktgen-cpu.h"
#include "pktgen-display.h"
#include "pktgen-random.h"
#include "pktgen-log.h"


// Allocated the pktgen structure for global use
pktgen_t        pktgen;

/**************************************************************************//**
*
* pktgen_wire_size - Calculate the wire size of the data to be sent.
*
* DESCRIPTION
* Calculate the number of bytes/bits in a burst of traffic.
*
* RETURNS: Number of bits in burst of packets.
*
* SEE ALSO:
*/

static __inline__ uint64_t
pktgen_wire_size( port_info_t * info ) {
    uint64_t    i, size = 0;

	if ( rte_atomic32_read(&info->port_flags) & SEND_PCAP_PKTS )
		size = info->pcap->pkt_size + PKT_PREAMBLE_SIZE + INTER_FRAME_GAP + FCS_SIZE;
	else {
		if ( unlikely(info->seqCnt > 0) ) {
			for(i = 0; i < info->seqCnt; i++)
				size += info->seq_pkt[i].pktSize + PKT_PREAMBLE_SIZE + INTER_FRAME_GAP + FCS_SIZE;
			size = size / info->seqCnt;		// Calculate the average sized packet
		} else
			size = info->seq_pkt[SINGLE_PKT].pktSize + PKT_PREAMBLE_SIZE + INTER_FRAME_GAP + FCS_SIZE;
	}
    return size;
}

/**************************************************************************//**
*
* pktgen_packet_rate - Calculate the transmit rate.
*
* DESCRIPTION
* Calculate the number of cycles to wait between sending bursts of traffic.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_packet_rate(port_info_t * info)
{
                          /*   0   1   2   3   4   5   6   7   8   9  10 */
	static int64_t ff[11] = { 31, 30, 25, 30, 17, 17, 17, 20, 50, 60, 90 };
    uint64_t    wire_size = (pktgen_wire_size(info) * 8);
    uint64_t	link = (uint64_t)info->link.link_speed * Million;
    uint64_t    pps = ((link/wire_size) * info->tx_rate)/100;
    uint64_t	cpp = (pps > 0) ? (pktgen.hz/pps) : (pktgen.hz / 4);

    info->tx_pps		= pps;
    info->tx_cycles 	= ((cpp * info->tx_burst) / wr_get_port_txcnt(pktgen.l2p, info->pid));
	info->tx_cycles		-= ff[info->tx_rate/10];
}

/**************************************************************************//**
*
* pktgen_fill_pattern - Create the fill pattern in a packet buffer.
*
* DESCRIPTION
* Create a fill pattern based on the arguments for the packet data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
pktgen_fill_pattern( uint8_t * p, uint32_t len, uint32_t type ) {
    uint32_t    i;

    switch(type) {
    case 1:                 // Byte wide ASCII pattern
        for(i = 0; i < len; i++)
            p[i] = "abcdefghijklmnopqrstuvwxyz012345"[i & 0x1f];
        break;
    default: memset(p, 0, len); break;
    }
}

/**************************************************************************//**
*
* pktgen_find_matching_ipsrc - Find the matching IP source address
*
* DESCRIPTION
* locate and return the pkt_seq_t pointer to the match IP address.
*
* RETURNS: pkt_seq_t  * or NULL
*
* SEE ALSO:
*/

pkt_seq_t *
pktgen_find_matching_ipsrc( port_info_t * info, uint32_t addr )
{
	pkt_seq_t * pkt = NULL;
	int		i;

	addr = ntohl(addr);

	/* Search the sequence packets for a match */
	for(i = 0; i < info->seqCnt; i++) {
		if ( addr == info->seq_pkt[i].ip_src_addr ) {
			pkt = &info->seq_pkt[i];
			break;
		}
	}

	/* Now try to match the single packet address */
	if ( pkt == NULL ) {
		if ( addr == info->seq_pkt[SINGLE_PKT].ip_src_addr )
			pkt = &info->seq_pkt[SINGLE_PKT];
	}

	return pkt;
}

/**************************************************************************//**
*
* pktgen_find_matching_ipdst - Find the matching IP destination address
*
* DESCRIPTION
* locate and return the pkt_seq_t pointer to the match IP address.
*
* RETURNS: pkt_seq_t  * or NULL
*
* SEE ALSO:
*/

pkt_seq_t *
pktgen_find_matching_ipdst( port_info_t * info, uint32_t addr )
{
	pkt_seq_t * pkt = NULL;
	int		i;

	addr = ntohl(addr);

	/* Search the sequence packets for a match */
	for(i = 0; i < info->seqCnt; i++) {
		if ( addr == info->seq_pkt[i].ip_dst_addr ) {
			pkt = &info->seq_pkt[i];
			break;
		}
	}

	/* Now try to match the single packet address */
	if ( pkt == NULL ) {
		if ( addr == info->seq_pkt[SINGLE_PKT].ip_dst_addr )
			pkt = &info->seq_pkt[SINGLE_PKT];
	}

	/* Now try to match the range packet address */
	if ( pkt == NULL ) {
		if ( addr == info->seq_pkt[RANGE_PKT].ip_dst_addr )
			pkt = &info->seq_pkt[RANGE_PKT];
	}

	return pkt;
}

/**************************************************************************//**
*
* pktgen_send_burst - Send a burst of packets.
*
* DESCRIPTION
* Transmit a burst of packets to a given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
pktgen_send_burst(port_info_t * info, uint16_t qid)
{
	struct mbuf_table	* mtab = &info->q[qid].tx_mbufs;
	struct rte_mbuf **pkts;
    uint32_t ret, cnt, i, flags;

    if ( unlikely((cnt = mtab->len) == 0) )
    	return;
    mtab->len = 0;

	pkts	= mtab->m_table;
	flags	= rte_atomic32_read(&info->port_flags);
	do {
		if ( unlikely(flags & SEND_RANDOM_PKTS) )
			pktgen_rnd_bits_apply(pkts, cnt, info->rnd_bitfields);

    	ret = rte_eth_tx_burst(info->pid, qid, pkts, cnt);

		if ( unlikely(flags & PROCESS_TX_TAP_PKTS) ) {
			for(i = 0; i < ret; i++) {
				if ( write(info->tx_tapfd, rte_pktmbuf_mtod(pkts[i], char *), pkts[i]->pkt_len) < 0 )
					pktgen_log_error("Write failed for tx_tap%d", info->pid);
			}
		}
		pkts += ret;
		cnt -= ret;
	} while( cnt > 0 );
}

/**************************************************************************//**
*
* pktgen_tx_flush - Flush Tx buffers from ring.
*
* DESCRIPTION
* Flush TX buffers from ring.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
pktgen_tx_flush(port_info_t * info, uint16_t qid)
{
	// Flush any queued pkts to the driver.
	pktgen_send_burst(info, qid);

	pktgen_clr_q_flags(info, qid, DO_TX_FLUSH);
}

/**************************************************************************//**
*
* pktgen_tx_cleanup - Handle the transmit cleanup and flush tx buffers.
*
* DESCRIPTION
* Routine to force the tx done routine to cleanup transmit buffers.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
pktgen_tx_cleanup(port_info_t * info, uint16_t qid)
{
	// Flush any done transmit buffers and descriptors.
	pktgen_send_burst(info, qid);

	rte_delay_us(500);

	// Stop and start the device to flush TX and RX buffers from the device rings.
	rte_eth_dev_stop(info->pid);

	rte_delay_us(250);

	rte_eth_dev_start(info->pid);

	pktgen_clr_q_flags(info, qid, DO_TX_CLEANUP);
}

/**************************************************************************//**
*
* pktgen_exit_cleanup - Clean up the hyperscan data and other items
*
* DESCRIPTION
* Clean up the hyperscan data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
pktgen_exit_cleanup(uint8_t lid)
{
	port_info_t	* info;
	uint8_t		idx, pid, qid;

	for( idx = 0; idx < wr_get_lcore_txcnt(pktgen.l2p, lid); idx++ ) {
		pid = wr_get_tx_pid(pktgen.l2p, lid, idx);
		if ( (info = (port_info_t *)wr_get_port_private(pktgen.l2p, pid)) != NULL ) {
			qid = wr_get_txque(pktgen.l2p, lid, pid);
			pktgen_set_q_flags(info, qid, DO_TX_CLEANUP);
			pktgen_tx_cleanup(info, qid);
		}
	}
}

/**************************************************************************//**
*
* pktgen_has_work - Determine if lcore has work to do, if not wait for stop.
*
* DESCRIPTION
* If lcore has work to do then return zero else spin till stopped and return 1.
*
* RETURNS: 0 or 1
*
* SEE ALSO:
*/

static __inline__ int pktgen_has_work(void) {

	if ( ! wr_get_map(pktgen.l2p, RTE_MAX_ETHPORTS, rte_lcore_id()) ) {
        pktgen_log_warning("Nothing to do on lcore %d: exiting", rte_lcore_id());
        return 1;
    }
    return 0;
}

/**************************************************************************//**
*
* pktgen_packet_ctor - Construct a complete packet with all headers and data.
*
* DESCRIPTION
* Construct a packet type based on the arguments passed with all headers.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_packet_ctor(port_info_t * info, int32_t seq_idx, int32_t type) {
	pkt_seq_t		  * pkt = &info->seq_pkt[seq_idx];
	struct ether_hdr  * eth = (struct ether_hdr *)&pkt->hdr.eth;
	uint16_t			tlen;

    // Fill in the pattern for data space.
    pktgen_fill_pattern((uint8_t *)&pkt->hdr, (sizeof(pkt_hdr_t) + sizeof(pkt->pad)), 1);

	char *ether_hdr = pktgen_ether_hdr_ctor(info, pkt, eth);

	/* Add GRE header and adjust ether_hdr pointer if requested */
	if (rte_atomic32_read(&info->port_flags) & SEND_GRE_IPv4_HEADER) {
		ether_hdr = pktgen_gre_hdr_ctor(info, pkt, (greIp_t *)ether_hdr);
	}
	else if (rte_atomic32_read(&info->port_flags) & SEND_GRE_ETHER_HEADER) {
		ether_hdr = pktgen_gre_ether_hdr_ctor(info, pkt, (greEther_t *)ether_hdr);
	}

    if ( likely(pkt->ethType == ETHER_TYPE_IPv4) ) {

		if ( likely(pkt->ipProto == PG_IPPROTO_TCP) ) {
			tcpip_t	  * tip;

			// Start from Ethernet header
			tip = (tcpip_t *)ether_hdr;

			// Construct the TCP header
			pktgen_tcp_hdr_ctor(pkt, tip, ETHER_TYPE_IPv4);

			// IPv4 Header constructor
			pktgen_ipv4_ctor(pkt, (ipHdr_t *)tip);

			pkt->tlen = pkt->ether_hdr_size + sizeof(ipHdr_t) + sizeof(tcpHdr_t);

		} else if ( pkt->ipProto == PG_IPPROTO_UDP ) {
			udpip_t	  * udp;

			// Construct the Ethernet header
			//udp = (udpip_t *)pktgen_ether_hdr_ctor(info, pkt, eth);
			udp = (udpip_t *)ether_hdr;

			// Construct the UDP header
			pktgen_udp_hdr_ctor(pkt, udp, ETHER_TYPE_IPv4);

			// IPv4 Header constructor
			pktgen_ipv4_ctor(pkt, (ipHdr_t *)udp);

			pkt->tlen = pkt->ether_hdr_size + sizeof(ipHdr_t) + sizeof(udpHdr_t);

		} else if ( pkt->ipProto == PG_IPPROTO_ICMP ) {
			udpip_t           * uip;
			icmpv4Hdr_t       * icmp;

			// Start from Ethernet header
			uip = (udpip_t *)ether_hdr;

			// Create the ICMP header
			uip->ip.src         = htonl(pkt->ip_src_addr);
			uip->ip.dst         = htonl(pkt->ip_dst_addr);
			tlen           		= pkt->pktSize - (pkt->ether_hdr_size + sizeof(ipHdr_t));
			uip->ip.len         = htons(tlen);
			uip->ip.proto       = pkt->ipProto;

			icmp = (icmpv4Hdr_t *)&uip->udp;
			icmp->code                      = 0;
			if ( (type == -1) || (type == ICMP4_TIMESTAMP) ) {
				icmp->type                      = ICMP4_TIMESTAMP;
				icmp->data.timestamp.ident      = 0x1234;
				icmp->data.timestamp.seq        = 0x5678;
				icmp->data.timestamp.originate  = 0x80004321;
				icmp->data.timestamp.receive    = 0;
				icmp->data.timestamp.transmit   = 0;
			} else if ( type == ICMP4_ECHO ) {
				icmp->type                      = ICMP4_ECHO;
				icmp->data.echo.ident      		= 0x1234;
				icmp->data.echo.seq        		= 0x5678;
				icmp->data.echo.data			= 0;
			}
			icmp->cksum     = 0;
			tlen       		= pkt->pktSize - (pkt->ether_hdr_size + sizeof(ipHdr_t)); //ICMP4_TIMESTAMP_SIZE
			icmp->cksum     = cksum(icmp, tlen, 0);
			if ( icmp->cksum == 0 )
				icmp->cksum = 0xFFFF;

			// IPv4 Header constructor
			pktgen_ipv4_ctor(pkt, (ipHdr_t *)uip);

			pkt->tlen = pkt->ether_hdr_size + sizeof(ipHdr_t) + ICMP4_TIMESTAMP_SIZE;
		}
    } else if ( pkt->ethType == ETHER_TYPE_IPv6 ) {
		if ( pkt->ipProto == PG_IPPROTO_TCP ) {
			uint32_t            addr;
			tcpipv6_t         * tip;

			// Start from Ethernet header
			tip = (tcpipv6_t *)ether_hdr;

			// Create the pseudo header and TCP information
			addr                = htonl(pkt->ip_dst_addr);
			(void)rte_memcpy(&tip->ip.daddr[8], &addr, sizeof(uint32_t));
			addr                = htonl(pkt->ip_src_addr);
			(void)rte_memcpy(&tip->ip.saddr[8], &addr, sizeof(uint32_t));

			tlen           		= sizeof(tcpHdr_t) + (pkt->pktSize - pkt->ether_hdr_size - sizeof(ipv6Hdr_t) - sizeof(tcpHdr_t));
			tip->ip.tcp_length  = htonl(tlen);
			tip->ip.next_header = pkt->ipProto;

			tip->tcp.sport      = htons(pkt->sport);
			tip->tcp.dport      = htons(pkt->dport);
			tip->tcp.seq        = htonl(DEFAULT_PKT_NUMBER);
			tip->tcp.ack        = htonl(DEFAULT_ACK_NUMBER);
			tip->tcp.offset     = ((sizeof(tcpHdr_t)/sizeof(uint32_t)) << 4);   /* Offset in words */
			tip->tcp.window     = htons(DEFAULT_WND_SIZE);
			tip->tcp.urgent     = 0;
			tip->tcp.flags      = ACK_FLAG;     /* ACK */

			tlen           		= sizeof(tcpipv6_t) + (pkt->pktSize - pkt->ether_hdr_size - sizeof(ipv6Hdr_t) - sizeof(tcpHdr_t));
			tip->tcp.cksum      = cksum(tip, tlen, 0);

			// IPv6 Header constructor
			pktgen_ipv6_ctor(pkt, (ipv6Hdr_t *)&tip->ip);

			pkt->tlen = sizeof(tcpHdr_t) + pkt->ether_hdr_size + sizeof(ipv6Hdr_t);
			if ( unlikely(pkt->pktSize < pkt->tlen) )
				pkt->pktSize = pkt->tlen;

		} else if ( pkt->ipProto == PG_IPPROTO_UDP ) {
			uint32_t            addr;
			udpipv6_t         * uip;

			// Start from Ethernet header
			uip = (udpipv6_t *)ether_hdr;

			// Create the pseudo header and TCP information
			addr                = htonl(pkt->ip_dst_addr);
			(void)rte_memcpy(&uip->ip.daddr[8], &addr, sizeof(uint32_t));
			addr                = htonl(pkt->ip_src_addr);
			(void)rte_memcpy(&uip->ip.saddr[8], &addr, sizeof(uint32_t));

			tlen           		= sizeof(udpHdr_t) + (pkt->pktSize - pkt->ether_hdr_size - sizeof(ipv6Hdr_t) - sizeof(udpHdr_t));
			uip->ip.tcp_length  = htonl(tlen);
			uip->ip.next_header = pkt->ipProto;

			uip->udp.sport      = htons(pkt->sport);
			uip->udp.dport      = htons(pkt->dport);

			tlen           		= sizeof(udpipv6_t) + (pkt->pktSize - pkt->ether_hdr_size - sizeof(ipv6Hdr_t) - sizeof(udpHdr_t));
			uip->udp.cksum      = cksum(uip, tlen, 0);
			if ( uip->udp.cksum == 0 )
				uip->udp.cksum = 0xFFFF;

			// IPv6 Header constructor
			pktgen_ipv6_ctor(pkt, (ipv6Hdr_t *)&uip->ip);

			pkt->tlen = sizeof(udpHdr_t) + pkt->ether_hdr_size + sizeof(ipv6Hdr_t);
			if ( unlikely(pkt->pktSize < pkt->tlen) )
				pkt->pktSize = pkt->tlen;
		}
    }
	else if ( pkt->ethType == ETHER_TYPE_ARP) {
		/* Start from Ethernet header */
		arpPkt_t *arp = (arpPkt_t *)ether_hdr;

		arp->hrd = htons(1);
		arp->pro = htons(ETHER_TYPE_IPv4);
		arp->hln = ETHER_ADDR_LEN;
		arp->pln = 4;			// TODO IPv6 ARP
		arp->op  = htons(2);	// FIXME make request/reply operation selectable by user

		ether_addr_copy(&pkt->eth_src_addr, (struct ether_addr *)&arp->sha);
		arp->spa._32 = htonl(pkt->ip_src_addr);

		ether_addr_copy(&pkt->eth_dst_addr, (struct ether_addr*)&arp->tha);
		arp->tpa._32 = htonl(pkt->ip_dst_addr);
	}
	else {
		pktgen_log_error("Unknown EtherType 0x%04x", pkt->ethType);
	}
}

/**************************************************************************//**
*
* pktgen_send_mbuf - Send a single packet to the given port.
*
* DESCRIPTION
* Send a single packet to a given port, but enqueue the packet until we have
* a given burst count of packets to send.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_send_mbuf(struct rte_mbuf *m, uint8_t pid, uint16_t qid)
{
    port_info_t * info = &pktgen.info[pid];
	struct mbuf_table	* mtab = &info->q[qid].tx_mbufs;

    // Add packet to the TX list.
    mtab->m_table[mtab->len++] = m;

    /* Fill our tx burst requirement */
    if (unlikely(mtab->len >= info->tx_burst))
        pktgen_send_burst(info, qid);
}

/**************************************************************************//**
*
* pktgen_packet_type - Examine a packet and return the type of packet
*
* DESCRIPTION
* Examine a packet and return the type of packet.
* the packet.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ pktType_e
pktgen_packet_type( struct rte_mbuf * m )
{
    pktType_e   ret;
    struct ether_hdr *eth;

    eth = rte_pktmbuf_mtod(m, struct ether_hdr *);

    ret = ntohs(eth->ether_type);

    return ret;
}

/**************************************************************************//**
*
* pktgen_packet_classify - Examine a packet and classify it for statistics
*
* DESCRIPTION
* Examine a packet and determine its type along with counting statistics around
* the packet.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void
pktgen_packet_classify( struct rte_mbuf * m, int pid )
{
    port_info_t * info = &pktgen.info[pid];
    int     plen = (m->pkt_len + FCS_SIZE);
	uint32_t	flags;
    pktType_e   pType;

    pType = pktgen_packet_type(m);

	flags = rte_atomic32_read(&info->port_flags);
	if ( unlikely(flags & (PROCESS_INPUT_PKTS | PROCESS_RX_TAP_PKTS)) ) {
		if ( unlikely(flags & PROCESS_RX_TAP_PKTS) ) {
			if ( write(info->rx_tapfd, rte_pktmbuf_mtod(m, char *), m->pkt_len) < 0 )
				pktgen_log_error("Write failed for rx_tap%d", pid);
		}

		switch((int)pType) {
		case ETHER_TYPE_ARP:	info->stats.arp_pkts++;		pktgen_process_arp(m, pid, 0);     break;
		case ETHER_TYPE_IPv4:   info->stats.ip_pkts++;		pktgen_process_ping4(m, pid, 0);   break;
		case ETHER_TYPE_IPv6:   info->stats.ipv6_pkts++;	pktgen_process_ping6(m, pid, 0);   break;
		case ETHER_TYPE_VLAN:   info->stats.vlan_pkts++;	pktgen_process_vlan(m, pid);       break;
		case UNKNOWN_PACKET:    /* FALL THRU */
		default: 				break;
		}
	} else {
		// Count the type of packets found.
		switch((int)pType) {
		case ETHER_TYPE_ARP:        info->stats.arp_pkts++;     break;
		case ETHER_TYPE_IPv4:       info->stats.ip_pkts++;      break;
		case ETHER_TYPE_IPv6:       info->stats.ipv6_pkts++;    break;
		case ETHER_TYPE_VLAN:       info->stats.vlan_pkts++;    break;
		default:			    	break;
		}
	}

    // Count the size of each packet.
    if ( plen == ETHER_MIN_LEN )
        info->sizes._64++;
    else if ( (plen >= (ETHER_MIN_LEN + 1)) && (plen <= 127) )
        info->sizes._65_127++;
    else if ( (plen >= 128) && (plen <= 255) )
        info->sizes._128_255++;
    else if ( (plen >= 256) && (plen <= 511) )
        info->sizes._256_511++;
    else if ( (plen >= 512) && (plen <= 1023) )
        info->sizes._512_1023++;
    else if ( (plen >= 1024) && (plen <= ETHER_MAX_LEN) )
        info->sizes._1024_1518++;
    else if ( plen < ETHER_MIN_LEN )
        info->sizes.runt++;
    else if ( plen >= (ETHER_MAX_LEN + 1) )
        info->sizes.jumbo++;

    // Process multicast and broadcast packets.
    if ( unlikely(((uint8_t *)m->buf_addr + m->data_off)[0] == 0xFF) ) {
		if ( (((uint64_t *)m->buf_addr + m->data_off)[0] & 0xFFFFFFFFFFFF0000LL) == 0xFFFFFFFFFFFF0000LL )
			info->sizes.broadcast++;
		else if ( ((uint8_t *)m->buf_addr + m->data_off)[0] & 1 )
			info->sizes.multicast++;
    }
}

/**************************************************************************//**
*
* pktgen_packet_classify_buld - Classify a set of packets in one call.
*
* DESCRIPTION
* Classify a list of packets and to improve classify performance.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

#define PREFETCH_OFFSET		3
static __inline__ void
pktgen_packet_classify_bulk(struct rte_mbuf ** pkts, int nb_rx, int pid )
{
	int j;

	/* Prefetch first packets */
	for (j = 0; j < PREFETCH_OFFSET && j < nb_rx; j++)
		rte_prefetch0(rte_pktmbuf_mtod(pkts[j], void *));

	/* Prefetch and handle already prefetched packets */
	for (j = 0; j < (nb_rx - PREFETCH_OFFSET); j++) {
		rte_prefetch0(rte_pktmbuf_mtod(pkts[j + PREFETCH_OFFSET], void *));
		pktgen_packet_classify(pkts[j], pid);
	}

	/* Handle remaining prefetched packets */
	for (; j < nb_rx; j++)
		pktgen_packet_classify(pkts[j], pid);
}

/**************************************************************************//**
*
* pktgen_send_special - Send a special packet to the given port.
*
* DESCRIPTION
* Create a special packet in the buffer provided.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void
pktgen_send_special(port_info_t * info)
{
    uint32_t    flags;
    uint32_t    s;

    flags = rte_atomic32_read(&info->port_flags);
    if ( unlikely((flags & SEND_ARP_PING_REQUESTS) == 0) )
        return;

    // Send packets attached to the sequence packets.
    for(s = 0; s < info->seqCnt; s++) {
        if ( flags & SEND_GRATUITOUS_ARP )
            pktgen_send_arp(info->pid, GRATUITOUS_ARP, s);
        if ( flags & SEND_ARP_REQUEST )
            pktgen_send_arp(info->pid, 0, s);

        if ( flags & SEND_PING4_REQUEST )
            pktgen_send_ping4(info->pid, s);
#ifdef INCLUDE_PING6
        if ( flags & SEND_PING6_REQUEST )
            pktgen_send_ping6(info->pid, s);
#endif
    }

    // Send the requests from the Single packet setup.
	if ( flags & SEND_GRATUITOUS_ARP )
		pktgen_send_arp(info->pid, GRATUITOUS_ARP, SINGLE_PKT);
	if ( flags & SEND_ARP_REQUEST )
		pktgen_send_arp(info->pid, 0, SINGLE_PKT);

	if ( flags & SEND_PING4_REQUEST )
		pktgen_send_ping4(info->pid, SINGLE_PKT);
#ifdef INCLUDE_PING6
	if ( flags & SEND_PING6_REQUEST )
		pktgen_send_ping6(info->pid, SINGLE_PKT);
#endif

	pktgen_clr_port_flags(info, SEND_ARP_PING_REQUESTS);
}

/**************************************************************************//**
*
* pktgen_setup_packets - Setup the default packets to be sent.
*
* DESCRIPTION
* Construct the default set of packets for a given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
pktgen_setup_packets(port_info_t * info, struct rte_mempool * mp, uint16_t qid)
{
	struct rte_mbuf	* m, * mm;
	pkt_seq_t * pkt;

	pktgen_clr_q_flags(info, qid, CLEAR_FAST_ALLOC_FLAG);

	if ( mp == info->q[qid].pcap_mp )
		return;

	mm	= NULL;
	pkt = NULL;

	if ( mp == info->q[qid].tx_mp )
		pkt = &info->seq_pkt[SINGLE_PKT];
	else if ( mp == info->q[qid].range_mp )
		pkt = &info->seq_pkt[RANGE_PKT];
	else if ( mp == info->q[qid].seq_mp )
		pkt = &info->seq_pkt[info->seqIdx];

	// allocate each mbuf and put them on a list to be freed.
	for(;;) {
		m = rte_pktmbuf_alloc(mp);
		if ( unlikely(m == NULL) )
			break;

		// Put the allocated mbuf into a list to be freed later
		m->next = mm;
		mm = m;

		if ( mp == info->q[qid].tx_mp ) {
			pktgen_packet_ctor(info, SINGLE_PKT, -1);

			rte_memcpy((uint8_t *)m->buf_addr + m->data_off, (uint8_t *)&pkt->hdr, MAX_PKT_SIZE);

			m->pkt_len  = pkt->pktSize;
			m->data_len = pkt->pktSize;
		} else if ( mp == info->q[qid].range_mp ) {
			pktgen_range_ctor(&info->range, pkt);
			pktgen_packet_ctor(info, RANGE_PKT, -1);

			rte_memcpy((uint8_t *)m->buf_addr + m->data_off, (uint8_t *)&pkt->hdr, MAX_PKT_SIZE);

			m->pkt_len  = pkt->pktSize;
			m->data_len = pkt->pktSize;
		} else if ( mp == info->q[qid].seq_mp ) {
			pktgen_packet_ctor(info, info->seqIdx++, -1);
			if ( unlikely(info->seqIdx >= info->seqCnt) )
				info->seqIdx = 0;

			rte_memcpy((uint8_t *)m->buf_addr + m->data_off, (uint8_t *)&pkt->hdr, MAX_PKT_SIZE);

			m->pkt_len  = pkt->pktSize;
			m->data_len = pkt->pktSize;

			// move to the next packet in the sequence.
			pkt = &info->seq_pkt[info->seqIdx];
		}
	}

	if ( mm != NULL )
		rte_pktmbuf_free(mm);
}

/**************************************************************************//**
*
* pktgen_send_pkts - Send a set of packet buffers to a given port.
*
* DESCRIPTION
* Transmit a set of packets mbufs to a given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
pktgen_send_pkts(port_info_t * info, uint16_t qid, struct rte_mempool * mp)
{
	int64_t		txCnt = 0;
	int			len = 0;

	if ( unlikely(rte_atomic32_read(&info->q[qid].flags) & CLEAR_FAST_ALLOC_FLAG) )
		pktgen_setup_packets(info, mp, qid);

	if ( rte_atomic32_read(&info->port_flags) & SEND_FOREVER ) {
		len = wr_pktmbuf_alloc_bulk_noreset(mp, info->q[qid].tx_mbufs.m_table, info->tx_burst);
	} else {
		txCnt = pkt_atomic64_tx_count(&info->current_tx_count, info->tx_burst);
		if ( txCnt )
			len = wr_pktmbuf_alloc_bulk_noreset(mp, info->q[qid].tx_mbufs.m_table, txCnt);
		else {
			pktgen_clr_port_flags(info, (SENDING_PACKETS | SEND_FOREVER));
			pktgen_set_q_flags(info, qid, DO_TX_CLEANUP);
		}
	}
	if ( len > 0 ) {
		info->q[qid].tx_mbufs.len = len;
		pktgen_send_burst(info, qid);
	}
}

/**************************************************************************//**
*
* pktgen_main_transmit - Determine the next packet format to transmit.
*
* DESCRIPTION
* Determine the next packet format to transmit for a given port.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
pktgen_main_transmit(port_info_t * info, uint16_t qid)
{
    uint32_t		flags;

	flags = rte_atomic32_read(&info->port_flags);

	/*
	 * Transmit ARP/Ping packets if needed
	 */
	pktgen_send_special(info);

	// When not transmitting on this port then continue.
	if ( likely( (flags & SENDING_PACKETS) == SENDING_PACKETS ) ) {
		struct rte_mempool * mp	= info->q[qid].tx_mp;

		if ( unlikely(flags & (SEND_RANGE_PKTS|SEND_PCAP_PKTS|SEND_SEQ_PKTS)) ) {
			if ( flags & SEND_RANGE_PKTS )
				mp = info->q[qid].range_mp;
			else if ( flags & SEND_SEQ_PKTS )
				mp = info->q[qid].seq_mp;
			else if ( flags & SEND_PCAP_PKTS )
				mp = info->q[qid].pcap_mp;
		}

		pktgen_send_pkts(info, qid, mp);

		flags = rte_atomic32_read(&info->q[qid].flags);
		if ( unlikely(flags & (DO_TX_CLEANUP |  DO_TX_FLUSH)) ) {
			if ( flags & DO_TX_CLEANUP )
				pktgen_tx_cleanup(info, qid);
			else if ( flags & DO_TX_FLUSH )
				pktgen_tx_flush(info, qid);
		}
	}
}

/**************************************************************************//**
*
* pktgen_main_receive - Main receive routine for packets of a port.
*
* DESCRIPTION
* Handle the main receive set of packets on a given port plus handle all of the
* input processing if required.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static __inline__ void
pktgen_main_receive(port_info_t * info, uint8_t lid, struct rte_mbuf *pkts_burst[])
{
	uint8_t pid;
	uint16_t qid, nb_rx;
	capture_t *capture;

	pid = info->pid;
	qid = wr_get_rxque(pktgen.l2p, lid, pid);

	/*
	 * Read packet from RX queues and free the mbufs
	 */
	if ( (nb_rx = rte_eth_rx_burst(pid, qid, pkts_burst, info->tx_burst)) == 0 )
		return;

	// packets are not freed in the next call.
	pktgen_packet_classify_bulk(pkts_burst, nb_rx, pid);

	if ( unlikely(info->dump_count > 0) )
		pktgen_packet_dump_bulk(pkts_burst, nb_rx, pid);

	if ( unlikely(rte_atomic32_read(&info->port_flags) & CAPTURE_PKTS) ) {
		capture = &pktgen.capture[pktgen.core_info[lid].s.socket_id];
		if ( unlikely((capture->port == pid) && (capture->lcore == lid)) ) {
			pktgen_packet_capture_bulk(pkts_burst, nb_rx, capture);
		}
	}

	rte_pktmbuf_free_bulk(pkts_burst, nb_rx);
}

/**************************************************************************//**
*
* pktgen_main_rxtx_loop - Single thread loop for tx/rx packets
*
* DESCRIPTION
* Handle sending and receiving packets from a given set of ports. This is the
* main loop or thread started on a single core.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void
pktgen_main_rxtx_loop(uint8_t lid)
{
    struct rte_mbuf *pkts_burst[DEFAULT_PKT_BURST];
	port_info_t	  * infos[RTE_MAX_ETHPORTS];
	uint8_t		 	qids[RTE_MAX_ETHPORTS];
    uint8_t			idx, pid, txcnt, rxcnt;
    uint64_t		 curr_tsc;
	uint64_t		tx_next_cycle;		/**< Next cycle to send a burst of traffic */
	char			msg[256];

    txcnt	= wr_get_lcore_txcnt(pktgen.l2p, lid);
	rxcnt	= wr_get_lcore_rxcnt(pktgen.l2p, lid);
	snprintf(msg, sizeof(msg),
			"=== RX/TX processing on lcore %2d, rxcnt %d, txcnt %d, port/qid,",
			lid, rxcnt, txcnt);

	for( idx = 0; idx < wr_get_lcore_txcnt(pktgen.l2p, lid); idx++ ) {
		pid = wr_get_rx_pid(pktgen.l2p, lid, idx);
    	if ( (infos[idx] = wr_get_port_private(pktgen.l2p, pid)) == NULL )
    		continue;
    	qids[idx] = wr_get_txque(pktgen.l2p, lid , pid);
		strncatf(msg, " %d/%d", infos[idx]->pid, qids[idx]);
	}
	pktgen_log_info("%s", msg);

    tx_next_cycle	= 0;

    wr_start_lcore(pktgen.l2p, lid);
    do {
		for(idx = 0; idx < rxcnt; idx++) {
			/*
			 * Read packet from RX queues and free the mbufs
			 */
			pktgen_main_receive(infos[idx], lid, pkts_burst);
		}

        curr_tsc = rte_rdtsc();

		// Determine when is the next time to send packets
		if ( unlikely(curr_tsc >= tx_next_cycle) ) {

			tx_next_cycle = curr_tsc + infos[0]->tx_cycles;

	    	for(idx = 0; idx < txcnt; idx++) {
				/*
				 * Transmit packets at a given rate.
				 */
				pktgen_main_transmit(infos[idx], qids[idx]);
	    	}
		}

		// Exit loop when flag is set.
    } while ( wr_lcore_is_running(pktgen.l2p, lid) );

    pktgen_log_debug("Exit %d", lid);

	pktgen_exit_cleanup(lid);
}

/**************************************************************************//**
*
* pktgen_main_tx_loop - Main transmit loop for a core, no receive packet handling
*
* DESCRIPTION
* When Tx and Rx are split across two cores this routing handles the tx packets.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void
pktgen_main_tx_loop(uint8_t lid)
{
    uint8_t		  idx, txcnt, pid;
    port_info_t * infos[RTE_MAX_ETHPORTS];
    uint8_t       qids[RTE_MAX_ETHPORTS];
    uint64_t	  curr_tsc;
	uint64_t	  tx_next_cycle;		/**< Next cycle to send a burst of traffic */
	char			msg[256];

    txcnt = wr_get_lcore_txcnt(pktgen.l2p, lid);
	snprintf(msg, sizeof(msg),
			"=== TX processing on lcore %2d, txcnt %d, port/qid,",
			lid, txcnt);

	for( idx = 0; idx < txcnt; idx++ ) {
		pid = wr_get_tx_pid(pktgen.l2p, lid, idx);
    	if ( (infos[idx] = wr_get_port_private(pktgen.l2p, pid)) == NULL )
    		continue;
    	qids[idx] = wr_get_txque(pktgen.l2p, lid, pid);
		strncatf(msg, " %d/%d", infos[idx]->pid, qids[idx]);
	}
	pktgen_log_info("%s", msg);

	tx_next_cycle = 0;

	wr_start_lcore(pktgen.l2p, lid);
    do {
		curr_tsc = rte_rdtsc();

		// Determine when is the next time to send packets
		if ( unlikely(curr_tsc >= tx_next_cycle) ) {

			tx_next_cycle = curr_tsc + infos[0]->tx_cycles;

	    	for(idx = 0; idx < txcnt; idx++) {
				/* Transmit packets */
				pktgen_main_transmit(infos[idx], qids[idx]);
	    	}
    	}

		// Exit loop when flag is set.
    } while( wr_lcore_is_running(pktgen.l2p, lid) );

    pktgen_log_debug("Exit %d", lid);

	pktgen_exit_cleanup(lid);
}

/**************************************************************************//**
*
* pktgen_main_rx_loop - Handle only the rx packets for a set of ports.
*
* DESCRIPTION
* When Tx and Rx processing is split between two ports this routine handles
* only the receive packets.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void
pktgen_main_rx_loop(uint8_t lid)
{
    struct rte_mbuf *pkts_burst[DEFAULT_PKT_BURST];
	uint8_t			pid, idx, rxcnt;
	port_info_t	  * infos[RTE_MAX_ETHPORTS];
	char			msg[256];

	rxcnt = wr_get_lcore_rxcnt(pktgen.l2p, lid);
	snprintf(msg, sizeof(msg),
			"=== RX processing on lcore %2d, rxcnt %d, port/qid,",
			lid, rxcnt);

    memset(infos, '\0', sizeof(infos));
	for( idx = 0; idx < rxcnt; idx++ ) {
		pid = wr_get_rx_pid(pktgen.l2p, lid, idx);
    	if ( (infos[idx] = wr_get_port_private(pktgen.l2p, pid)) == NULL )
    		continue;
		strncatf(msg, " %d/%d", infos[idx]->pid, wr_get_rxque(pktgen.l2p, lid, pid));
	}
	pktgen_log_info("%s", msg);

	wr_start_lcore(pktgen.l2p, lid);
    do {
		for(idx = 0; idx < rxcnt; idx++) {
			// Read packet from RX queues and free the mbufs
			pktgen_main_receive(infos[idx], lid, pkts_burst);
		}
		// Exit loop when flag is set.
    } while( wr_lcore_is_running(pktgen.l2p, lid) );

    pktgen_log_debug("Exit %d", lid);

    pktgen_exit_cleanup(lid);
}

/**************************************************************************//**
*
* pktgen_launch_one_lcore - Launch a single logical core thread.
*
* DESCRIPTION
* Help launch a single thread on one logical core.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

int
pktgen_launch_one_lcore(__attribute__ ((unused)) void * arg)
{
	uint8_t		lid = rte_lcore_id();

	if ( pktgen_has_work() )
		return 0;

	rte_delay_ms((lid + 1) *21);

	switch(wr_get_type(pktgen.l2p, lid)) {
	case RX_TYPE:				pktgen_main_rx_loop(lid);		break;
	case TX_TYPE:				pktgen_main_tx_loop(lid);		break;
	case (RX_TYPE | TX_TYPE):	pktgen_main_rxtx_loop(lid); 	break;
	}
	return 0;
}

/**************************************************************************//**
*
* pktgen_page_config - Show the configuration page for pktgen.
*
* DESCRIPTION
* Display the pktgen configuration page. (Not used)
*
* RETURNS: N/A
*
* SEE ALSO:
*/

static void
pktgen_page_config(void)
{
	display_topline("<Config Page>");

    wr_scrn_center(20, pktgen.scrn->ncols, "Need to add the configuration stuff here");
    display_dashline(22);
}

/**************************************************************************//**
*
* pktgen_page_display - Display the correct page based on timer0 callback.
*
* DESCRIPTION
* When timer0 is active update or display the correct page of data.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
pktgen_page_display(__attribute__((unused)) struct rte_timer *tim, __attribute__((unused)) void *arg)
{
    static unsigned int counter = 0;

    // Leave if the screen is paused
    if ( wr_scrn_is_paused() )
        return;

    wr_scrn_save();

	pktgen_display_set_color("top.spinner");
    wr_scrn_printf(1,1, "%c", "-\\|/"[(counter++ & 3)]);
	pktgen_display_set_color(NULL);

    if ( pktgen.flags & CPU_PAGE_FLAG )
        pktgen_page_cpu();
    else if ( pktgen.flags & PCAP_PAGE_FLAG )
        pktgen_page_pcap(pktgen.portNum);
    else if ( pktgen.flags & RANGE_PAGE_FLAG )
        pktgen_page_range();
    else if ( pktgen.flags & CONFIG_PAGE_FLAG )
        pktgen_page_config();
    else if ( pktgen.flags & SEQUENCE_PAGE_FLAG )
        pktgen_page_seq(pktgen.portNum);
	else if ( pktgen.flags & RND_BITFIELD_PAGE_FLAG )
		pktgen_page_random_bitfields(pktgen.flags & PRINT_LABELS_FLAG, pktgen.portNum, pktgen.info[pktgen.portNum].rnd_bitfields);
	else if ( pktgen.flags & LOG_PAGE_FLAG )
		pktgen_page_log(pktgen.flags & PRINT_LABELS_FLAG);
    else
        pktgen_page_stats();

    wr_scrn_restore();

    pktgen_print_packet_dump();

	if (pktgen.flags & PRINT_LABELS_FLAG)
		pktgen.flags &= ~PRINT_LABELS_FLAG;
}

static struct rte_timer timer0;
static struct rte_timer timer1;

/**************************************************************************//**
*
* pktgen_timer_setup - Set up the timer callback routines.
*
* DESCRIPTION
* Setup the two timers to be used for display and calculating statistics.
*
* RETURNS: N/A
*
* SEE ALSO:
*/

void
rte_timer_setup(void)
{
    int lcore_id = rte_get_master_lcore();

    /* init RTE timer library */
    rte_timer_subsystem_init();

    /* init timer structures */
    rte_timer_init(&timer0);
    rte_timer_init(&timer1);

    /* load timer0, every 2 seconds, on Display lcore, reloaded automatically */
    rte_timer_reset(&timer0, (pktgen.hz*2), PERIODICAL, lcore_id, pktgen_page_display, NULL);

    /* load timer1, every second, on timer lcore, reloaded automatically */
    rte_timer_reset(&timer1, pktgen.hz, PERIODICAL, lcore_id, pktgen_process_stats, NULL);
}
