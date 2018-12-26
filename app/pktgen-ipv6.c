/*-
 * Copyright (c) <2010-2019>, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Created 2010 by Keith Wiles @ intel.com */

#include <cli_scrn.h>
#include <rte_lua.h>

#include "pktgen.h"

#include "pktgen-ipv6.h"

/**************************************************************************//**
 *
 * pktgen_ipv6_ctor - IPv6 packet header constructor routine.
 *
 * DESCRIPTION
 * Construct the IPv6 header constructor routine.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_ipv6_ctor(pkt_seq_t *pkt, void *hdr)
{
	ipv6Hdr_t *ip = hdr;
	uint16_t tlen;

	/* IPv6 Header constructor */
	memset(ip, 0, sizeof(ipv6Hdr_t));

	ip->ver_tc_fl       = htonl(IPv6_VERSION << 28);
	tlen                = pkt->pktSize -
		(pkt->ether_hdr_size + sizeof(ipv6Hdr_t));

	ip->payload_length  = htons(tlen);
	ip->hop_limit       = 4;
	ip->next_header     = pkt->ipProto;

	rte_memcpy(&ip->daddr[8],
		   pkt->ip_dst_addr.addr.ipv6.s6_addr,
		   sizeof(struct in6_addr));
	rte_memcpy(&ip->saddr[8],
		   pkt->ip_dst_addr.addr.ipv6.s6_addr,
		   sizeof(struct in6_addr));
}

/**************************************************************************//**
 *
 * pktgen_process_ping6 - Process a IPv6 ICMP echo request packet.
 *
 * DESCRIPTION
 * Process a IPv6 ICMP echo request packet and send response if needed.
 *
 * RETURNS: N/A
 *
 * SEE ALSO:
 */

void
pktgen_process_ping6(struct rte_mbuf *m __rte_unused,
		     uint32_t pid __rte_unused, uint32_t vlan __rte_unused)
{
#if 0	/* Broken needs to be updated to do IPv6 packets */
	port_info_t     *info = &pktgen.info[pid];
	struct ether_hdr *eth = rte_pktmbuf_mtod(m, struct ether_hdr *);
	ipv6Hdr_t       *ip = (ipv6Hdr_t *)&eth[1];

	/* Adjust for a vlan header if present */
	if (vlan)
		ip = (ipv6Hdr_t *)((char *)ip + sizeof(struct vlan_hdr));

	/* Look for a ICMP echo requests, but only if enabled. */
	if ( (rte_atomic32_read(&info->port_flags) & ICMP_ECHO_ENABLE_FLAG) &&
	     (ip->next_header == PG_IPPROTO_ICMPV6) ) {
#if !defined(RTE_ARCH_X86_64)
		icmpv4Hdr_t *icmp =
			(icmpv4Hdr_t *)((uint32_t)ip + sizeof(ipHdr_t));
#else
		icmpv4Hdr_t *icmp =
			(icmpv4Hdr_t *)((uint64_t)ip + sizeof(ipHdr_t));
#endif
		/* We do not handle IP options, which will effect the IP header size. */
		if (cksum(icmp,
			  (m->pkt.data_len - sizeof(struct ether_hdr) -
			   sizeof(ipHdr_t)),
			  0) ) {
			rte_printf_status("ICMP checksum failed\n");
			goto leave :
		}

		if (icmp->type == ICMP4_ECHO) {
			/* Toss all broadcast addresses and requests not for this port */
			if ( (ip->dst == INADDR_BROADCAST) ||
			     (ip->dst != info->ip_src_addr) ) {
				char buff[24];
				rte_printf_status("IP address %s != ",
						  inet_ntop4(buff, sizeof(buff),
							     ip->dst,
							     INADDR_BROADCAST));
				rte_printf_status("%s\n",
						  inet_ntop4(buff, sizeof(buff),
							     htonl(info->
								   ip_src_addr),
							     INADDR_BROADCAST));
				goto leave;
			}

			info->echo_pkts++;

			icmp->type  = ICMP4_ECHO_REPLY;

			/* Recompute the ICMP checksum */
			icmp->cksum = 0;
			icmp->cksum =
				cksum(icmp,
				      (m->pkt.data_len -
				       sizeof(struct ether_hdr) -
				       sizeof(ipHdr_t)), 0);

			/* Swap the IP addresses. */
			inetAddrSwap(&ip->src, &ip->dst);

			/* Bump the ident value */
			ip->ident   = htons(ntohs(ip->ident) + m->pkt.data_len);

			/* Recompute the IP checksum */
			ip->cksum   = 0;
			ip->cksum   = cksum(ip, sizeof(ipHdr_t), 0);

			/* Swap the MAC addresses */
			ethAddrSwap(&eth->d_addr, &eth->s_addr);

			pktgen_send_mbuf(m, pid, 0);

			pktgen_set_q_flags(info, 0, DO_TX_FLUSH);

			/* No need to free mbuf as it was reused */
			return;
		}
	}
leave:
#else
#endif
}
