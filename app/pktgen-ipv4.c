/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <arpa/inet.h>

#include <cli_scrn.h>
#include <rte_lua.h>

#include "pktgen.h"
#include "pktgen-log.h"
#include "pktgen-ipv4.h"

/**************************************************************************//**
 *
 * pktgen_ipv4_ctor - Construct the IPv4 header for a packet
 *
 * DESCRIPTION
 * Constructor for the IPv4 header for a given packet.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_ipv4_ctor(pkt_seq_t *pkt, void *hdr)
{
	struct ipv4_hdr *ip = hdr;
	uint16_t tlen;

	/* IPv4 Header constructor */
	tlen = pkt->pktSize - pkt->ether_hdr_size;

	/* Zero out the header space */
	memset((char *)ip, 0, sizeof(struct ipv4_hdr));

	ip->version_ihl = (IPv4_VERSION << 4) | (sizeof(struct ipv4_hdr) / 4);

	ip->total_length = htons(tlen);
	ip->time_to_live = 4;
	ip->type_of_service = pkt->tos;

	pktgen.ident += 27;	/* bump by a prime number */
	ip->packet_id = htons(pktgen.ident);
	ip->fragment_offset = 0;
	ip->next_proto_id = pkt->ipProto;
	ip->src_addr = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
	ip->dst_addr = htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);
	ip->hdr_checksum = 0;
	ip->hdr_checksum = rte_ipv4_cksum((const struct ipv4_hdr *)ip);
}

/**************************************************************************//**
 *
 * pktgen_send_ping4 - Create and send a Ping or ICMP echo packet.
 *
 * DESCRIPTION
 * Create a ICMP echo request packet and send the packet to a give port.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_send_ping4(uint32_t pid, uint8_t seq_idx)
{
	port_info_t       *info = &pktgen.info[pid];
	pkt_seq_t         *ppkt = &info->seq_pkt[PING_PKT];
	pkt_seq_t         *spkt = &info->seq_pkt[seq_idx];
	struct rte_mbuf   *m;
	uint8_t qid = 0;

	m   = rte_pktmbuf_alloc(info->q[qid].special_mp);
	if (unlikely(m == NULL) ) {
		pktgen_log_warning("No packet buffers found");
		return;
	}
	*ppkt = *spkt;	/* Copy the sequence setup to the ping setup. */
	pktgen_packet_ctor(info, PING_PKT, ICMP4_ECHO);
	rte_memcpy((uint8_t *)m->buf_addr + m->data_off,
		   (uint8_t *)&ppkt->hdr, ppkt->pktSize);

	m->pkt_len  = ppkt->pktSize;
	m->data_len = ppkt->pktSize;

	pktgen_send_mbuf(m, pid, qid);

	pktgen_set_q_flags(info, qid, DO_TX_FLUSH);
}

/**************************************************************************//**
 *
 * pktgen_process_ping4 - Process a input ICMP echo packet for IPv4.
 *
 * DESCRIPTION
 * Process a input packet for IPv4 ICMP echo request and send response if needed.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_process_ping4(struct rte_mbuf *m, uint32_t pid, uint32_t vlan)
{
	port_info_t   *info = &pktgen.info[pid];
	pkt_seq_t     *pkt;
	struct ether_hdr *eth = rte_pktmbuf_mtod(m, struct ether_hdr *);
	struct ipv4_hdr *ip = (struct ipv4_hdr *)&eth[1];
	char buff[24];

	/* Adjust for a vlan header if present */
	if (vlan)
		ip = (struct ipv4_hdr *)((char *)ip + sizeof(struct vlan_hdr));

	/* Look for a ICMP echo requests, but only if enabled. */
	if ( (rte_atomic32_read(&info->port_flags) & ICMP_ECHO_ENABLE_FLAG) &&
	     (ip->next_proto_id == PG_IPPROTO_ICMP) ) {
		struct icmp_hdr *icmp =
			(struct icmp_hdr *)((uintptr_t)ip + sizeof(struct ipv4_hdr));

		/* We do not handle IP options, which will effect the IP header size. */
		if (unlikely(rte_raw_cksum(icmp,
				   (m->data_len - sizeof(struct ether_hdr) -
				    sizeof(struct ipv4_hdr)))) ) {
			pktgen_log_error("ICMP checksum failed");
			return;
		}

		if (unlikely(icmp->icmp_type == ICMP4_ECHO) ) {
			if (ntohl(ip->dst_addr) == INADDR_BROADCAST) {
				pktgen_log_warning(
					"IP address %s is a Broadcast",
					inet_ntop4(buff,
						   sizeof(buff), ip->dst_addr,
						   INADDR_BROADCAST));
				return;
			}

			/* Toss all broadcast addresses and requests not for this port */
			pkt = pktgen_find_matching_ipsrc(info, ip->dst_addr);

			/* ARP request not for this interface. */
			if (unlikely(pkt == NULL) ) {
				pktgen_log_warning("IP address %s not found",
						   inet_ntop4(buff,
							      sizeof(buff),
							      ip->dst_addr,
							      INADDR_BROADCAST));
				return;
			}

			info->stats.echo_pkts++;

			icmp->icmp_type  = ICMP4_ECHO_REPLY;

			/* Recompute the ICMP checksum */
			icmp->icmp_cksum = 0;
			icmp->icmp_cksum =
				rte_raw_cksum(icmp,
				      (m->data_len - sizeof(struct ether_hdr) -
				       sizeof(struct ipv4_hdr)));

			/* Swap the IP addresses. */
			inetAddrSwap(&ip->src_addr, &ip->dst_addr);

			/* Bump the ident value */
			ip->packet_id = htons(ntohs(ip->packet_id) + m->data_len);

			/* Recompute the IP checksum */
			ip->hdr_checksum   = 0;
			ip->hdr_checksum   = rte_raw_cksum(ip, sizeof(struct ipv4_hdr));

			/* Swap the MAC addresses */
			ethAddrSwap(&eth->d_addr, &eth->s_addr);

			pktgen_send_mbuf(m, pid, 0);

			pktgen_set_q_flags(info, 0, DO_TX_FLUSH);

			/* No need to free mbuf as it was reused. */
			return;
		} else if (unlikely(icmp->icmp_type == ICMP4_ECHO_REPLY) )
			info->stats.echo_pkts++;
	}
}
