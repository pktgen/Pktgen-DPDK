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

#include "pktgen-udp.h"

/**************************************************************************//**
 *
 * pktgen_udp_hdr_ctor - UDP header constructor routine.
 *
 * DESCRIPTION
 * Construct the UDP header in a packer buffer.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_udp_hdr_ctor(pkt_seq_t *pkt, void *hdr, int type)
{
	uint16_t tlen;

	if (type == ETHER_TYPE_IPv4) {
		udpip_t *uip = (udpip_t *)hdr;

		/* Zero out the header space */
		memset((char *)uip, 0, sizeof(udpip_t));

		/* Create the UDP header */
		uip->ip.src         = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
		uip->ip.dst         = htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);
		tlen                = pkt->pktSize -
			(pkt->ether_hdr_size + sizeof(ipHdr_t));

		uip->ip.len         = htons(tlen);
		uip->ip.proto       = pkt->ipProto;

		uip->udp.len        = htons(tlen);
		uip->udp.sport      = htons(pkt->sport);
		uip->udp.dport      = htons(pkt->dport);

		/* Includes the pseudo header information */
		tlen                = pkt->pktSize - pkt->ether_hdr_size;

		uip->udp.cksum      = cksum(uip, tlen, 0);
		if (uip->udp.cksum == 0)
			uip->udp.cksum = 0xFFFF;
	} else {
		uint32_t addr;
		udpipv6_t *uip = (udpipv6_t *)hdr;

		/* Zero out the header space */
		memset((char *)uip, 0, sizeof(udpipv6_t));

		/* Create the pseudo header and TCP information */
		addr                = htonl(pkt->ip_dst_addr.addr.ipv4.s_addr);
		(void)rte_memcpy(&uip->ip.daddr[8], &addr,
				 sizeof(uint32_t));
		addr                = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
		(void)rte_memcpy(&uip->ip.saddr[8], &addr,
				 sizeof(uint32_t));

		tlen                = sizeof(udpHdr_t) +
			(pkt->pktSize - pkt->ether_hdr_size -
			 sizeof(ipv6Hdr_t) - sizeof(udpHdr_t));
		uip->ip.tcp_length  = htonl(tlen);
		uip->ip.next_header = pkt->ipProto;

		uip->udp.sport      = htons(pkt->sport);
		uip->udp.dport      = htons(pkt->dport);

		tlen                = sizeof(udpipv6_t) +
			(pkt->pktSize - pkt->ether_hdr_size -
			 sizeof(ipv6Hdr_t) - sizeof(udpHdr_t));
		uip->udp.cksum      = cksum(uip, tlen, 0);
		if (uip->udp.cksum == 0)
			uip->udp.cksum = 0xFFFF;
	}
}
