/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include <lua_config.h>

#include "pktgen.h"

#include "pktgen-tcp.h"

/**************************************************************************//**
 *
 * pktgen_tcp_hdr_ctor - TCP header constructor routine.
 *
 * DESCRIPTION
 * Construct a TCP header in the packet buffer provided.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void *
pktgen_tcp_hdr_ctor(pkt_seq_t *pkt, void * hdr, int type)
{
	uint16_t tlen;

	if (type == PG_ETHER_TYPE_IPv4) {
		struct pg_ipv4_hdr *ipv4 = (struct pg_ipv4_hdr *)hdr;
		struct pg_tcp_hdr *tcp = (struct pg_tcp_hdr *)&ipv4[1];

		/* Create the TCP header */
		ipv4->src_addr = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
		ipv4->dst_addr = htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);

		tlen = pkt->pktSize - pkt->ether_hdr_size;
		ipv4->total_length = htons(tlen);
		ipv4->next_proto_id = pkt->ipProto;

		tcp->src_port = htons(pkt->sport);
		tcp->dst_port = htons(pkt->dport);
		tcp->sent_seq = htonl(DEFAULT_PKT_NUMBER);
		tcp->recv_ack = htonl(DEFAULT_ACK_NUMBER);
		tcp->data_off = ((sizeof(struct pg_tcp_hdr) / sizeof(uint32_t)) << 4);	/* Offset in words */
		tcp->tcp_flags = ACK_FLAG;						/* ACK */
		tcp->rx_win = htons(DEFAULT_WND_SIZE);
		tcp->tcp_urp = 0;

		tcp->cksum = 0;
		tcp->cksum = rte_ipv4_udptcp_cksum(ipv4, (const void *)tcp);
	} else {
		struct pg_ipv6_hdr *ipv6 = (struct pg_ipv6_hdr *)hdr;
		struct pg_tcp_hdr *tcp = (struct pg_tcp_hdr *)&ipv6[1];

		/* Create the pseudo header and TCP information */
		memset(ipv6->dst_addr, 0, sizeof(struct in6_addr));
		memset(ipv6->src_addr, 0, sizeof(struct in6_addr));
		rte_memcpy(ipv6->dst_addr, &pkt->ip_dst_addr.addr.ipv6.s6_addr,
				sizeof(struct in6_addr));
		rte_memcpy(ipv6->src_addr, &pkt->ip_src_addr.addr.ipv6.s6_addr,
				sizeof(struct in6_addr));

		tlen = pkt->pktSize - (pkt->ether_hdr_size + sizeof(struct pg_ipv6_hdr));
		ipv6->payload_len = htons(tlen);
		ipv6->proto = pkt->ipProto;

		tcp->src_port = htons(pkt->sport);
		tcp->dst_port = htons(pkt->dport);
		tcp->sent_seq = htonl(DEFAULT_PKT_NUMBER);
		tcp->recv_ack = htonl(DEFAULT_ACK_NUMBER);
		tcp->data_off =
			((sizeof(struct pg_tcp_hdr) / sizeof(uint32_t)) << 4);   /* Offset in words */
		tcp->tcp_flags = ACK_FLAG; /* ACK */
		tcp->rx_win     = htons(DEFAULT_WND_SIZE);
		tcp->tcp_urp = 0;

		tcp->cksum = 0;
		tcp->cksum = rte_ipv6_udptcp_cksum(ipv6, (const void *)tcp);
	}

	/* In this case we return the original value to allow IP ctor to work */
	return hdr;
}
