/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include <rte_lua.h>

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

	if (type == ETHER_TYPE_IPv4) {
		tcpip_t *tip = (tcpip_t *)hdr;

		/* Zero out the header space */
		memset((char *)tip, 0, sizeof(tcpip_t));

		/* Create the TCP header */
		tip->ip.src         = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
		tip->ip.dst         = htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);
		tlen                = pkt->pktSize -
			(pkt->ether_hdr_size + sizeof(ipHdr_t));

		tip->ip.len         = htons(tlen);
		tip->ip.proto       = pkt->ipProto;

		tip->tcp.sport      = htons(pkt->sport);
		tip->tcp.dport      = htons(pkt->dport);
		tip->tcp.seq        = htonl(DEFAULT_PKT_NUMBER);
		tip->tcp.ack        = htonl(DEFAULT_ACK_NUMBER);
		tip->tcp.offset     = ((sizeof(tcpHdr_t) / sizeof(uint32_t)) << 4);	/* Offset in words */
		tip->tcp.flags      = ACK_FLAG;						/* ACK */
		tip->tcp.window     = htons(DEFAULT_WND_SIZE);
		tip->tcp.urgent     = 0;

		tlen                = pkt->pktSize - pkt->ether_hdr_size;

		tip->tcp.cksum      = cksum(tip, tlen, 0);
	} else {
		tcpipv6_t *tip = (tcpipv6_t *)hdr;

		/* Zero out the header space */
		memset((char *)tip, 0, sizeof(tcpipv6_t));

		/* Create the pseudo header and TCP information */
		(void)rte_memcpy(tip->ip.daddr, &pkt->ip_dst_addr.addr.ipv4.s_addr,
				 sizeof(struct in6_addr));
		(void)rte_memcpy(tip->ip.saddr, &pkt->ip_src_addr.addr.ipv4.s_addr,
				 sizeof(struct in6_addr));

		tlen                = sizeof(tcpHdr_t) +
			(pkt->pktSize - pkt->ether_hdr_size -
			 sizeof(ipv6Hdr_t) - sizeof(tcpHdr_t));
		tip->ip.tcp_length  = htonl(tlen);
		tip->ip.next_header = pkt->ipProto;

		tip->tcp.sport      = htons(pkt->sport);
		tip->tcp.dport      = htons(pkt->dport);
		tip->tcp.seq        = htonl(DEFAULT_PKT_NUMBER);
		tip->tcp.ack        = htonl(DEFAULT_ACK_NUMBER);
		tip->tcp.offset     =
			((sizeof(tcpHdr_t) / sizeof(uint32_t)) << 4);   /* Offset in words */
		tip->tcp.window     = htons(DEFAULT_WND_SIZE);
		tip->tcp.urgent     = 0;
		tip->tcp.flags      = ACK_FLAG; /* ACK */

		tlen                = sizeof(tcpipv6_t) +
			(pkt->pktSize - pkt->ether_hdr_size - sizeof(ipv6Hdr_t) - sizeof(tcpHdr_t));

		tip->tcp.cksum      = cksum(tip, tlen, 0);
	}

	/* In this case we return the original value to allow IP ctor to work */
	return hdr;
}
