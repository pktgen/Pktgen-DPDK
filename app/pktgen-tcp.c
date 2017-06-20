/*-
 * Copyright (c) <2010-2017>, Intel Corporation
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
/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
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

void
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
}
