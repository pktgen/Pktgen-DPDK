/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include "rte_lua.h"

#include "pktgen.h"

#include "pktgen-udp.h"

/**************************************************************************//**
 *
 * pktgen_udp_hdr_ctor - UDP header constructor routine.
 *
 * DESCRIPTION
 * Construct the UDP header in a packer buffer.
 *
 * RETURNS: next header location
 *
 * SEE ALSO:
 */

void *
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
		tlen                = pkt->pktSize - (pkt->ether_hdr_size + sizeof(ipHdr_t));

		uip->ip.len         = htons(tlen);
		uip->ip.proto       = pkt->ipProto;

		uip->udp.len        = htons(tlen);
		uip->udp.sport      = htons(pkt->sport);
		uip->udp.dport      = htons(pkt->dport);

		if (pkt->dport == VXLAN_PORT_ID) {
			struct vxlan *vxlan = (struct vxlan *)&uip[1];

			vxlan->vni_flags = htons(pkt->vni_flags);
			vxlan->group_id  = htons(pkt->group_id);
			vxlan->vxlan_id  = htonl(pkt->vxlan_id) << 8;
		}

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
		(void)rte_memcpy(&uip->ip.daddr[8], &addr, sizeof(uint32_t));
		addr                = htonl(pkt->ip_src_addr.addr.ipv4.s_addr);
		(void)rte_memcpy(&uip->ip.saddr[8], &addr, sizeof(uint32_t));

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

	/* Return the original pointer for IP ctor */
	return hdr;
}
